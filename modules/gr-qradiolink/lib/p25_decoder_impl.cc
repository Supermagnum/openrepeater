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

#include <gnuradio/qradiolink/p25_decoder.h>
#include "p25_decoder_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/tags.h>
#include <cstring>
#include <algorithm>
#include <cstdint>
#include <cmath>

namespace gr {
namespace qradiolink {

static const pmt::pmt_t NAC_TAG = pmt::string_to_symbol("nac");
static const pmt::pmt_t SOURCE_ID_TAG = pmt::string_to_symbol("source_id");
static const pmt::pmt_t DESTINATION_ID_TAG = pmt::string_to_symbol("destination_id");
static const pmt::pmt_t TALKGROUP_ID_TAG = pmt::string_to_symbol("talkgroup_id");
static const pmt::pmt_t ENCRYPTED_TAG = pmt::string_to_symbol("encrypted");
static const pmt::pmt_t FRAME_TYPE_TAG = pmt::string_to_symbol("frame_type");

p25_decoder::sptr p25_decoder::make(float sync_threshold)
{
    return gnuradio::get_initial_sptr(new p25_decoder_impl(sync_threshold));
}

p25_decoder_impl::p25_decoder_impl(float sync_threshold)
    : p25_decoder("p25_decoder",
                  gr::io_signature::make(1, 1, sizeof(unsigned char)),
                  gr::io_signature::make(1, 1, sizeof(unsigned char)),
                  sync_threshold),
      d_sync_threshold(sync_threshold),
      d_state(STATE_SYNC_SEARCH),
      d_bytes_received(0),
      d_expected_bytes(0),
      d_nac(0),
      d_source_id(0),
      d_destination_id(0),
      d_talkgroup_id(0),
      d_encrypted(false)
{
    // Validate sync threshold
    if (sync_threshold < 0.0f || sync_threshold > 1.0f) {
        throw std::invalid_argument("Sync threshold must be between 0.0 and 1.0");
    }
    
    d_current_nid.reserve(NID_BYTES);
    d_current_ldu1.reserve(LDU_BYTES);
    d_current_ldu2.reserve(LDU_BYTES);
}

p25_decoder_impl::~p25_decoder_impl() {}

bool p25_decoder_impl::check_frame_sync(const uint8_t* data)
{
    // Check if data matches frame sync pattern (48 bits = 6 bytes)
    uint64_t received_sync = 0;
    for (int i = 0; i < SYNC_BYTES; i++) {
        received_sync = (received_sync << 8) | data[i];
    }
    
    // Exact match
    if (received_sync == FRAME_SYNC) {
        return true;
    }
    
    // Allow some tolerance for soft-decision decoding
    int matching_bits = 0;
    for (int i = 0; i < 48; i++) {
        if (((received_sync >> (47 - i)) & 1) == ((FRAME_SYNC >> (47 - i)) & 1)) {
            matching_bits++;
        }
    }
    
    float confidence = static_cast<float>(matching_bits) / 48.0f;
    return confidence >= d_sync_threshold;
}

uint16_t p25_decoder_impl::bch_decode_63_16(uint64_t nid)
{
    // BCH(63,16) decoding for NID
    // This is a simplified implementation
    // Full implementation requires syndrome table or lookup
    
    // Extract NAC from NID (bits 0-11)
    return static_cast<uint16_t>(nid & 0xFFF);
}

uint16_t p25_decoder_impl::golay_decode_24_12(uint32_t codeword)
{
    // Golay(24,12) decoding with error correction
    // This is a simplified implementation
    
    // For now, extract lower 12 bits (simplified)
    return static_cast<uint16_t>(codeword & 0xFFF);
}

void p25_decoder_impl::decode_nid(const std::vector<uint8_t>& nid)
{
    if (nid.size() < static_cast<size_t>(NID_BYTES)) {
        return;
    }
    
    // Reconstruct 64-bit NID
    uint64_t nid_value = 0;
    for (size_t i = 0; i < NID_BYTES; i++) {
        nid_value = (nid_value << 8) | nid[i];
    }
    
    // Decode BCH to extract NAC
    d_nac = bch_decode_63_16(nid_value);
}

void p25_decoder_impl::decode_ldu1(const std::vector<uint8_t>& ldu1)
{
    if (ldu1.size() < static_cast<size_t>(LDU_BYTES)) {
        return;
    }
    
    // LDU1 contains:
    // - 9 IMBE voice frames (interleaved)
    // - Link Control Word (LCW) with source/destination IDs
    // - Low Speed Data (LSD)
    
    // Extract Link Control Word (simplified - would need proper parsing)
    // LCW typically contains source ID, destination ID, talkgroup, etc.
    
    // For now, placeholder extraction
    // Real implementation would de-interleave and parse LCW
}

void p25_decoder_impl::decode_ldu2(const std::vector<uint8_t>& ldu2)
{
    if (ldu2.size() < static_cast<size_t>(LDU_BYTES)) {
        return;
    }
    
    // LDU2 contains:
    // - 9 IMBE voice frames (interleaved)
    // - Link Control Word (LCW) continuation
    // - Low Speed Data (LSD)
    
    // Similar to LDU1 but with different LCW content
}

void p25_decoder_impl::trellis_decode(const std::vector<uint8_t>& input, std::vector<uint8_t>& output)
{
    // Trellis decoding (Viterbi algorithm) rate 3/4
    // This is a simplified implementation
    // Full implementation requires Viterbi decoder
    
    output.clear();
    output.reserve((input.size() * 3) / 4);
    
    // Simplified: just pass through (placeholder)
    // Real implementation would perform Viterbi decoding
    output = input;
}

int p25_decoder_impl::work(int noutput_items,
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
    const size_t MAX_BUFFER_SIZE = 2000;
    if (d_buffer.size() > MAX_BUFFER_SIZE) {
        size_t to_remove = d_buffer.size() - MAX_BUFFER_SIZE;
        for (size_t i = 0; i < to_remove; i++) {
            d_buffer.pop_front();
        }
    }
    
    if (d_state == STATE_SYNC_SEARCH) {
        // Search for frame sync pattern (48 bits = 6 bytes)
        if (d_buffer.size() >= static_cast<size_t>(SYNC_BYTES)) {
            for (size_t i = 0; i <= d_buffer.size() - SYNC_BYTES; i++) {
                if (check_frame_sync(&d_buffer[i])) {
                    // Found sync - remove bytes before sync
                    for (size_t j = 0; j < i; j++) {
                        d_buffer.pop_front();
                    }
                    
                    // Switch to NID receive
                    d_state = STATE_NID_RECEIVE;
                    d_current_nid.clear();
                    d_bytes_received = 0;
                    d_expected_bytes = NID_BYTES;
                    break;
                }
            }
        }
    }
    
    if (d_state == STATE_NID_RECEIVE) {
        // Skip sync bytes (already processed)
        if (d_buffer.size() >= static_cast<size_t>(SYNC_BYTES)) {
            for (int i = 0; i < SYNC_BYTES; i++) {
                d_buffer.pop_front();
            }
        }
        
        // Collect NID bytes
        while (d_bytes_received < d_expected_bytes && !d_buffer.empty()) {
            d_current_nid.push_back(d_buffer.front());
            d_buffer.pop_front();
            d_bytes_received++;
        }
        
        if (d_bytes_received >= d_expected_bytes) {
            // Decode NID
            decode_nid(d_current_nid);
            
            // Switch to LDU1 receive
            d_state = STATE_LDU1_RECEIVE;
            d_current_ldu1.clear();
            d_bytes_received = 0;
            d_expected_bytes = LDU_BYTES;
        }
    }
    
    if (d_state == STATE_LDU1_RECEIVE) {
        // Collect LDU1 bytes
        while (d_bytes_received < d_expected_bytes && !d_buffer.empty()) {
            d_current_ldu1.push_back(d_buffer.front());
            d_buffer.pop_front();
            d_bytes_received++;
        }
        
        if (d_bytes_received >= d_expected_bytes) {
            // Decode LDU1
            decode_ldu1(d_current_ldu1);
            
            // Switch to LDU2 receive
            d_state = STATE_LDU2_RECEIVE;
            d_current_ldu2.clear();
            d_bytes_received = 0;
            d_expected_bytes = LDU_BYTES;
        }
    }
    
    if (d_state == STATE_LDU2_RECEIVE) {
        // Collect LDU2 bytes
        while (d_bytes_received < d_expected_bytes && !d_buffer.empty()) {
            d_current_ldu2.push_back(d_buffer.front());
            d_buffer.pop_front();
            d_bytes_received++;
        }
        
        if (d_bytes_received >= d_expected_bytes) {
            // Decode LDU2
            decode_ldu2(d_current_ldu2);
            
            // Extract IMBE voice frames from LDU1 and LDU2
            // Output voice data
            for (size_t i = 0; i < d_current_ldu1.size() && out_idx < noutput_items; i++) {
                out[out_idx++] = d_current_ldu1[i];
            }
            
            // Add tags for metadata
            if (out_idx > 0) {
                add_item_tag(0, nitems_written(0),
                            FRAME_TYPE_TAG, pmt::string_to_symbol("voice"));
                add_item_tag(0, nitems_written(0),
                            NAC_TAG, pmt::from_uint64(d_nac));
                add_item_tag(0, nitems_written(0),
                            SOURCE_ID_TAG, pmt::from_uint64(d_source_id));
                add_item_tag(0, nitems_written(0),
                            DESTINATION_ID_TAG, pmt::from_uint64(d_destination_id));
                add_item_tag(0, nitems_written(0),
                            TALKGROUP_ID_TAG, pmt::from_uint64(d_talkgroup_id));
                add_item_tag(0, nitems_written(0),
                            ENCRYPTED_TAG, pmt::from_bool(d_encrypted));
            }
            
            // Reset for next frame
            d_state = STATE_SYNC_SEARCH;
            d_bytes_received = 0;
            d_expected_bytes = 0;
            d_current_nid.clear();
            d_current_ldu1.clear();
            d_current_ldu2.clear();
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

