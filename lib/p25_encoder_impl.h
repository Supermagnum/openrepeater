/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_P25_ENCODER_IMPL_H
#define INCLUDED_QRADIOLINK_P25_ENCODER_IMPL_H

#include <gnuradio/qradiolink/p25_encoder.h>
#include <vector>
#include <deque>

namespace gr {
namespace qradiolink {

class p25_encoder_impl : public p25_encoder
{
private:
    // P25 constants
    static constexpr uint64_t FRAME_SYNC = 0x5575F5FF77FFULL; // 48 bits
    static constexpr int NID_LENGTH = 64; // bits
    static constexpr int LDU1_LENGTH = 720; // bits
    static constexpr int LDU2_LENGTH = 720; // bits
    static constexpr int IMBE_FRAMES_PER_SUPERFRAME = 9;
    static constexpr int IMBE_FRAME_BITS = 88;
    static constexpr int IMBE_FRAME_BYTES = 11;
    
    uint16_t d_nac; // Network Access Code (12 bits)
    uint32_t d_source_id;
    uint32_t d_destination_id;
    uint16_t d_talkgroup_id;
    
    // State machine
    enum state_t {
        STATE_NID,
        STATE_LDU1,
        STATE_LDU2
    } d_state;
    
    int d_frame_count;
    std::deque<uint8_t> d_voice_queue;
    std::vector<uint8_t> d_nid;
    std::vector<uint8_t> d_ldu1;
    std::vector<uint8_t> d_ldu2;
    
    // FEC encoders
    uint64_t build_nid(); // Build NID with BCH(63,16)
    uint32_t bch_encode_63_16(uint16_t data);
    uint32_t golay_encode_24_12(uint16_t data);
    void build_ldu1();
    void build_ldu2();
    void trellis_encode(const std::vector<uint8_t>& input, std::vector<uint8_t>& output);

public:
    p25_encoder_impl(uint16_t nac,
                     uint32_t source_id,
                     uint32_t destination_id,
                     uint16_t talkgroup_id);
    ~p25_encoder_impl();

    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_P25_ENCODER_IMPL_H */

