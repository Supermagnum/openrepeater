/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DSTAR_DECODER_IMPL_H
#define INCLUDED_QRADIOLINK_DSTAR_DECODER_IMPL_H

#include <gnuradio/qradiolink/dstar_decoder.h>
#include <gnuradio/tags.h>
#include <vector>
#include <deque>
#include <string>

namespace gr {
namespace qradiolink {

class dstar_decoder_impl : public dstar_decoder
{
private:
    // D-STAR constants
    static constexpr uint8_t FRAME_SYNC[3] = {0x55, 0x2D, 0x16};
    static constexpr uint8_t END_PATTERN[3] = {0x55, 0xC8, 0x7A};
    static constexpr int HEADER_LENGTH = 41;
    static constexpr int VOICE_FRAME_BITS = 96;
    static constexpr int SLOW_DATA_BITS = 24;
    static constexpr int VOICE_FRAME_BYTES = VOICE_FRAME_BITS / 8;
    static constexpr int SLOW_DATA_BYTES = SLOW_DATA_BITS / 8;
    
    float d_sync_threshold;
    
    // State machine
    enum state_t {
        STATE_SYNC_SEARCH,
        STATE_HEADER_RECEIVE,
        STATE_VOICE_FRAME_RECEIVE
    } d_state;
    
    std::deque<uint8_t> d_buffer;
    std::vector<uint8_t> d_current_header;
    std::vector<uint8_t> d_current_voice_frame;
    int d_bytes_received;
    int d_expected_bytes;
    int d_slow_data_bit_pos;
    std::vector<uint8_t> d_slow_data_bits;
    std::string d_decoded_message;
    
    // Golay(24,12) decoder
    uint16_t golay_decode_24bit(uint32_t codeword);
    bool check_frame_sync(const uint8_t* data);
    void decode_header(const std::vector<uint8_t>& header);
    void decode_slow_data(const std::vector<uint8_t>& slow_data);
    void assemble_message();

public:
    dstar_decoder_impl(float sync_threshold);
    ~dstar_decoder_impl();

    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DSTAR_DECODER_IMPL_H */

