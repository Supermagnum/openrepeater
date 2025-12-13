/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DSTAR_DECODER_H
#define INCLUDED_QRADIOLINK_DSTAR_DECODER_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace qradiolink {

/*!
 * \brief D-STAR Decoder
 * \ingroup qradiolink
 *
 * Decodes D-STAR DV frames according to JARL specification.
 *
 * Features:
 * - Frame sync detection (0x55 0x2D 0x16)
 * - Header decode with Golay(24,12) FEC
 * - Slow data extraction and assembly
 * - Voice frame extraction
 * - Callsign and message extraction
 *
 * Input: D-STAR frame bytes
 * Output: Decoded voice data + metadata (tagged)
 */
class QRADIOLINK_API dstar_decoder : public gr::sync_block
{
public:
    typedef std::shared_ptr<dstar_decoder> sptr;

    /*!
     * \brief Make a D-STAR decoder block
     *
     * \param sync_threshold Sync word detection threshold (0.0-1.0, default: 0.9)
     */
    static sptr make(float sync_threshold = 0.9f);

protected:
    dstar_decoder(const std::string& name,
                  gr::io_signature::sptr input_signature,
                  gr::io_signature::sptr output_signature,
                  float sync_threshold)
        : sync_block(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DSTAR_DECODER_H */

