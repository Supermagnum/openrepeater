/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_MMDVM_SOURCE_IMPL_H
#define INCLUDED_QRADIOLINK_MMDVM_SOURCE_IMPL_H

#include <gnuradio/qradiolink/mmdvm_source.h>
#include <gnuradio/tags.h>
#include <zmq.hpp>
#include <chrono>
#include <vector>
#include <mutex>

// Forward declaration - BurstTimer is application-level
class BurstTimer;

// Constants from src/bursttimer.h and src/config_mmdvm.h
#ifndef MAX_MMDVM_CHANNELS
#define MAX_MMDVM_CHANNELS 7
#endif

namespace gr {
namespace qradiolink {

class mmdvm_source_impl : public mmdvm_source
{
private:
    void get_zmq_message();
    void add_time_tag(uint64_t nsec, int offset, int which);
    void add_zero_tag(int offset, int num_samples, int which);
    void handle_idle_time(short* out, int noutput_items, int which, bool add_tag);
    int handle_data_bursts(short* out, unsigned int n, int which, bool add_tag);
    void alternate_slots();

    BurstTimer* d_burst_timer;
    zmq::context_t d_zmqcontext[MAX_MMDVM_CHANNELS];
    zmq::socket_t d_zmqsocket[MAX_MMDVM_CHANNELS];
    std::vector<uint8_t> d_control_buf[MAX_MMDVM_CHANNELS];
    std::vector<int16_t> d_data_buf[MAX_MMDVM_CHANNELS];
    bool d_in_tx[MAX_MMDVM_CHANNELS];
    gr::thread::mutex d_mutex;
    int d_num_channels;
    int64_t d_timing_correction;
    int d_sn;
    bool d_add_time_tag;
    bool d_use_tdma;

public:
    mmdvm_source_impl(BurstTimer* burst_timer,
                      uint8_t cn = 0,
                      bool multi_channel = false,
                      bool use_tdma = true);
    ~mmdvm_source_impl() override;

    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_MMDVM_SOURCE_IMPL_H */

