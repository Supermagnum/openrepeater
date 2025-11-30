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

#include <gnuradio/qradiolink/ysf_decoder.h>
#include "ysf_decoder_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/tags.h>
#include <cstring>
#include <algorithm>
#include <cstdint>
#include <cmath>

namespace gr {
namespace qradiolink {

static const pmt::pmt_t SOURCE_CALLSIGN_TAG = pmt::string_to_symbol("source_callsign");
static const pmt::pmt_t DESTINATION_CALLSIGN_TAG = pmt::string_to_symbol("destination_callsign");
static const pmt::pmt_t RADIO_ID_TAG = pmt::string_to_symbol("radio_id");
static const pmt::pmt_t GROUP_ID_TAG = pmt::string_to_symbol("group_id");
static const pmt::pmt_t FRAME_TYPE_TAG = pmt::string_to_symbol("frame_type");

ysf_decoder::sptr ysf_decoder::make(float sync_threshold)
{
    return gnuradio::get_initial_sptr(new ysf_decoder_impl(sync_threshold));
}

ysf_decoder_impl::ysf_decoder_impl(float sync_threshold)
    : ysf_decoder("ysf_decoder",
                  gr::io_signature::make(1, 1, sizeof(unsigned char)),
                  gr::io_signature::make(1, 1, sizeof(unsigned char)),
                  sync_threshold),
      d_sync_threshold(sync_threshold),
      d_state(STATE_SYNC_SEARCH),
      d_bytes_received(0),
      d_expected_bytes(0),
      d_radio_id(0),
      d_group_id(0)
{
    // Validate sync threshold
    if (sync_threshold < 0.0f || sync_threshold > 1.0f) {
        throw std::invalid_argument("Sync threshold must be between 0.0 and 1.0");
    }
    
    d_current_fich.reserve(FICH_LENGTH);
    d_current_voice_frame.reserve(VOICE_FRAME_BYTES);
}

ysf_decoder_impl::~ysf_decoder_impl() {}

bool ysf_decoder_impl::check_frame_sync(const uint8_t* data)
{
    // Check if data matches frame sync pattern (2 bytes)
    if (data[0] == ((FRAME_SYNC >> 8) & 0xFF) && 
        data[1] == (FRAME_SYNC & 0xFF)) {
        return true;
    }
    
    // Allow some tolerance for soft-decision decoding
    int matches = 0;
    if ((data[0] ^ ((FRAME_SYNC >> 8) & 0xFF)) == 0) matches++;
    if ((data[1] ^ (FRAME_SYNC & 0xFF)) == 0) matches++;
    
    float confidence = static_cast<float>(matches) / 2.0f;
    return confidence >= d_sync_threshold;
}

uint8_t ysf_decoder_impl::golay_decode_20bit(uint32_t codeword)
{
    // Golay(20,8) decoding with error correction
    // This is a simplified implementation
    // Full implementation requires syndrome table or lookup
    
    // For now, extract lower 8 bits (simplified)
    // Real implementation would perform error correction
    return static_cast<uint8_t>(codeword & 0xFF);
}

uint16_t ysf_decoder_impl::golay_decode_23bit(uint32_t codeword)
{
    // Golay(23,12) decoding with error correction
    // This is a simplified implementation
    
    // For now, extract lower 12 bits (simplified)
    return static_cast<uint16_t>(codeword & 0xFFF);
}

bool ysf_decoder_impl::check_crc16_ccitt(const uint8_t* data, int length, uint16_t received_crc)
{
    // Compute CRC-16-CCITT and compare
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
    
    return crc == received_crc;
}

void ysf_decoder_impl::decode_fich(const std::vector<uint8_t>& fich)
{
    if (fich.size() < FICH_LENGTH) {
        return;
    }
    
    // Decode FICH to extract frame information
    // Byte 0: Frame type and mode
    uint8_t frame_type = fich[0] & 0x0F;
    
    // Bytes 1-4: Radio ID, Group ID, etc.
    d_radio_id = (static_cast<uint32_t>(fich[1]) << 16) |
                 (static_cast<uint32_t>(fich[2]) << 8) |
                 static_cast<uint32_t>(fich[3]);
    d_group_id = fich[4];
    
    // Apply Golay decoding (simplified - would decode each byte)
    // For now, just extract values
}

void ysf_decoder_impl::decode_callsign(const uint8_t* data, std::string& callsign)
{
    // Decode 10-character callsign from data
    callsign.clear();
    callsign.reserve(CALLSIGN_LENGTH);
    
    for (int i = 0; i < CALLSIGN_LENGTH; i++) {
        char c = static_cast<char>(data[i]);
        if (c >= 32 && c <= 126) { // Printable ASCII
            callsign += c;
        } else {
            callsign += ' ';
        }
    }
    
    // Trim trailing spaces
    while (!callsign.empty() && callsign.back() == ' ') {
        callsign.pop_back();
    }
}

int ysf_decoder_impl::work(int noutput_items,
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
        if (d_buffer.size() >= 2) {
            for (size_t i = 0; i <= d_buffer.size() - 2; i++) {
                if (check_frame_sync(&d_buffer[i])) {
                    // Found sync - remove bytes before sync
                    for (size_t j = 0; j < i; j++) {
                        d_buffer.pop_front();
                    }
                    
                    // Determine what follows
                    if (d_buffer.size() >= 2 + FICH_LENGTH) {
                        // Likely a FICH frame
                        d_state = STATE_FICH_RECEIVE;
                        d_current_fich.clear();
                        d_bytes_received = 0;
                        d_expected_bytes = FICH_LENGTH;
                    }
                    break;
                }
            }
        }
    }
    
