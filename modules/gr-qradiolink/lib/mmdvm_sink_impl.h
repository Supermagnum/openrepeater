/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_MMDVM_SINK_IMPL_H
#define INCLUDED_QRADIOLINK_MMDVM_SINK_IMPL_H

#include <gnuradio/qradiolink/mmdvm_sink.h>
#include <gnuradio/tags.h>
#include <zmq.hpp>
#include <vector>
#include <cmath>

// Forward declaration - BurstTimer is application-level
class BurstTimer;

#ifndef MAX_MMDVM_CHANNELS
#define MAX_MMDVM_CHANNELS 7
#endif

namespace gr {
namespace qradiolink {

class mmdvm_sink_impl : public mmdvm_sink
{
private:
    BurstTimer* d_burst_timer;
    zmq::context_t d_zmqcontext[MAX_MMDVM_CHANNELS];
    zmq::socket_t d_zmqsocket[MAX_MMDVM_CHANNELS];
    std::vector<uint8_t> d_control_buf[MAX_MMDVM_CHANNELS];
    std::vector<int16_t> d_data_buf[MAX_MMDVM_CHANNELS];
    int d_num_channels;
    std::vector<uint64_t> d_rssi[MAX_MMDVM_CHANNELS];
    int d_last_rssi_on_timeslot[MAX_MMDVM_CHANNELS];
    uint64_t d_slot_sample_counter[MAX_MMDVM_CHANNELS];
    bool d_use_tdma;

public:
    mmdvm_sink_impl(BurstTimer* burst_timer,
                   uint8_t cn = 0,
                   bool multi_channel = true,
                   bool use_tdma = true);
    ~mmdvm_sink_impl() override;

    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_MMDVM_SINK_IMPL_H */

