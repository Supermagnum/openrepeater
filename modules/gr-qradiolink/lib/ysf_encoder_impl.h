/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_YSF_ENCODER_IMPL_H
#define INCLUDED_QRADIOLINK_YSF_ENCODER_IMPL_H

#include <gnuradio/qradiolink/ysf_encoder.h>
#include <vector>
#include <deque>
#include <string>

namespace gr {
namespace qradiolink {

class ysf_encoder_impl : public ysf_encoder
{
private:
    // YSF constants
    static constexpr uint16_t FRAME_SYNC = 0xD471;
    static constexpr int FICH_LENGTH = 5;  // 5 bytes = 40 bits
    static constexpr int FRAME_LENGTH = 180; // 180 bytes per frame
    static constexpr int CALLSIGN_LENGTH = 10;
    
    std::string d_source_callsign;
    std::string d_destination_callsign;
    uint32_t d_radio_id;
    uint32_t d_group_id;
    
    // State machine
    enum state_t {
        STATE_FICH,
        STATE_VOICE_FRAME
    } d_state;
    
    int d_frame_count;
    std::deque<uint8_t> d_voice_queue;
    std::vector<uint8_t> d_fich;
    
    // Golay FEC
    uint32_t golay_encode_8bit(uint8_t data);  // Golay(20,8)
    uint32_t golay_encode_12bit(uint16_t data); // Golay(23,12)
    uint16_t compute_crc16_ccitt(const uint8_t* data, int length);
    void build_fich();
    void encode_callsign(const std::string& callsign, std::vector<uint8_t>& output);

public:
    ysf_encoder_impl(const std::string& source_callsign,
                     const std::string& destination_callsign,
                     uint32_t radio_id,
                     uint32_t group_id);
    ~ysf_encoder_impl();

    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_YSF_ENCODER_IMPL_H */

