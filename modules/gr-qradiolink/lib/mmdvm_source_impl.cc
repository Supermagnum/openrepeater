/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <climits>
#include <cstdint>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/qradiolink/mmdvm_source.h>
#include "mmdvm_source_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/tags.h>
#include "../src/bursttimer.h"
#include <zmq.hpp>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <ctime>

#include "../src/config_mmdvm.h"

namespace gr {
namespace qradiolink {

const uint8_t MARK_SLOT1 = 0x08U;
const uint8_t MARK_SLOT2 = 0x04U;
const uint8_t MARK_NONE = 0x00U;
const int32_t ZERO_SAMPLES = 720 * 25 / 24; // resampling ratio (SAMPLES_PER_SLOT = 720)

static const pmt::pmt_t TIME_TAG = pmt::string_to_symbol("tx_time");
static const pmt::pmt_t LENGTH_TAG = pmt::string_to_symbol("burst_length");
static const pmt::pmt_t ZERO_TAG = pmt::string_to_symbol("zero_samples");

mmdvm_source::sptr mmdvm_source::make(BurstTimer* burst_timer,
                                      uint8_t cn,
                                      bool multi_channel,
                                      bool use_tdma)
{
    return gnuradio::get_initial_sptr(
        new mmdvm_source_impl(burst_timer, cn, multi_channel, use_tdma));
}

mmdvm_source_impl::mmdvm_source_impl(BurstTimer* burst_timer,
                                    uint8_t cn,
                                    bool multi_channel,
                                    bool use_tdma)
    : mmdvm_source("mmdvm_source",
                   gr::io_signature::make(0, 0, 0),
                   gr::io_signature::make(cn, cn, sizeof(short))),
      d_burst_timer(burst_timer),
      d_num_channels(cn),
      d_timing_correction(0),
      d_sn(2),
      d_add_time_tag((cn == 0) || (cn == 1)),
      d_use_tdma(use_tdma)
{
    if (use_tdma && burst_timer == nullptr) {
        std::cerr << "Warning: mmdvm_source: use_tdma=true but burst_timer is nullptr. TDMA timing will be disabled." << std::endl;
    }
    for (int i = 0; i < d_num_channels; i++) {
        d_zmqcontext[i] = zmq::context_t(1);
        d_zmqsocket[i] = zmq::socket_t(d_zmqcontext[i], ZMQ_REQ);
        d_zmqsocket[i].set(zmq::sockopt::sndhwm, 10);
        d_zmqsocket[i].set(zmq::sockopt::linger, 0);
        int socket_no = multi_channel ? i + 1 : i;
        d_zmqsocket[i].connect("ipc:///tmp/mmdvm-tx" + std::to_string(socket_no) + ".ipc");
        d_in_tx[i] = false;
    }
    set_min_noutput_items(SAMPLES_PER_SLOT);
    set_max_noutput_items(SAMPLES_PER_SLOT);
}

mmdvm_source_impl::~mmdvm_source_impl() {}

void mmdvm_source_impl::get_zmq_message()
{
    for (int j = 0; j < d_num_channels; j++) {
        zmq::message_t mq_message;
        int size = 0;
        zmq::recv_result_t recv_result;

        zmq::message_t request_msg(1);
        memcpy(request_msg.data(), "s", sizeof(char));
        d_zmqsocket[j].send(request_msg, zmq::send_flags::none);
        recv_result = d_zmqsocket[j].recv(mq_message);
        size = mq_message.size();

        if (size < 1) {
            d_in_tx[j] = false;
            continue;
        }

        uint32_t buf_size = 0;
        memcpy(&buf_size, (uint8_t*)mq_message.data(), sizeof(uint32_t));

        if (buf_size > 0) {
            d_in_tx[j] = true;
            uint8_t control[buf_size];
            int16_t data[buf_size];
            memcpy(&control,
                   (uint8_t*)mq_message.data() + sizeof(uint32_t),
                   buf_size * sizeof(uint8_t));

            memcpy(&data,
                   (uint8_t*)mq_message.data() + sizeof(uint32_t) + buf_size * sizeof(uint8_t),
                   buf_size * sizeof(int16_t));
            for (uint32_t i = 0; i < buf_size; i++) {
                d_control_buf[j].push_back(control[i]);
                d_data_buf[j].push_back(data[i]);
            }
        } else {
            d_in_tx[j] = false;
        }
    }
}

void mmdvm_source_impl::handle_idle_time(short* out, int noutput_items, int which, bool add_tag)
{
    alternate_slots();
    add_zero_tag(0, ZERO_SAMPLES, which);
    for (int i = 0; i < noutput_items; i++) {
        out[i] = 0;
        if (i == 710 && d_burst_timer != nullptr) {
            uint64_t time = d_burst_timer->allocate_slot(d_sn, d_timing_correction, which);
            if (time > 0L && add_tag) {
                add_time_tag(time, i, which);
            }
        }
    }
}

int mmdvm_source_impl::handle_data_bursts(short* out, unsigned int n, int which, bool add_tag)
{
    int num_tags_added = 0;
    for (unsigned int i = 0; i < n; i++) {
        uint8_t control = d_control_buf[which].at(i);
        if (control == MARK_SLOT1 || control == MARK_SLOT2) {
            num_tags_added++;
        }
    }

    for (unsigned int i = 0; i < n; i++) {
        short sample = d_data_buf[which].at(i);
        uint8_t control = d_control_buf[which].at(i);
        out[i] = sample;
        if (control == MARK_SLOT1 && d_burst_timer != nullptr) {
            d_sn = 1;
            uint64_t time = d_burst_timer->allocate_slot(1, d_timing_correction, which);
            if (time > 0L && add_tag) {
                add_time_tag(time, i, which);
            }
        }
        if (control == MARK_SLOT2 && d_burst_timer != nullptr) {
            d_sn = 2;
            uint64_t time = d_burst_timer->allocate_slot(2, d_timing_correction, which);
            if (time > 0 && add_tag) {
                add_time_tag(time, i, which);
            }
        }
    }
    return num_tags_added;
}

void mmdvm_source_impl::alternate_slots()
{
    if (d_sn == 2)
        d_sn = 1;
    else
        d_sn = 2;
}

int mmdvm_source_impl::work(int noutput_items,
                             gr_vector_const_void_star& input_items,
                             gr_vector_void_star& output_items)
{
    (void)input_items;
    short* out[MAX_MMDVM_CHANNELS];
    for (int i = 0; i < d_num_channels; i++) {
        out[i] = (short*)(output_items[i]);
    }

    bool start = true;
    if (d_burst_timer != nullptr) {
        for (int i = 0; i < d_num_channels; i++) {
            if (!d_burst_timer->get_timing_initialized(i)) {
                std::cout << "Waiting for RX samples to initialize timebase" << std::endl;
                d_control_buf[i].clear();
                d_data_buf[i].clear();
                start = false;
            }
        }
    }
    if (!start && d_use_tdma)
        return 0;
    else if (!start)
        return 720; // SAMPLES_PER_SLOT

    get_zmq_message();
    if (d_timing_correction > 0) {
        struct timespec time_to_sleep = { 0, d_timing_correction };
        nanosleep(&time_to_sleep, NULL);
        d_timing_correction = 0;
    }

    for (int i = 0; i < d_num_channels; i++) {
        if (d_data_buf[i].size() < 1) {
            handle_idle_time(out[i], noutput_items, i, i == 0);
        }
    }
    for (int i = 0; i < d_num_channels; i++) {
        unsigned int n = std::min((unsigned int)d_data_buf[i].size(), (unsigned int)noutput_items);

        handle_data_bursts(out[i], n, i, i == 0);
        d_data_buf[i].erase(d_data_buf[i].begin(), d_data_buf[i].begin() + n);
        d_control_buf[i].erase(d_control_buf[i].begin(), d_control_buf[i].begin() + n);
    }
    return 720; // SAMPLES_PER_SLOT
}

void mmdvm_source_impl::add_time_tag(uint64_t nsec, int offset, int which)
{
    uint64_t intpart = nsec / 1000000000L;
    double fracpart = ((double)nsec / 1000000000.0) - (double)intpart;

    const pmt::pmt_t t_val = pmt::make_tuple(pmt::from_uint64(intpart), pmt::from_double(fracpart));
    this->add_item_tag(which, nitems_written(which) + (uint64_t)offset, TIME_TAG, t_val);
}

void mmdvm_source_impl::add_zero_tag(int offset, int num_samples, int which)
{
    const pmt::pmt_t t_val = pmt::from_uint64((uint64_t)num_samples);
    this->add_item_tag(which, nitems_written(which) + (uint64_t)offset, ZERO_TAG, t_val);
}

} // namespace qradiolink
} // namespace gr

