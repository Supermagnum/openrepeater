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

#include <gnuradio/qradiolink/ysf_encoder.h>
#include "ysf_encoder_impl.h"
#include <gnuradio/io_signature.h>
#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <cstdint>
#include <cctype>

namespace gr {
namespace qradiolink {

ysf_encoder::sptr ysf_encoder::make(const std::string& source_callsign,
                                    const std::string& destination_callsign,
                                    uint32_t radio_id,
                                    uint32_t group_id)
{
    return gnuradio::get_initial_sptr(new ysf_encoder_impl(
        source_callsign, destination_callsign, radio_id, group_id));
}

ysf_encoder_impl::ysf_encoder_impl(const std::string& source_callsign,
                                   const std::string& destination_callsign,
                                   uint32_t radio_id,
                                   uint32_t group_id)
    : ysf_encoder("ysf_encoder",
                  gr::io_signature::make(1, 1, sizeof(unsigned char)),
                  gr::io_signature::make(1, 1, sizeof(unsigned char)),
                  source_callsign,
                  destination_callsign,
                  radio_id,
                  group_id),
      d_source_callsign(source_callsign),
      d_destination_callsign(destination_callsign),
      d_radio_id(radio_id),
      d_group_id(group_id),
      d_state(STATE_FICH),
      d_frame_count(0)
{
    // Validate and pad callsigns to 10 characters
    auto pad_callsign = [](const std::string& cs) -> std::string {
        std::string result = cs;
        if (result.length() > CALLSIGN_LENGTH) {
            result = result.substr(0, CALLSIGN_LENGTH);
        } else {
            result.resize(CALLSIGN_LENGTH, ' ');
        }
        // Convert to uppercase
        for (char& c : result) {
            c = std::toupper(static_cast<unsigned char>(c));
        }
        return result;
    };
    
    d_source_callsign = pad_callsign(source_callsign);
    d_destination_callsign = pad_callsign(destination_callsign);
    
    // Build FICH
    build_fich();
}

ysf_encoder_impl::~ysf_encoder_impl() {}

uint32_t ysf_encoder_impl::golay_encode_8bit(uint8_t data)
{
    // Golay(20,8) encoding
    // This is a simplified implementation
    // Full implementation requires generator matrix or lookup table
    
    // For now, return data with parity (placeholder)
    // Real implementation would use proper Golay encoding
    return (static_cast<uint32_t>(data) << 12) | (data & 0xFF);
}

uint32_t ysf_encoder_impl::golay_encode_12bit(uint16_t data)
{
    // Golay(23,12) encoding
    // This is a simplified implementation
    // Full implementation requires generator matrix or lookup table
    
    // For now, return data with parity (placeholder)
    return (static_cast<uint32_t>(data) << 11) | (data & 0xFFF);
}

uint16_t ysf_encoder_impl::compute_crc16_ccitt(const uint8_t* data, int length)
{
    // CRC-16-CCITT: polynomial 0x1021, initial value 0xFFFF
    uint16_t crc = 0xFFFF;
    
    for (int i = 0; i < length; i++) {
        crc ^= (static_cast<uint16_t>(data[i]) << 8);
        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    
    return crc;
}

void ysf_encoder_impl::build_fich()
{
    // FICH (Frame Information Channel Header): 5 bytes = 40 bits
    // Contains frame type, callsign info, etc.
    
    d_fich.clear();
    d_fich.resize(FICH_LENGTH, 0);
    
    // FICH structure (simplified):
    // Byte 0: Frame type and mode
    d_fich[0] = 0x01; // Voice frame mode 1 (placeholder)
    
    // Bytes 1-4: Additional FICH data
    // Would contain callsign info, radio ID, etc.
    d_fich[1] = (d_radio_id >> 16) & 0xFF;
    d_fich[2] = (d_radio_id >> 8) & 0xFF;
    d_fich[3] = d_radio_id & 0xFF;
    d_fich[4] = d_group_id & 0xFF;
    
    // Apply Golay(20,8) encoding to FICH bytes
    // This is simplified - real implementation would encode each byte
}

void ysf_encoder_impl::encode_callsign(const std::string& callsign, std::vector<uint8_t>& output)
{
    // Encode 10-character callsign
    output.clear();
    output.reserve(callsign.length());
    
    for (char c : callsign) {
        output.push_back(static_cast<uint8_t>(c));
    }
}

int ysf_encoder_impl::work(int noutput_items,
                           gr_vector_const_void_star& input_items,
                           gr_vector_void_star& output_items)
{
    const unsigned char* in = (const unsigned char*)input_items[0];
    unsigned char* out = (unsigned char*)output_items[0];
    
    int out_idx = 0;
    int in_idx = 0;
    
    // Collect voice data
    while (in_idx < noutput_items) {
        d_voice_queue.push_back(in[in_idx++]);
    }
    
    // Output frames
    // YSF frame structure: [Sync][FICH][Voice/Data]
    const int VOICE_FRAME_BYTES = 144; // Typical voice frame size
    
    while (out_idx + 2 + FICH_LENGTH + VOICE_FRAME_BYTES <= noutput_items &&
           d_voice_queue.size() >= static_cast<size_t>(VOICE_FRAME_BYTES)) {
        
        // Send frame sync (2 bytes)
        out[out_idx++] = (FRAME_SYNC >> 8) & 0xFF;
        out[out_idx++] = FRAME_SYNC & 0xFF;
        
        // Send FICH (5 bytes)
        for (int i = 0; i < FICH_LENGTH && out_idx < noutput_items; i++) {
            out[out_idx++] = d_fich[i];
        }
        
        // Send voice frame (144 bytes)
        for (int i = 0; i < VOICE_FRAME_BYTES && !d_voice_queue.empty() && out_idx < noutput_items; i++) {
            out[out_idx++] = d_voice_queue.front();
            d_voice_queue.pop_front();
        }
        
        d_frame_count++;
        
        // Every N frames, include callsign data
        if (d_frame_count % 20 == 0) {
            // Include callsign in data channel (simplified)
            std::vector<uint8_t> source_call_bytes;
            encode_callsign(d_source_callsign, source_call_bytes);
            // Would interleave callsign into data channel here
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

