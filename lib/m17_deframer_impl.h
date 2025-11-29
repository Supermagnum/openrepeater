/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_M17_DEFRAMER_IMPL_H
#define INCLUDED_QRADIOLINK_M17_DEFRAMER_IMPL_H

#include <gnuradio/qradiolink/m17_deframer.h>
#include <gnuradio/tags.h>
#include <vector>
#include <deque>

namespace gr {
namespace qradiolink {

class m17_deframer_impl : public m17_deframer
{
private:
    // M17 sync words
    static constexpr uint16_t SYNC_LSF = 0xDF55;      // Link Setup Frame
    static constexpr uint16_t SYNC_STREAM = 0xDF55;   // Stream frame (same as LSF)
    static constexpr uint16_t SYNC_PACKET = 0x9FF6;   // Packet frame
    
    int d_max_frame_length;
    std::deque<uint8_t> d_buffer;
    enum state_t {
        STATE_SYNC_SEARCH,
        STATE_FRAME_RECEIVE
    } d_state;
    int d_frame_bytes_received;
    int d_frame_length;
    uint16_t d_sync_word;
    
    bool find_sync_word();

public:
    m17_deframer_impl(int max_frame_length);
    ~m17_deframer_impl() override;

    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_M17_DEFRAMER_IMPL_H */

