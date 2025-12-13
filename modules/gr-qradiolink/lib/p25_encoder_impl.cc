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

#include <gnuradio/qradiolink/p25_encoder.h>
#include "p25_encoder_impl.h"
#include <gnuradio/io_signature.h>
#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <cstdint>

namespace gr {
namespace qradiolink {

p25_encoder::sptr p25_encoder::make(uint16_t nac,
                                    uint32_t source_id,
                                    uint32_t destination_id,
                                    uint16_t talkgroup_id)
{
    return gnuradio::get_initial_sptr(new p25_encoder_impl(
        nac, source_id, destination_id, talkgroup_id));
}

p25_encoder_impl::p25_encoder_impl(uint16_t nac,
                                   uint32_t source_id,
                                   uint32_t destination_id,
                                   uint16_t talkgroup_id)
    : p25_encoder("p25_encoder",
                  gr::io_signature::make(1, 1, sizeof(unsigned char)),
                  gr::io_signature::make(1, 1, sizeof(unsigned char)),
                  nac,
                  source_id,
                  destination_id,
                  talkgroup_id),
      d_nac(nac & 0xFFF), // Ensure 12 bits
      d_source_id(source_id),
      d_destination_id(destination_id),
      d_talkgroup_id(talkgroup_id),
      d_state(STATE_NID),
      d_frame_count(0)
{
    // Validate NAC (12 bits max)
    if (d_nac > 0xFFF) {
        throw std::invalid_argument("NAC must be 12 bits (max 0xFFF)");
    }
    
    // Build NID
    uint64_t nid = build_nid();
    d_nid.clear();
    for (int i = 0; i < 8; i++) {
        d_nid.push_back((nid >> (56 - i * 8)) & 0xFF);
    }
}

p25_encoder_impl::~p25_encoder_impl() {}

uint32_t p25_encoder_impl::bch_encode_63_16(uint16_t data)
{
    // BCH(63,16) encoding for NID
    // This is a simplified implementation
    // Full implementation requires generator polynomial or lookup table
    
    // For now, return data with parity (placeholder)
    // Real implementation would use proper BCH encoding
    return (static_cast<uint32_t>(data) << 16) | (data & 0xFFFF);
}

uint32_t p25_encoder_impl::golay_encode_24_12(uint16_t data)
{
    // Golay(24,12) encoding
    // This is a simplified implementation
    // Full implementation requires generator matrix or lookup table
    
    // For now, return data with parity (placeholder)
    return (static_cast<uint32_t>(data) << 12) | (data & 0xFFF);
}

uint64_t p25_encoder_impl::build_nid()
{
    // NID (Network Identifier): 64 bits
    // Contains: NAC (12 bits) + BCH(63,16) parity
    
    // Build NID structure
    // Bits 0-11: NAC
    // Bits 12-15: Reserved/Flags
    // Bits 16-31: BCH parity (16 bits from BCH(63,16))
    // Bits 32-63: Additional NID data
    
    uint64_t nid = 0;
    
    // Set NAC (bits 0-11)
    nid |= static_cast<uint64_t>(d_nac) & 0xFFF;
    
    // Encode with BCH(63,16) - simplified
    uint16_t nid_data = d_nac; // 12 bits, pad to 16 for encoding
    uint32_t bch_encoded = bch_encode_63_16(nid_data);
    
    // Add BCH parity to NID
    nid |= (static_cast<uint64_t>(bch_encoded) << 16);
    
    // Add frame sync pattern (48 bits) would be sent separately
    // NID is sent after frame sync
    
    return nid;
}

void p25_encoder_impl::build_ldu1()
{
    // LDU1 (Logical Data Unit 1): 720 bits
    // Contains: 9 IMBE voice frames + Link Control Word + Low Speed Data
    
    d_ldu1.clear();
    d_ldu1.reserve(LDU1_LENGTH / 8);
    
    // LDU1 structure (simplified):
    // - 9 IMBE voice frames (88 bits each = 792 bits total, but interleaved)
    // - Link Control Word (LCW)
    // - Low Speed Data (LSD)
    
    // For now, create placeholder structure
    // Real implementation would interleave IMBE frames and add LCW/LSD
    
    // Pad to 720 bits (90 bytes)
    d_ldu1.resize(90, 0);
}

void p25_encoder_impl::build_ldu2()
{
    // LDU2 (Logical Data Unit 2): 720 bits
    // Contains: 9 IMBE voice frames + Link Control Word + Low Speed Data
    
    d_ldu2.clear();
    d_ldu2.reserve(LDU2_LENGTH / 8);
    
    // Similar to LDU1 but with different LCW content
    // Pad to 720 bits (90 bytes)
    d_ldu2.resize(90, 0);
}

void p25_encoder_impl::trellis_encode(const std::vector<uint8_t>& input, std::vector<uint8_t>& output)
{
    // Trellis encoding rate 3/4
    // This is a simplified implementation
    // Full implementation requires trellis encoder state machine
    
    output.clear();
    output.reserve((input.size() * 4) / 3);
    
    // Simplified: just pass through (placeholder)
    // Real implementation would perform rate 3/4 trellis encoding
    output = input;
}

int p25_encoder_impl::work(int noutput_items,
                           gr_vector_const_void_star& input_items,
                           gr_vector_void_star& output_items)
{
    const unsigned char* in = (const unsigned char*)input_items[0];
    unsigned char* out = (unsigned char*)output_items[0];
    
    int out_idx = 0;
    int in_idx = 0;
    
    // Collect voice data (IMBE frames: 11 bytes each)
    while (in_idx < noutput_items) {
        d_voice_queue.push_back(in[in_idx++]);
    }
    
    // Output P25 frames
    // P25 frame structure: [Sync (48 bits)][NID (64 bits)][LDU1/LDU2 (720 bits)]
    const int SYNC_BYTES = 6; // 48 bits
    const int NID_BYTES = 8;  // 64 bits
    const int LDU_BYTES = 90; // 720 bits
    
    // Send frame sync and NID first
    if (d_state == STATE_NID && out_idx + SYNC_BYTES + NID_BYTES <= noutput_items) {
        // Send frame sync (48 bits = 6 bytes)
        for (int i = 0; i < SYNC_BYTES; i++) {
            out[out_idx++] = (FRAME_SYNC >> (40 - i * 8)) & 0xFF;
        }
        
        // Send NID (64 bits = 8 bytes)
        for (size_t i = 0; i < d_nid.size() && out_idx < noutput_items; i++) {
            out[out_idx++] = d_nid[i];
        }
        
        d_state = STATE_LDU1;
        build_ldu1();
    }
    
    // Send LDU1
    if (d_state == STATE_LDU1 && 
        d_voice_queue.size() >= static_cast<size_t>(IMBE_FRAME_BYTES * IMBE_FRAMES_PER_SUPERFRAME) &&
        out_idx + LDU_BYTES <= noutput_items) {
        
        // Build LDU1 from voice data
        build_ldu1();
        
        // Extract IMBE frames from queue
        for (int frame = 0; frame < IMBE_FRAMES_PER_SUPERFRAME && !d_voice_queue.empty(); frame++) {
            // Would interleave IMBE frame into LDU1 here
            // For now, just consume the bytes
            for (int i = 0; i < IMBE_FRAME_BYTES && !d_voice_queue.empty(); i++) {
                d_voice_queue.pop_front();
            }
        }
        
        // Send LDU1
        for (size_t i = 0; i < d_ldu1.size() && out_idx < noutput_items; i++) {
            out[out_idx++] = d_ldu1[i];
        }
        
        d_state = STATE_LDU2;
        build_ldu2();
    }
    
    // Send LDU2
    if (d_state == STATE_LDU2 && 
        d_voice_queue.size() >= static_cast<size_t>(IMBE_FRAME_BYTES * IMBE_FRAMES_PER_SUPERFRAME) &&
        out_idx + LDU_BYTES <= noutput_items) {
        
        // Build LDU2 from voice data
        build_ldu2();
        
        // Extract IMBE frames from queue
        for (int frame = 0; frame < IMBE_FRAMES_PER_SUPERFRAME && !d_voice_queue.empty(); frame++) {
            // Would interleave IMBE frame into LDU2 here
            for (int i = 0; i < IMBE_FRAME_BYTES && !d_voice_queue.empty(); i++) {
                d_voice_queue.pop_front();
            }
        }
        
        // Send LDU2
        for (size_t i = 0; i < d_ldu2.size() && out_idx < noutput_items; i++) {
            out[out_idx++] = d_ldu2[i];
        }
        
        d_state = STATE_NID; // Cycle back to NID for next superframe
        d_frame_count++;
    }
    
    // Pad output if needed
    while (out_idx < noutput_items) {
        out[out_idx++] = 0;
    }
    
    return noutput_items;
}

} // namespace qradiolink
} // namespace gr

