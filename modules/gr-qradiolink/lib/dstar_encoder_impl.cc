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

#include <gnuradio/qradiolink/dstar_encoder.h>
#include "dstar_encoder_impl.h"
#include <gnuradio/io_signature.h>
#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <cstdint>
#include <cctype>

namespace gr {
namespace qradiolink {

dstar_encoder::sptr dstar_encoder::make(const std::string& my_callsign,
                                        const std::string& your_callsign,
                                        const std::string& rpt1_callsign,
                                        const std::string& rpt2_callsign,
                                        const std::string& message_text)
{
    return gnuradio::get_initial_sptr(new dstar_encoder_impl(
        my_callsign, your_callsign, rpt1_callsign, rpt2_callsign, message_text));
}

dstar_encoder_impl::dstar_encoder_impl(const std::string& my_callsign,
                                       const std::string& your_callsign,
                                       const std::string& rpt1_callsign,
                                       const std::string& rpt2_callsign,
                                       const std::string& message_text)
    : dstar_encoder("dstar_encoder",
                    gr::io_signature::make(1, 1, sizeof(unsigned char)),
                    gr::io_signature::make(1, 1, sizeof(unsigned char)),
                    my_callsign,
                    your_callsign,
                    rpt1_callsign,
                    rpt2_callsign,
                    message_text),
      d_my_callsign(my_callsign),
      d_your_callsign(your_callsign),
      d_rpt1_callsign(rpt1_callsign),
      d_rpt2_callsign(rpt2_callsign),
      d_message_text(message_text),
      d_state(STATE_HEADER),
      d_header_sent(false),
      d_frame_count(0),
      d_slow_data_bit_pos(0)
{
    // Validate and pad callsigns to 8 characters
    auto pad_callsign = [](const std::string& cs) -> std::string {
        std::string result = cs;
        if (result.length() > 8) {
            result = result.substr(0, 8);
        } else {
            result.resize(8, ' ');
        }
        // Convert to uppercase
        for (char& c : result) {
            c = std::toupper(static_cast<unsigned char>(c));
        }
        return result;
    };
    
    d_my_callsign = pad_callsign(my_callsign);
    d_your_callsign = pad_callsign(your_callsign);
    d_rpt1_callsign = pad_callsign(rpt1_callsign);
    d_rpt2_callsign = pad_callsign(rpt2_callsign);
    
    // Limit message text to 20 characters
    if (d_message_text.length() > 20) {
        d_message_text = d_message_text.substr(0, 20);
    }
    
    // Build header
    build_header();
    
    // Prepare slow data (message text encoded as slow data)
    encode_slow_data();
}

dstar_encoder_impl::~dstar_encoder_impl() {}

void dstar_encoder_impl::build_header()
{
    d_header.clear();
    d_header.resize(HEADER_LENGTH, 0);
    
    // Flag bytes (bytes 0-2)
    d_header[0] = 0x00; // Flag 1
    d_header[1] = 0x00; // Flag 2
    d_header[2] = 0x00; // Flag 3
    
    // Callsigns (8 bytes each)
    // RPT2 (bytes 3-10)
    std::memcpy(&d_header[3], d_rpt2_callsign.c_str(), 8);
    
    // RPT1 (bytes 11-18)
    std::memcpy(&d_header[11], d_rpt1_callsign.c_str(), 8);
    
    // YOUR (bytes 19-26)
    std::memcpy(&d_header[19], d_your_callsign.c_str(), 8);
    
    // MY (bytes 27-34)
    std::memcpy(&d_header[27], d_my_callsign.c_str(), 8);
    
    // Suffix (bytes 35-38) - typically "    " or "TEST"
    std::string suffix = "    ";
    if (!d_message_text.empty()) {
        suffix = d_message_text.substr(0, 4);
        suffix.resize(4, ' ');
    }
    std::memcpy(&d_header[35], suffix.c_str(), 4);
    
    // CRC (bytes 39-40) - would be calculated here
    // For now, set to 0 (real implementation would compute CRC)
    d_header[39] = 0x00;
    d_header[40] = 0x00;
    
    // Apply Golay(24,12) FEC to header
    // D-STAR header uses Golay encoding on 12-bit chunks
    // This is simplified - full implementation would encode each 12-bit chunk
}

uint32_t dstar_encoder_impl::golay_encode_12bit(uint16_t data)
{
    // Golay(24,12) encoding
    // This is a simplified implementation
    // Full implementation requires generator matrix or lookup table
    
    // Golay(24,12) can correct up to 3 errors
    // Generator polynomial approach (simplified)
    
    // For now, return data with parity (placeholder)
    // Real implementation would use proper Golay encoding
    return (static_cast<uint32_t>(data) << 12) | (data & 0xFFF);
}

void dstar_encoder_impl::encode_slow_data()
{
    // Slow data: 24 bits per 20ms frame = 1200 bps
    // Encode message text into slow data bits
    
    d_slow_data_bits.clear();
    
    if (d_message_text.empty()) {
        // No message - send idle pattern
        for (int i = 0; i < SLOW_DATA_BITS; i++) {
            d_slow_data_bits.push_back(0);
        }
        return;
    }
    
    // Encode message as 7-bit ASCII
    for (char c : d_message_text) {
        uint8_t ascii = static_cast<uint8_t>(c);
        for (int i = 6; i >= 0; i--) {
            d_slow_data_bits.push_back((ascii >> i) & 1);
        }
    }
    
    // Pad to multiple of 24 bits
    while (d_slow_data_bits.size() % SLOW_DATA_BITS != 0) {
        d_slow_data_bits.push_back(0);
    }
}

void dstar_encoder_impl::generate_pn9_scrambler(std::vector<uint8_t>& sequence, int length)
{
    // PN9 scrambler: x^9 + x^5 + 1
    // Used for D-STAR voice scrambling
    
    sequence.clear();
    sequence.reserve(length);
    
    uint16_t state = 0x1FF; // Initial state (all ones)
    
    for (int i = 0; i < length; i++) {
        // Extract bit 8 (MSB)
        uint8_t bit = (state >> 8) & 1;
        sequence.push_back(bit);
        
        // Shift and XOR
        uint8_t feedback = ((state >> 8) ^ (state >> 4)) & 1;
        state = ((state << 1) | feedback) & 0x1FF;
    }
}

int dstar_encoder_impl::work(int noutput_items,
                             gr_vector_const_void_star& input_items,
                             gr_vector_void_star& output_items)
{
    const unsigned char* in = (const unsigned char*)input_items[0];
    unsigned char* out = (unsigned char*)output_items[0];
    
    int out_idx = 0;
    int in_idx = 0;
    
    // Send header first
    if (!d_header_sent && d_state == STATE_HEADER) {
        // Send frame sync
        if (out_idx + 3 <= noutput_items) {
            out[out_idx++] = FRAME_SYNC[0];
            out[out_idx++] = FRAME_SYNC[1];
            out[out_idx++] = FRAME_SYNC[2];
        }
        
        // Send header
        int header_bytes = std::min(HEADER_LENGTH, noutput_items - out_idx);
        for (int i = 0; i < header_bytes; i++) {
            out[out_idx++] = d_header[i];
        }
        
        if (out_idx >= HEADER_LENGTH + 3) {
            d_header_sent = true;
            d_state = STATE_VOICE_FRAMES;
        }
    }
    
    // Process voice frames
    if (d_state == STATE_VOICE_FRAMES) {
        // Collect voice data (96 bits = 12 bytes per frame)
        const int VOICE_FRAME_BYTES = VOICE_FRAME_BITS / 8;
        
        while (in_idx < noutput_items && 
               d_voice_queue.size() < static_cast<size_t>(VOICE_FRAME_BYTES) &&
               out_idx + VOICE_FRAME_BYTES + (SLOW_DATA_BITS / 8) + 3 <= noutput_items) {
            d_voice_queue.push_back(in[in_idx++]);
        }
        
        // When we have a complete voice frame, output it
        if (d_voice_queue.size() >= static_cast<size_t>(VOICE_FRAME_BYTES)) {
            // Send frame sync
            if (out_idx + 3 <= noutput_items) {
                out[out_idx++] = FRAME_SYNC[0];
                out[out_idx++] = FRAME_SYNC[1];
                out[out_idx++] = FRAME_SYNC[2];
            }
            
            // Send voice data (96 bits = 12 bytes)
            for (int i = 0; i < VOICE_FRAME_BYTES && !d_voice_queue.empty(); i++) {
                out[out_idx++] = d_voice_queue.front();
                d_voice_queue.pop_front();
            }
            
            // Send slow data (24 bits = 3 bytes)
            int slow_data_bytes = SLOW_DATA_BITS / 8;
            for (int i = 0; i < slow_data_bytes; i++) {
                uint8_t slow_byte = 0;
                for (int j = 0; j < 8; j++) {
                    int bit_pos = d_slow_data_bit_pos + (i * 8) + j;
                    if (bit_pos < static_cast<int>(d_slow_data_bits.size())) {
                        slow_byte |= (d_slow_data_bits[bit_pos] << (7 - j));
                    }
                }
                if (out_idx < noutput_items) {
                    out[out_idx++] = slow_byte;
                }
            }
            
            d_slow_data_bit_pos += SLOW_DATA_BITS;
            if (d_slow_data_bit_pos >= static_cast<int>(d_slow_data_bits.size())) {
                d_slow_data_bit_pos = 0; // Wrap around
            }
            
            d_frame_count++;
        }
    }
    
    // Pad output if needed
    while (out_idx < noutput_items) {
        out[out_idx++] = 0;
    }
    
    return noutput_items;
}

} // namespace qradiolink
} // namespace gr

