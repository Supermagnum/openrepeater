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

#include <gnuradio/qradiolink/pocsag_encoder.h>
#include "pocsag_encoder_impl.h"
#include <gnuradio/io_signature.h>
#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <cstdint>

namespace gr {
namespace qradiolink {

pocsag_encoder::sptr pocsag_encoder::make(int baud_rate, uint32_t address, int function_bits)
{
    return gnuradio::get_initial_sptr(new pocsag_encoder_impl(baud_rate, address, function_bits));
}

pocsag_encoder_impl::pocsag_encoder_impl(int baud_rate, uint32_t address, int function_bits)
    : pocsag_encoder("pocsag_encoder",
                     gr::io_signature::make(1, 1, sizeof(unsigned char)),
                     gr::io_signature::make(1, 1, sizeof(unsigned char)),
                     baud_rate,
                     address,
                     function_bits),
      d_baud_rate(baud_rate),
      d_address(address),
      d_function_bits(function_bits & 0x3), // Ensure 2 bits only
      d_state(STATE_PREAMBLE),
      d_preamble_bits_sent(0),
      d_current_batch(0),
      d_current_frame(0),
      d_current_codeword_in_frame(0),
      d_preamble_sent(false)
{
    // Validate baud rate
    if (baud_rate != 512 && baud_rate != 1200 && baud_rate != 2400) {
        throw std::invalid_argument("Baud rate must be 512, 1200, or 2400");
    }
    
    // Validate address (21 bits max)
    if (address > 0x1FFFFF) {
        throw std::invalid_argument("Address must be 21 bits (max 0x1FFFFF)");
    }
    
    // Validate function bits (2 bits max)
    if (d_function_bits > 3) {
        throw std::invalid_argument("Function bits must be 0-3");
    }
}

pocsag_encoder_impl::~pocsag_encoder_impl() {}

uint32_t pocsag_encoder_impl::compute_bch_parity(uint32_t data)
{
    // BCH(31,21): 21 data bits + 10 parity bits = 31 bits total
    // Generator polynomial: g(x) = x^10 + x^9 + x^8 + x^6 + x^5 + x^3 + 1
    // Binary: 11101101001 = 0x769
    
    // Ensure data is 21 bits max
    data = data & 0x1FFFFF;
    
    // Shift data left by 10 bits to make room for parity
    uint32_t shifted = data << 10;
    
    // Polynomial division to compute parity
    // Generator polynomial: 0x769 (11 bits, but we use it as 10-bit for parity)
    uint32_t generator = BCH_GENERATOR; // 0x769 = 0b11101101001
    uint32_t remainder = shifted;
    
    // Perform polynomial division (XOR operations)
    // Process from MSB (bit 30) down to bit 10
    for (int i = 30; i >= 10; i--) {
        if (remainder & (1U << i)) {
            // XOR with generator polynomial shifted to align at bit i
            // Generator is 11 bits, we align it so bit 10 of generator aligns with bit i
            remainder ^= (generator << (i - 10));
        }
    }
    
    // Parity bits are the lower 10 bits of remainder
    return remainder & 0x3FF;
}

uint32_t pocsag_encoder_impl::create_address_codeword(uint32_t address, int function_bits)
{
    // Address codeword format (32 bits):
    // Bit 0: 0 (indicates address codeword)
    // Bits 1-19: Address bits (19 bits, from 21-bit address)
    // Bits 20-21: Function bits (2 bits)
    // Bits 22-31: BCH parity (10 bits)
    // Bit 31: Even parity bit (over all 31 bits)
    
    // Extract frame number from address (bits 18-20 of 21-bit address)
    // Frame number = address % 8
    int frame_num = address & 0x7;
    
    // Extract address bits 0-17 (18 bits)
    uint32_t addr_bits = (address >> 3) & 0x3FFFF;
    
    // Build codeword: bit 0 = 0, bits 1-19 = address, bits 20-21 = function
    uint32_t codeword = 0;
    codeword |= (addr_bits & 0x7FFFF) << 1; // Bits 1-19
    codeword |= (function_bits & 0x3) << 20; // Bits 20-21
    
    // Compute BCH parity (10 bits)
    uint32_t data_for_bch = codeword >> 1; // 20 bits: address + function
    uint32_t parity = compute_bch_parity(data_for_bch);
    codeword |= parity << 22; // Bits 22-31
    
    // Compute even parity over all 31 bits
    int ones = 0;
    for (int i = 0; i < 31; i++) {
        if (codeword & (1U << i)) {
            ones++;
        }
    }
    if (ones % 2 == 0) {
        codeword |= 1U << 31; // Set parity bit to make even
    }
    
    return codeword;
}

uint32_t pocsag_encoder_impl::create_message_codeword(const std::vector<uint8_t>& message, int start_bit)
{
    // Message codeword format (32 bits):
    // Bit 0: 1 (indicates message codeword)
    // Bits 1-20: Message data (20 bits)
    // Bits 21-30: BCH parity (10 bits)
    // Bit 31: Even parity bit
    
    uint32_t codeword = 1; // Bit 0 = 1 for message codeword
    
    // Extract 20 bits from message starting at start_bit
    int bits_available = (message.size() * 8) - start_bit;
    int bits_to_use = std::min(20, bits_available);
    
    for (int i = 0; i < bits_to_use; i++) {
        int bit_pos = start_bit + i;
        int byte_idx = bit_pos / 8;
        int bit_in_byte = 7 - (bit_pos % 8); // MSB first
        
        if (byte_idx < static_cast<int>(message.size())) {
            if (message[byte_idx] & (1U << bit_in_byte)) {
                codeword |= 1U << (i + 1); // Bits 1-20
            }
        }
    }
    
    // Compute BCH parity
    uint32_t data_for_bch = (codeword >> 1) & 0xFFFFF; // 20 bits
    uint32_t parity = compute_bch_parity(data_for_bch);
    codeword |= parity << 21; // Bits 21-30
    
    // Compute even parity over all 31 bits
    int ones = 0;
    for (int i = 0; i < 31; i++) {
        if (codeword & (1U << i)) {
            ones++;
        }
    }
    if (ones % 2 == 0) {
        codeword |= 1U << 31; // Set parity bit to make even
    }
    
    return codeword;
}

void pocsag_encoder_impl::encode_message_to_codewords(const std::vector<uint8_t>& message)
{
    d_codeword_queue.clear();
    
    // Determine which frame this address belongs to (0-7)
    int frame_num = d_address & 0x7;
    
    // Create address codeword
    uint32_t addr_codeword = create_address_codeword(d_address, d_function_bits);
    
    // Add address codeword to the appropriate frame position
    // We need to pad with idle codewords before the address codeword
    for (int f = 0; f < frame_num; f++) {
        for (int c = 0; c < CODEWORDS_PER_FRAME; c++) {
            d_codeword_queue.push_back(IDLE_CODEWORD);
        }
    }
    
    // Add address codeword as first codeword in its frame
    d_codeword_queue.push_back(addr_codeword);
    
    // Encode message into codewords (20 bits per codeword)
    int message_bits = message.size() * 8;
    int bit_pos = 0;
    
    // First message codeword goes in second position of the frame
    if (message_bits > 0) {
        uint32_t msg_codeword = create_message_codeword(message, bit_pos);
        d_codeword_queue.push_back(msg_codeword);
        bit_pos += 20;
    }
    
    // Remaining message codewords
    while (bit_pos < message_bits) {
        uint32_t msg_codeword = create_message_codeword(message, bit_pos);
        d_codeword_queue.push_back(msg_codeword);
        bit_pos += 20;
    }
    
    // Pad remaining frames with idle codewords
    int codewords_in_batch = d_codeword_queue.size();
    int frames_used = (codewords_in_batch + CODEWORDS_PER_FRAME - 1) / CODEWORDS_PER_FRAME;
    
    // Complete current frame if needed
    while (d_codeword_queue.size() % CODEWORDS_PER_FRAME != 0) {
        d_codeword_queue.push_back(IDLE_CODEWORD);
    }
    
    // Complete remaining frames in batch
    while (frames_used < FRAMES_PER_BATCH) {
        for (int c = 0; c < CODEWORDS_PER_FRAME; c++) {
            d_codeword_queue.push_back(IDLE_CODEWORD);
        }
        frames_used++;
    }
}

void pocsag_encoder_impl::generate_preamble_bits(std::vector<uint8_t>& output, int count)
{
    for (int i = 0; i < count; i++) {
        output.push_back((d_preamble_bits_sent + i) % 2); // Alternating 1, 0, 1, 0...
    }
    d_preamble_bits_sent += count;
    
    if (d_preamble_bits_sent >= PREAMBLE_BITS) {
        d_preamble_sent = true;
        d_state = STATE_BATCHES;
    }
}

void pocsag_encoder_impl::generate_batch_bits(std::vector<uint8_t>& output, int count)
{
    while (output.size() < static_cast<size_t>(count) && (d_codeword_queue.size() > 0 || !d_message_queue.empty())) {
        // If codeword queue is empty, try to encode next message
        if (d_codeword_queue.empty() && !d_message_queue.empty()) {
            // Extract message from queue
            std::vector<uint8_t> message;
            while (!d_message_queue.empty() && d_message_queue.front() != 0) {
                message.push_back(d_message_queue.front());
                d_message_queue.pop_front();
            }
            if (!d_message_queue.empty() && d_message_queue.front() == 0) {
                d_message_queue.pop_front(); // Remove null terminator
            }
            
            if (!message.empty()) {
                encode_message_to_codewords(message);
            }
        }
        
        // If still empty, add sync codeword and start new batch
        if (d_codeword_queue.empty()) {
            // Start new batch with sync codeword
            for (int i = 0; i < BITS_PER_CODEWORD && output.size() < static_cast<size_t>(count); i++) {
                output.push_back((SYNC_CODEWORD >> (31 - i)) & 1);
            }
            d_current_batch++;
            d_current_frame = 0;
            d_current_codeword_in_frame = 0;
            continue;
        }
        
        // Output codeword bits
        uint32_t codeword = d_codeword_queue.front();
        d_codeword_queue.pop_front();
        
        for (int i = 0; i < BITS_PER_CODEWORD && output.size() < static_cast<size_t>(count); i++) {
            output.push_back((codeword >> (31 - i)) & 1);
        }
        
        d_current_codeword_in_frame++;
        if (d_current_codeword_in_frame >= CODEWORDS_PER_FRAME) {
            d_current_codeword_in_frame = 0;
            d_current_frame++;
            if (d_current_frame >= FRAMES_PER_BATCH) {
                d_current_frame = 0;
                // Next batch will start with sync codeword
            }
        }
    }
}

int pocsag_encoder_impl::work(int noutput_items,
                               gr_vector_const_void_star& input_items,
                               gr_vector_void_star& output_items)
{
    const unsigned char* in = (const unsigned char*)input_items[0];
    unsigned char* out = (unsigned char*)output_items[0];
    
    // Collect input message bytes (buffer them for processing)
    for (int i = 0; i < noutput_items; i++) {
        if (in[i] != 0) { // Non-null bytes are message data
            d_message_queue.push_back(in[i]);
        } else {
            // Null byte indicates end of message - trigger encoding
            if (!d_message_queue.empty()) {
                std::vector<uint8_t> message;
                while (!d_message_queue.empty()) {
                    message.push_back(d_message_queue.front());
                    d_message_queue.pop_front();
                }
                if (!message.empty()) {
                    encode_message_to_codewords(message);
                }
            }
        }
    }
    
    // Generate output bits
    std::vector<uint8_t> output_bits;
    output_bits.reserve(noutput_items);
    
    if (d_state == STATE_PREAMBLE) {
        int bits_needed = std::min(noutput_items, PREAMBLE_BITS - d_preamble_bits_sent);
        generate_preamble_bits(output_bits, bits_needed);
    }
    
    if (d_state == STATE_BATCHES) {
        generate_batch_bits(output_bits, noutput_items);
    }
    
    // Output bits (unpacked format: 0 or 1 per byte)
    int output_count = std::min(static_cast<int>(output_bits.size()), noutput_items);
    for (int i = 0; i < output_count; i++) {
        out[i] = output_bits[i];
    }
    
    // If we didn't produce enough output, pad with zeros or idle
    for (int i = output_count; i < noutput_items; i++) {
        // If we have queued codewords or messages, try to generate more
        if (!d_codeword_queue.empty()) {
            generate_batch_bits(output_bits, noutput_items - i);
            int additional = std::min(static_cast<int>(output_bits.size()) - output_count, noutput_items - i);
            for (int j = 0; j < additional; j++) {
                out[i + j] = output_bits[output_count + j];
            }
            i += additional - 1; // -1 because loop will increment
        } else if (!d_message_queue.empty()) {
            // Have message but not encoded yet - output zeros for now
            out[i] = 0;
        } else {
            // No data - output zeros (idle)
            out[i] = 0;
        }
    }
    
    return noutput_items;
}

} // namespace qradiolink
} // namespace gr

