/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/qradiolink/pocsag_decoder.h>
#include "pocsag_decoder_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/tags.h>
#include <cstring>
#include <algorithm>
#include <cstdint>
#include <cmath>

namespace gr {
namespace qradiolink {

static const pmt::pmt_t ADDRESS_TAG = pmt::string_to_symbol("address");
static const pmt::pmt_t FUNCTION_TAG = pmt::string_to_symbol("function");
static const pmt::pmt_t MESSAGE_TAG = pmt::string_to_symbol("message");

pocsag_decoder::sptr pocsag_decoder::make(int baud_rate, float sync_threshold)
{
    return gnuradio::get_initial_sptr(new pocsag_decoder_impl(baud_rate, sync_threshold));
}

pocsag_decoder_impl::pocsag_decoder_impl(int baud_rate, float sync_threshold)
    : pocsag_decoder("pocsag_decoder",
                     gr::io_signature::make(1, 1, sizeof(unsigned char)),
                     gr::io_signature::make(1, 1, sizeof(unsigned char)),
                     baud_rate,
                     sync_threshold),
      d_baud_rate(baud_rate),
      d_sync_threshold(sync_threshold),
      d_state(STATE_SYNC_SEARCH),
      d_codewords_received(0),
      d_bits_received(0)
{
    // Validate baud rate
    if (baud_rate != 512 && baud_rate != 1200 && baud_rate != 2400) {
        throw std::invalid_argument("Baud rate must be 512, 1200, or 2400");
    }
    
    // Validate sync threshold
    if (sync_threshold < 0.0f || sync_threshold > 1.0f) {
        throw std::invalid_argument("Sync threshold must be between 0.0 and 1.0");
    }
    
    d_current_batch.reserve(CODEWORDS_PER_BATCH);
}

pocsag_decoder_impl::~pocsag_decoder_impl() {}

bool pocsag_decoder_impl::check_parity(uint32_t codeword)
{
    int ones = 0;
    for (int i = 0; i < 32; i++) {
        if (codeword & (1U << i)) {
            ones++;
        }
    }
    return (ones % 2) == 0; // Even parity
}

uint32_t pocsag_decoder_impl::correct_bch_errors(uint32_t codeword)
{
    // BCH(31,21) can correct up to 2 errors
    // For simplicity, we'll check for single-bit errors first
    
    // Extract data and parity
    uint32_t data = (codeword >> 1) & 0xFFFFF; // 20 bits for message, or 19+2 for address
    uint32_t received_parity = (codeword >> 22) & 0x3FF; // 10 bits
    
    // Compute expected parity
    uint32_t expected_parity = 0;
    uint32_t shifted = data << 10;
    uint32_t generator = BCH_GENERATOR << 21;
    uint32_t remainder = shifted;
    
    for (int i = 30; i >= 10; i--) {
        if (remainder & (1U << i)) {
            remainder ^= (generator >> (30 - i));
        }
    }
    expected_parity = remainder & 0x3FF;
    
    // If parity matches, no errors (or uncorrectable errors)
    if (received_parity == expected_parity) {
        return codeword; // No correction needed (or beyond our correction capability)
    }
    
    // Try single-bit error correction
    // This is a simplified approach - full BCH decoder would be more complex
    // For now, we'll just check parity and return corrected codeword if we can determine it
    
    // Compute syndrome
    uint32_t syndrome = received_parity ^ expected_parity;
    
    // Simple error correction: try flipping bits in data
    // This is not a complete BCH decoder, but handles common cases
    for (int i = 0; i < 20; i++) {
        uint32_t test_data = data ^ (1U << i);
        uint32_t test_shifted = test_data << 10;
        uint32_t test_remainder = test_shifted;
        
        for (int j = 30; j >= 10; j--) {
            if (test_remainder & (1U << j)) {
                test_remainder ^= (generator >> (30 - j));
            }
        }
        uint32_t test_parity = test_remainder & 0x3FF;
        
        if (test_parity == received_parity) {
            // Found correctable error
            uint32_t corrected = codeword;
            corrected &= ~(0xFFFFF << 1); // Clear data bits
            corrected |= (test_data << 1);
            return corrected;
        }
    }
    
    // Could not correct, return original
    return codeword;
}

bool pocsag_decoder_impl::check_sync_word(uint32_t codeword, float& confidence)
{
    // Check if codeword matches sync word
    // Allow some bit errors for soft-decision decoding
    int matching_bits = 0;
    for (int i = 0; i < 32; i++) {
        if (((codeword >> i) & 1) == ((SYNC_CODEWORD >> i) & 1)) {
            matching_bits++;
        }
    }
    
    confidence = static_cast<float>(matching_bits) / 32.0f;
    return confidence >= d_sync_threshold;
}

bool pocsag_decoder_impl::decode_address_codeword(uint32_t codeword, uint32_t& address, int& function_bits)
{
    // Address codeword: bit 0 = 0
    if ((codeword & 1) != 0) {
        return false; // Not an address codeword
    }
    
    // Correct BCH errors
    codeword = correct_bch_errors(codeword);
    
    // Check parity
    if (!check_parity(codeword)) {
        return false; // Parity error
    }
    
    // Extract address bits (bits 1-19) and function bits (bits 20-21)
    uint32_t addr_bits = (codeword >> 1) & 0x7FFFF; // 19 bits
    function_bits = (codeword >> 20) & 0x3; // 2 bits
    
    // Reconstruct 21-bit address
    // Frame number is determined by position in batch, not stored in codeword
    // For now, we'll use the lower 19 bits and let the caller determine frame
    address = addr_bits << 3; // Will need frame number added
    
    return true;
}

bool pocsag_decoder_impl::decode_message_codeword(uint32_t codeword, std::vector<uint8_t>& message_bits)
{
    // Message codeword: bit 0 = 1
    if ((codeword & 1) == 0) {
        return false; // Not a message codeword
    }
    
    // Correct BCH errors
    codeword = correct_bch_errors(codeword);
    
    // Check parity
    if (!check_parity(codeword)) {
        return false; // Parity error
    }
    
    // Extract message bits (bits 1-20)
    message_bits.clear();
    for (int i = 1; i <= 20; i++) {
        message_bits.push_back((codeword >> i) & 1);
    }
    
    return true;
}

void pocsag_decoder_impl::assemble_message(const std::vector<uint8_t>& bits, std::vector<uint8_t>& message)
{
    // Convert bit stream to bytes (7-bit ASCII for alphanumeric, 4-bit BCD for numeric)
    // For simplicity, we'll decode as 7-bit ASCII
    
    message.clear();
    
    for (size_t i = 0; i < bits.size(); i += 7) {
        if (i + 7 > bits.size()) {
            break; // Incomplete character
        }
        
        uint8_t ch = 0;
        for (int j = 0; j < 7; j++) {
            if (bits[i + j]) {
                ch |= (1U << (6 - j)); // MSB first
            }
        }
        
        // POCSAG uses 7-bit ASCII with some special mappings
        // For now, output as-is (caller can handle special cases)
        message.push_back(ch);
    }
}

int pocsag_decoder_impl::work(int noutput_items,
                               gr_vector_const_void_star& input_items,
                               gr_vector_void_star& output_items)
{
    const unsigned char* in = (const unsigned char*)input_items[0];
    unsigned char* out = (unsigned char*)output_items[0];
    
    int out_idx = 0;
    
    // Collect input bits
    for (int i = 0; i < noutput_items; i++) {
        d_bit_buffer.push_back(in[i] != 0 ? 1 : 0);
    }
    
    // Keep buffer size reasonable
    if (d_bit_buffer.size() > static_cast<size_t>(CODEWORDS_PER_BATCH * BITS_PER_CODEWORD + 100)) {
        // Remove old bits if buffer too large
        size_t to_remove = d_bit_buffer.size() - (CODEWORDS_PER_BATCH * BITS_PER_CODEWORD);
        for (size_t i = 0; i < to_remove; i++) {
            d_bit_buffer.pop_front();
        }
    }
    
    if (d_state == STATE_SYNC_SEARCH) {
        // Search for sync word
        if (d_bit_buffer.size() >= static_cast<size_t>(BITS_PER_CODEWORD)) {
            // Try to find sync word at various positions
            for (size_t start = 0; start <= d_bit_buffer.size() - BITS_PER_CODEWORD; start++) {
                // Assemble codeword from bits
                uint32_t codeword = 0;
                for (int i = 0; i < BITS_PER_CODEWORD; i++) {
                    if (d_bit_buffer[start + i]) {
                        codeword |= (1U << (31 - i));
                    }
                }
                
                float confidence;
                if (check_sync_word(codeword, confidence)) {
                    // Found sync word
                    // Remove bits before sync word
                    for (size_t i = 0; i < start; i++) {
                        d_bit_buffer.pop_front();
                    }
                    
                    d_state = STATE_BATCH_RECEIVE;
                    d_current_batch.clear();
                    d_current_batch.push_back(codeword); // Add sync codeword
                    d_codewords_received = 1;
                    d_bits_received = BITS_PER_CODEWORD;
                    break;
                }
            }
        }
    }
    
    if (d_state == STATE_BATCH_RECEIVE) {
        // Receive batch codewords
        while (d_bit_buffer.size() >= static_cast<size_t>(BITS_PER_CODEWORD) &&
               d_codewords_received < CODEWORDS_PER_BATCH &&
               out_idx < noutput_items) {
            
            // Assemble codeword
            uint32_t codeword = 0;
            for (int i = 0; i < BITS_PER_CODEWORD; i++) {
                if (d_bit_buffer[i]) {
                    codeword |= (1U << (31 - i));
                }
            }
            
            // Remove bits from buffer
            for (int i = 0; i < BITS_PER_CODEWORD; i++) {
                d_bit_buffer.pop_front();
            }
            
            d_current_batch.push_back(codeword);
            d_codewords_received++;
            d_bits_received += BITS_PER_CODEWORD;
            
            // Check if batch is complete
            if (d_codewords_received >= CODEWORDS_PER_BATCH) {
                // Process batch
                uint32_t address = 0;
                int function_bits = 0;
                std::vector<uint8_t> message_bits;
                bool address_found = false;
                int address_frame = -1;
                
                // Find address codeword and collect message codewords
                for (int frame = 0; frame < FRAMES_PER_BATCH; frame++) {
                    int codeword_idx = 1 + (frame * CODEWORDS_PER_FRAME); // Skip sync
                    
                    if (codeword_idx < static_cast<int>(d_current_batch.size())) {
                        uint32_t cw1 = d_current_batch[codeword_idx];
                        uint32_t cw2 = (codeword_idx + 1 < static_cast<int>(d_current_batch.size())) ?
                                       d_current_batch[codeword_idx + 1] : IDLE_CODEWORD;
                        
                        // Check first codeword
                        if (!address_found && (cw1 & 1) == 0) {
                            // Might be address codeword
                            if (decode_address_codeword(cw1, address, function_bits)) {
                                address_found = true;
                                address_frame = frame;
                                address |= frame; // Add frame number to address
                            }
                        }
                        
                        // Collect message codewords
                        if (address_found && frame >= address_frame) {
                            std::vector<uint8_t> bits;
                            if (decode_message_codeword(cw1, bits)) {
                                message_bits.insert(message_bits.end(), bits.begin(), bits.end());
                            }
                            if (decode_message_codeword(cw2, bits)) {
                                message_bits.insert(message_bits.end(), bits.begin(), bits.end());
                            }
                        }
                    }
                }
                
                // Output decoded message if found
                if (address_found && !message_bits.empty()) {
                    std::vector<uint8_t> message;
                    assemble_message(message_bits, message);
                    
                    if (!message.empty() && out_idx < noutput_items) {
                        // Output message bytes
                        for (size_t i = 0; i < message.size() && out_idx < noutput_items; i++) {
                            out[out_idx++] = message[i];
                        }
                        
                        // Add tags for metadata
                        if (out_idx > 0) {
                            add_item_tag(0, nitems_written(0),
                                        ADDRESS_TAG, pmt::from_uint64(address));
                            add_item_tag(0, nitems_written(0),
                                        FUNCTION_TAG, pmt::from_long(function_bits));
                            add_item_tag(0, nitems_written(0),
                                        MESSAGE_TAG, pmt::make_blob(message.data(), message.size()));
                        }
                    }
                }
                
                // Reset for next batch
                d_current_batch.clear();
                d_codewords_received = 0;
                d_bits_received = 0;
                d_state = STATE_SYNC_SEARCH;
            }
        }
    }
    
    // If no output produced, return 0 to allow flowgraph to make progress
    // But we've consumed input, so we need to handle this carefully
    // For sync_block, we should consume noutput_items input and produce noutput_items output
    // If we can't produce output, we still need to consume input
    
    // Pad output if needed
    while (out_idx < noutput_items) {
        out[out_idx++] = 0; // Null byte
    }
    
    return noutput_items;
}

} // namespace qradiolink
} // namespace gr

