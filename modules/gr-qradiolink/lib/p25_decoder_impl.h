/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_P25_DECODER_IMPL_H
#define INCLUDED_QRADIOLINK_P25_DECODER_IMPL_H

#include <gnuradio/qradiolink/p25_decoder.h>
#include <gnuradio/tags.h>
#include <vector>
#include <deque>

namespace gr {
namespace qradiolink {

class p25_decoder_impl : public p25_decoder
{
private:
    // P25 constants
    static constexpr uint64_t FRAME_SYNC = 0x5575F5FF77FFULL; // 48 bits
    static constexpr int SYNC_BYTES = 6; // 48 bits
    static constexpr int NID_LENGTH = 64; // bits
    static constexpr int NID_BYTES = 8; // 64 bits
    static constexpr int LDU1_LENGTH = 720; // bits
    static constexpr int LDU2_LENGTH = 720; // bits
    static constexpr int LDU_BYTES = 90; // 720 bits
    static constexpr int IMBE_FRAMES_PER_SUPERFRAME = 9;
    static constexpr int IMBE_FRAME_BYTES = 11;
    
    float d_sync_threshold;
    
    // State machine
    enum state_t {
        STATE_SYNC_SEARCH,
        STATE_NID_RECEIVE,
        STATE_LDU1_RECEIVE,
        STATE_LDU2_RECEIVE
    } d_state;
    
    std::deque<uint8_t> d_buffer;
    std::vector<uint8_t> d_current_nid;
    std::vector<uint8_t> d_current_ldu1;
    std::vector<uint8_t> d_current_ldu2;
    int d_bytes_received;
    int d_expected_bytes;
    
    uint16_t d_nac;
    uint32_t d_source_id;
    uint32_t d_destination_id;
    uint16_t d_talkgroup_id;
    bool d_encrypted;
    
    // FEC decoders
    bool check_frame_sync(const uint8_t* data);
    uint16_t bch_decode_63_16(uint64_t nid);
    uint16_t golay_decode_24_12(uint32_t codeword);
    void decode_nid(const std::vector<uint8_t>& nid);
    void decode_ldu1(const std::vector<uint8_t>& ldu1);
    void decode_ldu2(const std::vector<uint8_t>& ldu2);
    void trellis_decode(const std::vector<uint8_t>& input, std::vector<uint8_t>& output);

public:
    p25_decoder_impl(float sync_threshold);
    ~p25_decoder_impl();

    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_P25_DECODER_IMPL_H */

