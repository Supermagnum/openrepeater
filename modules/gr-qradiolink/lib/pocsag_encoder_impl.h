/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_POCSAG_ENCODER_IMPL_H
#define INCLUDED_QRADIOLINK_POCSAG_ENCODER_IMPL_H

#include <gnuradio/qradiolink/pocsag_encoder.h>
#include <vector>
#include <deque>

namespace gr {
namespace qradiolink {

class pocsag_encoder_impl : public pocsag_encoder
{
private:
    // POCSAG constants
    static constexpr uint32_t SYNC_CODEWORD = 0x7CD215D8;
    static constexpr uint32_t IDLE_CODEWORD = 0x7A89C197;
    static constexpr int PREAMBLE_BITS = 576;
    static constexpr int BITS_PER_CODEWORD = 32;
    static constexpr int CODEWORDS_PER_FRAME = 2;
    static constexpr int FRAMES_PER_BATCH = 8;
    static constexpr int CODEWORDS_PER_BATCH = 1 + (FRAMES_PER_BATCH * CODEWORDS_PER_FRAME); // 1 sync + 16 data
    
    int d_baud_rate;
    uint32_t d_address;
    int d_function_bits;
    
    // State machine
    enum state_t {
        STATE_PREAMBLE,
        STATE_BATCHES
    } d_state;
    
    int d_preamble_bits_sent;
    std::deque<uint8_t> d_message_queue;
    std::deque<uint32_t> d_codeword_queue;
    int d_current_batch;
    int d_current_frame;
    int d_current_codeword_in_frame;
    bool d_preamble_sent;
    
    // BCH(31,21) generator polynomial
    // g(x) = x^10 + x^9 + x^8 + x^6 + x^5 + x^3 + 1
    static constexpr uint32_t BCH_GENERATOR = 0x769; // 0b11101101001
    
    // Helper functions
    uint32_t compute_bch_parity(uint32_t data);
    uint32_t create_address_codeword(uint32_t address, int function_bits);
    uint32_t create_message_codeword(const std::vector<uint8_t>& message, int start_bit);
    void encode_message_to_codewords(const std::vector<uint8_t>& message);
    void generate_preamble_bits(std::vector<uint8_t>& output, int count);
    void generate_batch_bits(std::vector<uint8_t>& output, int count);

public:
    pocsag_encoder_impl(int baud_rate, uint32_t address, int function_bits);
    ~pocsag_encoder_impl();

    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_POCSAG_ENCODER_IMPL_H */

