/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DSTAR_ENCODER_IMPL_H
#define INCLUDED_QRADIOLINK_DSTAR_ENCODER_IMPL_H

#include <gnuradio/qradiolink/dstar_encoder.h>
#include <vector>
#include <deque>
#include <string>

namespace gr {
namespace qradiolink {

class dstar_encoder_impl : public dstar_encoder
{
private:
    // D-STAR constants
    static constexpr uint8_t FRAME_SYNC[3] = {0x55, 0x2D, 0x16};
    static constexpr uint8_t END_PATTERN[3] = {0x55, 0xC8, 0x7A};
    static constexpr int HEADER_LENGTH = 41;
    static constexpr int VOICE_FRAME_BITS = 96;
    static constexpr int SLOW_DATA_BITS = 24;
    static constexpr int FRAME_DURATION_MS = 20;
    static constexpr int SLOW_DATA_RATE_BPS = 1200;
    
    std::string d_my_callsign;
    std::string d_your_callsign;
    std::string d_rpt1_callsign;
    std::string d_rpt2_callsign;
    std::string d_message_text;
    
    // State machine
    enum state_t {
        STATE_HEADER,
        STATE_VOICE_FRAMES,
        STATE_END
    } d_state;
    
    bool d_header_sent;
    std::vector<uint8_t> d_header;
    std::deque<uint8_t> d_voice_queue;
    int d_frame_count;
    int d_slow_data_bit_pos;
    std::vector<uint8_t> d_slow_data_bits;
    
    // Golay(24,12) generator matrix (simplified - full implementation needs lookup table)
    static constexpr uint32_t GOLAY_GENERATOR = 0xC75; // Placeholder
    
    // Helper functions
    void build_header();
    uint32_t golay_encode_12bit(uint16_t data);
    void encode_slow_data();
    void generate_pn9_scrambler(std::vector<uint8_t>& sequence, int length);

public:
    dstar_encoder_impl(const std::string& my_callsign,
                       const std::string& your_callsign,
                       const std::string& rpt1_callsign,
                       const std::string& rpt2_callsign,
                       const std::string& message_text);
    ~dstar_encoder_impl();

    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DSTAR_ENCODER_IMPL_H */

