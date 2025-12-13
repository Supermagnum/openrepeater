/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_P25_ENCODER_H
#define INCLUDED_QRADIOLINK_P25_ENCODER_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace qradiolink {

/*!
 * \brief P25 Phase 1 Encoder
 * \ingroup qradiolink
 *
 * Encodes voice data and metadata into P25 Phase 1 C4FM frames according to TIA-102 standards.
 *
 * P25 Phase 1 encoding includes:
 * - Frame sync: 0x5575F5FF77FF (48 bits)
 * - NID (Network Identifier): 64 bits with BCH(63,16)
 * - LDU1/LDU2 (Logical Data Unit) structure
 * - Voice superframe: 9 IMBE frames
 * - Trellis encoding (rate 3/4)
 * - Low Speed Data (LSD) for metadata
 * - Reed-Solomon RS(24,12,13) for data
 * - Golay(24,12) for control
 *
 * Input: Voice data (bytes) + metadata
 * Output: P25 C4FM symbols
 */
class QRADIOLINK_API p25_encoder : public gr::sync_block
{
public:
    typedef std::shared_ptr<p25_encoder> sptr;

    /*!
     * \brief Make a P25 Phase 1 encoder block
     *
     * \param nac Network Access Code (12 bits, default: 0x293)
     * \param source_id Source ID (24 bits, default: 0)
     * \param destination_id Destination ID (24 bits, default: 0)
     * \param talkgroup_id Talkgroup ID (16 bits, default: 0)
     */
    static sptr make(uint16_t nac = 0x293,
                     uint32_t source_id = 0,
                     uint32_t destination_id = 0,
                     uint16_t talkgroup_id = 0);

protected:
    p25_encoder(const std::string& name,
                gr::io_signature::sptr input_signature,
                gr::io_signature::sptr output_signature,
                uint16_t nac,
                uint32_t source_id,
                uint32_t destination_id,
                uint16_t talkgroup_id)
        : sync_block(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_P25_ENCODER_H */

