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

#include <gnuradio/qradiolink/dstar_decoder.h>
#include "dstar_decoder_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/tags.h>
#include <cstring>
#include <algorithm>
#include <cstdint>
#include <cmath>

namespace gr {
namespace qradiolink {

static const pmt::pmt_t MY_CALLSIGN_TAG = pmt::string_to_symbol("my_callsign");
static const pmt::pmt_t YOUR_CALLSIGN_TAG = pmt::string_to_symbol("your_callsign");
static const pmt::pmt_t RPT1_CALLSIGN_TAG = pmt::string_to_symbol("rpt1_callsign");
static const pmt::pmt_t RPT2_CALLSIGN_TAG = pmt::string_to_symbol("rpt2_callsign");
static const pmt::pmt_t MESSAGE_TAG = pmt::string_to_symbol("message");
static const pmt::pmt_t FRAME_TYPE_TAG = pmt::string_to_symbol("frame_type");

dstar_decoder::sptr dstar_decoder::make(float sync_threshold)
{
    return gnuradio::get_initial_sptr(new dstar_decoder_impl(sync_threshold));
}

dstar_decoder_impl::dstar_decoder_impl(float sync_threshold)
    : dstar_decoder("dstar_decoder",
                    gr::io_signature::make(1, 1, sizeof(unsigned char)),
                    gr::io_signature::make(1, 1, sizeof(unsigned char)),
                    sync_threshold),
      d_sync_threshold(sync_threshold),
      d_state(STATE_SYNC_SEARCH),
      d_bytes_received(0),
      d_expected_bytes(0),
      d_slow_data_bit_pos(0)
{
    // Validate sync threshold
    if (sync_threshold < 0.0f || sync_threshold > 1.0f) {
        throw std::invalid_argument("Sync threshold must be between 0.0 and 1.0");
    }
    
    d_current_header.reserve(HEADER_LENGTH);
    d_current_voice_frame.reserve(VOICE_FRAME_BYTES);
}

dstar_decoder_impl::~dstar_decoder_impl() {}

bool dstar_decoder_impl::check_frame_sync(const uint8_t* data)
{
    // Check if data matches frame sync pattern
    int matches = 0;
    for (int i = 0; i < 3; i++) {
        if (data[i] == FRAME_SYNC[i]) {
            matches++;
        }
    }
    
    float confidence = static_cast<float>(matches) / 3.0f;
    return confidence >= d_sync_threshold;
}

uint16_t dstar_decoder_impl::golay_decode_24bit(uint32_t codeword)
{
    // Golay(24,12) decoding with error correction
    // This is a simplified implementation
    // Full implementation requires syndrome table or lookup
    
    // For now, extract lower 12 bits (simplified)
    // Real implementation would perform error correction
    return static_cast<uint16_t>(codeword & 0xFFF);
}

void dstar_decoder_impl::decode_header(const std::vector<uint8_t>& header)
{
    if (header.size() < HEADER_LENGTH) {
        return;
    }
    
    // Extract callsigns (8 bytes each, space-padded)
    std::string rpt2(reinterpret_cast<const char*>(&header[3]), 8);
    std::string rpt1(reinterpret_cast<const char*>(&header[11]), 8);
    std::string your(reinterpret_cast<const char*>(&header[19]), 8);
    std::string my(reinterpret_cast<const char*>(&header[27]), 8);
    
    // Trim trailing spaces
    rpt2.erase(rpt2.find_last_not_of(' ') + 1);
    rpt1.erase(rpt1.find_last_not_of(' ') + 1);
    your.erase(your.find_last_not_of(' ') + 1);
    my.erase(my.find_last_not_of(' ') + 1);
    
    // Extract suffix (bytes 35-38)
    std::string suffix(reinterpret_cast<const char*>(&header[35]), 4);
    
    // Store decoded information (would be output via tags in real implementation)
    // For now, just store for potential use
}

void dstar_decoder_impl::decode_slow_data(const std::vector<uint8_t>& slow_data)
{
    // Extract bits from slow data bytes
    for (uint8_t byte : slow_data) {
        for (int i = 7; i >= 0; i--) {
            d_slow_data_bits.push_back((byte >> i) & 1);
        }
    }
    
    // Try to assemble message from slow data bits
    assemble_message();
}

void dstar_decoder_impl::assemble_message()
{
    // Assemble 7-bit ASCII characters from slow data bits
    d_decoded_message.clear();
    
    for (size_t i = 0; i + 6 < d_slow_data_bits.size(); i += 7) {
        uint8_t ch = 0;
        for (int j = 0; j < 7; j++) {
            if (i + j < d_slow_data_bits.size()) {
                ch |= (d_slow_data_bits[i + j] << (6 - j));
            }
        }
        
        // Stop at null character or non-printable
        if (ch == 0 || ch < 32 || ch > 126) {
            break;
        }
        
        d_decoded_message += static_cast<char>(ch);
    }
}

int dstar_decoder_impl::work(int noutput_items,
                             gr_vector_const_void_star& input_items,
                             gr_vector_void_star& output_items)
{
    const unsigned char* in = (const unsigned char*)input_items[0];
    unsigned char* out = (unsigned char*)output_items[0];
    
    int out_idx = 0;
    
    // Collect input bytes
    for (int i = 0; i < noutput_items; i++) {
        d_buffer.push_back(in[i]);
    }
    
    // Keep buffer size reasonable
    const size_t MAX_BUFFER_SIZE = 1000;
    if (d_buffer.size() > MAX_BUFFER_SIZE) {
        size_t to_remove = d_buffer.size() - MAX_BUFFER_SIZE;
        for (size_t i = 0; i < to_remove; i++) {
            d_buffer.pop_front();
        }
    }
    
    if (d_state == STATE_SYNC_SEARCH) {
        // Search for frame sync pattern
        if (d_buffer.size() >= 3) {
            for (size_t i = 0; i <= d_buffer.size() - 3; i++) {
                if (check_frame_sync(&d_buffer[i])) {
                    // Found sync - remove bytes before sync
                    for (size_t j = 0; j < i; j++) {
                        d_buffer.pop_front();
                    }
                    
                    // Determine frame type based on what follows
                    if (d_buffer.size() >= 3 + HEADER_LENGTH) {
                        // Likely a header frame
                        d_state = STATE_HEADER_RECEIVE;
                        d_current_header.clear();
                        d_bytes_received = 0;
                        d_expected_bytes = HEADER_LENGTH;
                    } else if (d_buffer.size() >= 3 + VOICE_FRAME_BYTES + SLOW_DATA_BYTES) {
                        // Likely a voice frame
                        d_state = STATE_VOICE_FRAME_RECEIVE;
                        d_current_voice_frame.clear();
                        d_bytes_received = 0;
                        d_expected_bytes = VOICE_FRAME_BYTES;
                    }
                    break;
                }
            }
        }
    }
    
    if (d_state == STATE_HEADER_RECEIVE) {
        // Skip sync bytes (already processed)
        if (d_buffer.size() >= 3) {
            for (int i = 0; i < 3; i++) {
                d_buffer.pop_front();
            }
        }
        
        // Collect header bytes
        while (d_bytes_received < d_expected_bytes && !d_buffer.empty()) {
            d_current_header.push_back(d_buffer.front());
            d_buffer.pop_front();
            d_bytes_received++;
        }
        
        if (d_bytes_received >= d_expected_bytes) {
            // Decode header
            decode_header(d_current_header);
            
            // Output header (for now, just pass through)
            for (size_t i = 0; i < d_current_header.size() && out_idx < noutput_items; i++) {
                out[out_idx++] = d_current_header[i];
            }
            
            // Add tags for metadata
            if (out_idx > 0) {
                add_item_tag(0, nitems_written(0),
                            FRAME_TYPE_TAG, pmt::string_to_symbol("header"));
            }
            
            // Reset for next frame
            d_state = STATE_SYNC_SEARCH;
            d_bytes_received = 0;
            d_expected_bytes = 0;
        }
    }
    
    if (d_state == STATE_VOICE_FRAME_RECEIVE) {
        // Skip sync bytes
        if (d_buffer.size() >= 3) {
            for (int i = 0; i < 3; i++) {
                d_buffer.pop_front();
            }
        }
        
        // Collect voice frame bytes
        while (d_bytes_received < d_expected_bytes && !d_buffer.empty()) {
            d_current_voice_frame.push_back(d_buffer.front());
            d_buffer.pop_front();
            d_bytes_received++;
        }
        
        if (d_bytes_received >= d_expected_bytes) {
            // Collect slow data bytes
            std::vector<uint8_t> slow_data;
            for (int i = 0; i < SLOW_DATA_BYTES && !d_buffer.empty(); i++) {
                slow_data.push_back(d_buffer.front());
                d_buffer.pop_front();
            }
            
            // Decode slow data
            if (!slow_data.empty()) {
                decode_slow_data(slow_data);
            }
            
            // Output voice frame
            for (size_t i = 0; i < d_current_voice_frame.size() && out_idx < noutput_items; i++) {
                out[out_idx++] = d_current_voice_frame[i];
            }
            
            // Add tags
            if (out_idx > 0) {
                add_item_tag(0, nitems_written(0),
                            FRAME_TYPE_TAG, pmt::string_to_symbol("voice"));
                
                if (!d_decoded_message.empty()) {
                    add_item_tag(0, nitems_written(0),
                                MESSAGE_TAG, pmt::string_to_symbol(d_decoded_message));
                }
            }
            
            // Reset for next frame
            d_state = STATE_SYNC_SEARCH;
            d_bytes_received = 0;
            d_expected_bytes = 0;
            d_current_voice_frame.clear();
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

