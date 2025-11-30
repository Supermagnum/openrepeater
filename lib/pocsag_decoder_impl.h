/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_POCSAG_DECODER_IMPL_H
#define INCLUDED_QRADIOLINK_POCSAG_DECODER_IMPL_H

#include <gnuradio/qradiolink/pocsag_decoder.h>
#include <gnuradio/tags.h>
#include <vector>
#include <deque>

namespace gr {
namespace qradiolink {

class pocsag_decoder_impl : public pocsag_decoder
{
private:
    // POCSAG constants
    static constexpr uint32_t SYNC_CODEWORD = 0x7CD215D8;
    static constexpr uint32_t IDLE_CODEWORD = 0x7A89C197;
    static constexpr int BITS_PER_CODEWORD = 32;
    static constexpr int CODEWORDS_PER_FRAME = 2;
    static constexpr int FRAMES_PER_BATCH = 8;
    static constexpr int CODEWORDS_PER_BATCH = 1 + (FRAMES_PER_BATCH * CODEWORDS_PER_FRAME);
    
    int d_baud_rate;
    float d_sync_threshold;
    
    // State machine
    enum state_t {
        STATE_SYNC_SEARCH,
        STATE_BATCH_RECEIVE
    } d_state;
    
    std::deque<uint8_t> d_bit_buffer;
    std::vector<uint32_t> d_current_batch;
    int d_codewords_received;
    int d_bits_received;
    
    // BCH(31,21) generator polynomial
    static constexpr uint32_t BCH_GENERATOR = 0x769;
    
    // Helper functions
    bool check_sync_word(uint32_t codeword, float& confidence);
    bool check_parity(uint32_t codeword);
    uint32_t correct_bch_errors(uint32_t codeword);
    bool decode_address_codeword(uint32_t codeword, uint32_t& address, int& function_bits);
    bool decode_message_codeword(uint32_t codeword, std::vector<uint8_t>& message_bits);
    void assemble_message(const std::vector<uint8_t>& bits, std::vector<uint8_t>& message);

public:
    pocsag_decoder_impl(int baud_rate, float sync_threshold);
    ~pocsag_decoder_impl();

    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_POCSAG_DECODER_IMPL_H */

