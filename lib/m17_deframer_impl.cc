/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <climits>
#include <cstdint>
#include <cstring>
#include <algorithm>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/qradiolink/m17_deframer.h>
#include "m17_deframer_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/tags.h>

namespace gr {
namespace qradiolink {

static const pmt::pmt_t FRAME_TYPE_TAG = pmt::string_to_symbol("frame_type");
static const pmt::pmt_t FRAME_LENGTH_TAG = pmt::string_to_symbol("frame_length");

m17_deframer::sptr m17_deframer::make(int max_frame_length)
{
    return gnuradio::get_initial_sptr(new m17_deframer_impl(max_frame_length));
}

m17_deframer_impl::m17_deframer_impl(int max_frame_length)
    : m17_deframer("m17_deframer",
                   gr::io_signature::make(1, 1, sizeof(uint8_t)),
                   gr::io_signature::make(1, 1, sizeof(uint8_t)),
                   max_frame_length),
      d_max_frame_length(max_frame_length),
      d_state(STATE_SYNC_SEARCH),
      d_frame_bytes_received(0),
      d_frame_length(0),
      d_sync_word(0)
{
    // Reserve buffer space (deque doesn't have reserve, but we'll let it grow naturally)
}

m17_deframer_impl::~m17_deframer_impl() {}

bool m17_deframer_impl::find_sync_word()
{
    // Search for sync words in the buffer
    // M17 sync words are 16 bits, so we need at least 2 bytes
    if (d_buffer.size() < 2) {
        return false;
    }
    
    // Check last 2 bytes for sync word
    uint16_t word = (static_cast<uint16_t>(d_buffer[d_buffer.size() - 2]) << 8) |
                     static_cast<uint16_t>(d_buffer[d_buffer.size() - 1]);
    
    if (word == SYNC_LSF || word == SYNC_STREAM) {
        d_sync_word = word;
        d_frame_length = 48;  // LSF/Stream frame is 48 bytes (including sync)
        d_state = STATE_FRAME_RECEIVE;
        d_frame_bytes_received = 2;  // Sync word already received
        return true;
    } else if (word == SYNC_PACKET) {
        d_sync_word = word;
        d_frame_length = d_max_frame_length;  // Packet frames are variable length
        d_state = STATE_FRAME_RECEIVE;
        d_frame_bytes_received = 2;  // Sync word already received
        return true;
    }
    
    return false;
}


int m17_deframer_impl::work(int noutput_items,
                           gr_vector_const_void_star& input_items,
                           gr_vector_void_star& output_items)
{
    const uint8_t* in = (const uint8_t*)input_items[0];
    uint8_t* out = (uint8_t*)output_items[0];
    
    int ninput_items = noutput_items;
    int out_idx = 0;
    int in_idx = 0;
    
    // For sync_block, we must consume ninput_items and produce noutput_items
    // However, if we can't produce output (no sync found), we still need to consume input
    // to allow the flowgraph to make progress
    
    if (d_state == STATE_SYNC_SEARCH) {
        // Search for sync word - process all available input
        bool sync_found = false;
        for (int i = 0; i < ninput_items; i++) {
            d_buffer.push_back(in[in_idx++]);
            
            // Keep buffer size reasonable
            if (d_buffer.size() > static_cast<size_t>(d_max_frame_length + 16)) {
                d_buffer.pop_front();
            }
            
            // Check for sync word
            if (d_buffer.size() >= 2 && find_sync_word()) {
                // Found sync, switch to frame receive state
                sync_found = true;
                break;
            }
        }
        
        // If no sync found, we've consumed all input (in_idx == ninput_items)
        // Return 0 to indicate no output produced
        // This allows the flowgraph to make progress
        if (!sync_found) {
            return 0;
        }
    }
    
    if (d_state == STATE_FRAME_RECEIVE) {
        // Process frame data
        while (in_idx < ninput_items && d_frame_bytes_received < d_frame_length && out_idx < noutput_items) {
            d_buffer.push_back(in[in_idx++]);
            d_frame_bytes_received++;
        }
        
        // Check if frame is complete (either we got all required bytes, or input ended)
        bool input_ended = (in_idx >= ninput_items);
        bool frame_complete = (d_frame_bytes_received >= d_frame_length);
        
        // Only output if frame is complete AND valid
        if (frame_complete || input_ended) {
            // Validate frame length before outputting
            bool frame_valid = false;
            
            if (d_sync_word == SYNC_LSF || d_sync_word == SYNC_STREAM) {
                // LSF/Stream frames must be exactly 48 bytes (2 byte sync + 46 byte payload)
                if (d_frame_bytes_received == 48) {
                    frame_valid = true;
                }
            } else if (d_sync_word == SYNC_PACKET) {
                // Packet frames: minimum reasonable size is 4 bytes (sync + 2 bytes payload)
                // Reject frames that are too short (just sync word or sync + 1 byte)
                // TODO: Parse packet length field if available for proper validation
                if (d_frame_bytes_received >= 4 && d_frame_bytes_received <= d_max_frame_length) {
                    frame_valid = true;
                }
            }
            
            // CRITICAL: Only output if frame is BOTH valid AND complete
            // Never output truncated or invalid frames - reject them silently
            if (!frame_valid || !frame_complete || d_frame_bytes_received != d_frame_length) {
                // Invalid or incomplete frame - reject it, don't output anything
                // Reset state and continue - ensure out_idx remains 0
                d_buffer.clear();
                d_state = STATE_SYNC_SEARCH;
                d_frame_bytes_received = 0;
                d_frame_length = 0;
                // Explicitly ensure no output for invalid frames
                out_idx = 0;
            } else {
                // Frame is valid and complete - output it
                // Output the frame (excluding sync word, starting from byte 2)
                int payload_start = 2;  // Skip sync word
                // Use actual received bytes, not expected frame length
                int payload_length = d_frame_bytes_received - payload_start;
                
                // Ensure we don't output more than we actually have
                if (payload_length < 0) {
                    payload_length = 0;
                }
                
                // Determine frame type
                pmt::pmt_t frame_type;
                if (d_sync_word == SYNC_LSF || d_sync_word == SYNC_STREAM) {
                    frame_type = pmt::string_to_symbol("LSF_STREAM");
                } else {
                    frame_type = pmt::string_to_symbol("PACKET");
                }
                
                // Output frame payload
                int frame_start_idx = out_idx;
                for (int i = 0; i < payload_length && out_idx < noutput_items; i++) {
                    if (payload_start + i < static_cast<int>(d_buffer.size())) {
                        out[out_idx++] = d_buffer[payload_start + i];
                    }
                }
                
                // Add tags for frame type and length
                if (out_idx > frame_start_idx) {
                    add_item_tag(0, nitems_written(0) + frame_start_idx,
                                FRAME_TYPE_TAG, frame_type);
                    add_item_tag(0, nitems_written(0) + frame_start_idx,
                                FRAME_LENGTH_TAG, pmt::from_long(out_idx - frame_start_idx));
                }
                
                // Reset for next frame
                d_buffer.clear();
                d_state = STATE_SYNC_SEARCH;
                d_frame_bytes_received = 0;
                d_frame_length = 0;
            }
        }
        
        // If frame not complete yet, return 0 (consumed input but no output yet)
        if (!frame_complete && !input_ended) {
            return 0;
        }
    }
    
    // Return number of output items produced
    return out_idx;
}

} // namespace qradiolink
} // namespace gr

