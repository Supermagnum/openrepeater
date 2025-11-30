/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_YSF_DECODER_H
#define INCLUDED_QRADIOLINK_YSF_DECODER_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace qradiolink {

/*!
 * \brief YSF Decoder
 * \ingroup qradiolink
 *
 * Decodes YSF C4FM frames according to Yaesu System Fusion specification.
 *
 * Features:
 * - Frame sync detection (0xD471)
 * - FICH (Frame Information Channel Header) decode
 * - Golay FEC decode
 * - De-interleaving
 * - DCH/VCH separation
 * - Callsign and metadata extraction
 *
 * Input: YSF C4FM symbols (4-level FSK, bytes)
 * Output: Decoded voice data + metadata (tagged)
 */
class QRADIOLINK_API ysf_decoder : public gr::sync_block
{
public:
    typedef std::shared_ptr<ysf_decoder> sptr;

    /*!
     * \brief Make a YSF decoder block
     *
     * \param sync_threshold Sync word detection threshold (0.0-1.0, default: 0.9)
     */
    static sptr make(float sync_threshold = 0.9f);

protected:
    ysf_decoder(const std::string& name,
                gr::io_signature::sptr input_signature,
                gr::io_signature::sptr output_signature,
                float sync_threshold)
        : sync_block(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_YSF_DECODER_H */

