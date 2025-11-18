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

#include <gnuradio/qradiolink/mmdvm_sink.h>
#include "mmdvm_sink_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/tags.h>
#include "../../src/bursttimer.h"
#include <zmq.hpp>
#include <algorithm>
#include <cmath>

namespace gr {
namespace qradiolink {

const uint8_t MARK_SLOT1 = 0x08U;
const uint8_t MARK_SLOT2 = 0x04U;
const uint8_t MARK_NONE = 0x00U;

static const pmt::pmt_t TIME_TAG = pmt::string_to_symbol("rx_time");
static const pmt::pmt_t RSSI_TAG = pmt::string_to_symbol("RSSI");

mmdvm_sink::sptr mmdvm_sink::make(BurstTimer* burst_timer,
                                  uint8_t cn,
                                  bool multi_channel,
                                  bool use_tdma)
{
    return gnuradio::get_initial_sptr(new mmdvm_sink_impl(burst_timer, cn, multi_channel, use_tdma));
}

mmdvm_sink_impl::mmdvm_sink_impl(BurstTimer* burst_timer,
                                 uint8_t cn,
                                 bool multi_channel,
                                 bool use_tdma)
    : mmdvm_sink("mmdvm_sink",
                gr::io_signature::make(cn, cn, sizeof(short)),
                gr::io_signature::make(0, 0, 0)),
      d_burst_timer(burst_timer),
      d_num_channels(cn),
      d_use_tdma(use_tdma)
{
    for (int i = 0; i < d_num_channels; i++) {
        d_zmqcontext[i] = zmq::context_t(1);
        d_zmqsocket[i] = zmq::socket_t(d_zmqcontext[i], ZMQ_PUSH);
        d_zmqsocket[i].set(zmq::sockopt::sndhwm, 100);
        d_zmqsocket[i].set(zmq::sockopt::linger, 0);
        int socket_no = multi_channel ? i + 1 : 0;
        d_zmqsocket[i].bind("ipc:///tmp/mmdvm-rx" + std::to_string(socket_no) + ".ipc");
        d_last_rssi_on_timeslot[i] = 0;
        d_slot_sample_counter[i] = 0;
        d_data_buf[i].reserve(2 * 720); // SAMPLES_PER_SLOT
        d_control_buf[i].reserve(2 * 720);
        d_rssi[i].reserve(720);
    }

    set_max_noutput_items(720); // SAMPLES_PER_SLOT
}

mmdvm_sink_impl::~mmdvm_sink_impl() {}

int mmdvm_sink_impl::work(int noutput_items,
                          gr_vector_const_void_star& input_items,
                          gr_vector_void_star& output_items)
{
    (void)output_items;
    short* in[MAX_MMDVM_CHANNELS];
    for (int i = 0; i < d_num_channels; i++) {
        in[i] = (short*)(input_items[i]);
    }

    for (int chan = 0; chan < d_num_channels; chan++) {
        std::vector<gr::tag_t> tags;
        std::vector<gr::tag_t> rssi_tags;
        get_tags_in_window(tags, chan, 0, noutput_items, TIME_TAG);
        get_tags_in_window(rssi_tags, chan, 0, noutput_items, RSSI_TAG);

        if (!tags.empty()) {
            std::sort(tags.begin(), tags.end(), gr::tag_t::offset_compare);
        }
        if (!rssi_tags.empty()) {
            std::sort(rssi_tags.begin(), rssi_tags.end(), gr::tag_t::offset_compare);
        }

        uint64_t nitems = nitems_read(chan);
        int slot_no = 0;
        for (int i = 0; i < noutput_items; i++) {
            for (gr::tag_t& tag : tags) {
                if (tag.offset == nitems + (uint64_t)i) {
                    pmt::pmt_t t_val = tag.value;
                    uint64_t intpart = pmt::to_uint64(pmt::tuple_ref(t_val, 0));
                    double fracpart = pmt::to_double(pmt::tuple_ref(t_val, 1));
                    uint64_t nsec = intpart * 1000000000L + (uint64_t)(fracpart * 1000000000.0);
                    slot_no = d_burst_timer->check_time(chan);
                    if (slot_no > 0) {
                        d_burst_timer->set_timer(nsec, chan);
                        d_slot_sample_counter[chan] = 0;
                    }
                    break;
                }
            }

            for (gr::tag_t& tag : rssi_tags) {
                if (tag.offset == nitems + (uint64_t)i) {
                    float rssi_db = pmt::to_float(tag.value);
                    d_rssi[chan].push_back((uint64_t)rssi_db);
                    d_last_rssi_on_timeslot[chan] = d_slot_sample_counter[chan];
                    break;
                }
            }

            short sample = in[chan][i];
            d_data_buf[chan].push_back(sample);

            if (slot_no > 0) {
                d_control_buf[chan].push_back(slot_no == 1 ? MARK_SLOT1 : MARK_SLOT2);
            } else {
                d_control_buf[chan].push_back(MARK_NONE);
            }
            d_slot_sample_counter[chan]++;
        }

        if (d_data_buf[chan].size() >= 720) { // SAMPLES_PER_SLOT
            uint32_t buf_size = d_data_buf[chan].size();
            size_t msg_size = sizeof(uint32_t) + buf_size * sizeof(uint8_t) + buf_size * sizeof(int16_t);
            zmq::message_t msg(msg_size);

            memcpy((uint8_t*)msg.data(), &buf_size, sizeof(uint32_t));
            memcpy((uint8_t*)msg.data() + sizeof(uint32_t), d_control_buf[chan].data(), buf_size * sizeof(uint8_t));
            memcpy((uint8_t*)msg.data() + sizeof(uint32_t) + buf_size * sizeof(uint8_t),
                   d_data_buf[chan].data(),
                   buf_size * sizeof(int16_t));

            d_zmqsocket[chan].send(msg, zmq::send_flags::dontwait);

            d_data_buf[chan].clear();
            d_control_buf[chan].clear();
            d_rssi[chan].clear();
        }
    }

    return noutput_items;
}

} // namespace qradiolink
} // namespace gr