    if (d_state == STATE_FICH_RECEIVE) {
        // Skip sync bytes (already processed)
        if (d_buffer.size() >= 2) {
            for (int i = 0; i < 2; i++) {
                d_buffer.pop_front();
            }
        }
        
        // Collect FICH bytes
        while (d_bytes_received < d_expected_bytes && !d_buffer.empty()) {
            d_current_fich.push_back(d_buffer.front());
            d_buffer.pop_front();
            d_bytes_received++;
        }
        
        if (d_bytes_received >= d_expected_bytes) {
            // Decode FICH
            decode_fich(d_current_fich);
            
            // Switch to voice frame receive
            d_state = STATE_VOICE_FRAME_RECEIVE;
            d_current_voice_frame.clear();
            d_bytes_received = 0;
            d_expected_bytes = VOICE_FRAME_BYTES;
        }
    }
    
    if (d_state == STATE_VOICE_FRAME_RECEIVE) {
        // Collect voice frame bytes
        while (d_bytes_received < d_expected_bytes && !d_buffer.empty()) {
            d_current_voice_frame.push_back(d_buffer.front());
            d_buffer.pop_front();
            d_bytes_received++;
        }
        
        if (d_bytes_received >= d_expected_bytes) {
            // Output voice frame
            for (size_t i = 0; i < d_current_voice_frame.size() && out_idx < noutput_items; i++) {
                out[out_idx++] = d_current_voice_frame[i];
            }
            
            // Add tags for metadata
            if (out_idx > 0) {
                add_item_tag(0, nitems_written(0),
                            FRAME_TYPE_TAG, pmt::string_to_symbol("voice"));
                add_item_tag(0, nitems_written(0),
                            RADIO_ID_TAG, pmt::from_uint64(d_radio_id));
                add_item_tag(0, nitems_written(0),
                            GROUP_ID_TAG, pmt::from_uint64(d_group_id));
                
                if (!d_source_callsign.empty()) {
                    add_item_tag(0, nitems_written(0),
                                SOURCE_CALLSIGN_TAG, pmt::string_to_symbol(d_source_callsign));
                }
                if (!d_destination_callsign.empty()) {
                    add_item_tag(0, nitems_written(0),
                                DESTINATION_CALLSIGN_TAG, pmt::string_to_symbol(d_destination_callsign));
                }
            }
            
            // Reset for next frame
            d_state = STATE_SYNC_SEARCH;
            d_bytes_received = 0;
            d_expected_bytes = 0;
            d_current_voice_frame.clear();
            d_current_fich.clear();
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

