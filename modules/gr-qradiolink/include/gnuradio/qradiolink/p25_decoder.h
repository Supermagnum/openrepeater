/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_P25_DECODER_H
#define INCLUDED_QRADIOLINK_P25_DECODER_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace qradiolink {

/*!
 * \brief P25 Phase 1 Decoder
 * \ingroup qradiolink
 *
 * Decodes P25 Phase 1 C4FM frames according to TIA-102 standards.
 *
 * Features:
 * - Frame sync detection (0x5575F5FF77FF, 48 bits)
 * - NID decode with BCH(63,16) FEC
 * - Trellis decode (Viterbi)
 * - LDU1/LDU2 processing
 * - Link Control Word extraction
 * - Talkgroup/ID extraction
 * - Encryption status detection
 *
 * Input: P25 C4FM symbols (bytes)
 * Output: Decoded voice data + trunking control info (tagged)
 */
class QRADIOLINK_API p25_decoder : public gr::sync_block
{
public:
    typedef std::shared_ptr<p25_decoder> sptr;

    /*!
     * \brief Make a P25 Phase 1 decoder block
     *
     * \param sync_threshold Sync word detection threshold (0.0-1.0, default: 0.9)
     */
    static sptr make(float sync_threshold = 0.9f);

protected:
    p25_decoder(const std::string& name,
                gr::io_signature::sptr input_signature,
                gr::io_signature::sptr output_signature,
                float sync_threshold)
        : sync_block(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_P25_DECODER_H */

