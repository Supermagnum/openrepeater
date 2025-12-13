/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_YSF_DECODER_IMPL_H
#define INCLUDED_QRADIOLINK_YSF_DECODER_IMPL_H

#include <gnuradio/qradiolink/ysf_decoder.h>
#include <gnuradio/tags.h>
#include <vector>
#include <deque>
#include <string>

namespace gr {
namespace qradiolink {

class ysf_decoder_impl : public ysf_decoder
{
private:
    // YSF constants
    static constexpr uint16_t FRAME_SYNC = 0xD471;
    static constexpr int FICH_LENGTH = 5;  // 5 bytes = 40 bits
    static constexpr int FRAME_LENGTH = 180; // 180 bytes per frame
    static constexpr int CALLSIGN_LENGTH = 10;
    static constexpr int VOICE_FRAME_BYTES = 144;
    
    float d_sync_threshold;
    
    // State machine
    enum state_t {
        STATE_SYNC_SEARCH,
        STATE_FICH_RECEIVE,
        STATE_VOICE_FRAME_RECEIVE
    } d_state;
    
    std::deque<uint8_t> d_buffer;
    std::vector<uint8_t> d_current_fich;
    std::vector<uint8_t> d_current_voice_frame;
    int d_bytes_received;
    int d_expected_bytes;
    std::string d_source_callsign;
    std::string d_destination_callsign;
    uint32_t d_radio_id;
    uint32_t d_group_id;
    
    // Golay FEC decoder
    uint8_t golay_decode_20bit(uint32_t codeword);  // Golay(20,8)
    uint16_t golay_decode_23bit(uint32_t codeword); // Golay(23,12)
    bool check_crc16_ccitt(const uint8_t* data, int length, uint16_t received_crc);
    bool check_frame_sync(const uint8_t* data);
    void decode_fich(const std::vector<uint8_t>& fich);
    void decode_callsign(const uint8_t* data, std::string& callsign);

public:
    ysf_decoder_impl(float sync_threshold);
    ~ysf_decoder_impl();

    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_YSF_DECODER_IMPL_H */

