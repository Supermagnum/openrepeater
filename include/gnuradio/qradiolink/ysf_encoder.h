/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_YSF_ENCODER_H
#define INCLUDED_QRADIOLINK_YSF_ENCODER_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/sync_block.h>
#include <string>

namespace gr {
namespace qradiolink {

/*!
 * \brief YSF Encoder
 * \ingroup qradiolink
 *
 * Encodes voice data and metadata into YSF C4FM frames according to Yaesu System Fusion specification.
 *
 * YSF encoding includes:
 * - Frame sync: 0xD471
 * - Frame types: V/D mode 1, V/D mode 2, Data FR, Voice FR
 * - FEC: Golay(20,8) and Golay(23,12)
 * - FICH (Frame Information Channel Header)
 * - DCH (Data Channel) and VCH (Voice Channel)
 * - CRC-16-CCITT for data integrity
 *
 * Input: Voice data (bytes) + metadata
 * Output: YSF C4FM symbols (4-level FSK)
 */
class QRADIOLINK_API ysf_encoder : public gr::sync_block
{
public:
    typedef std::shared_ptr<ysf_encoder> sptr;

    /*!
     * \brief Make a YSF encoder block
     *
     * \param source_callsign Source callsign (10 characters, space-padded)
     * \param destination_callsign Destination callsign (10 characters)
     * \param radio_id Radio ID (optional)
     * \param group_id Group ID (optional)
     */
    static sptr make(const std::string& source_callsign = "N0CALL    ",
                     const std::string& destination_callsign = "CQCQCQ    ",
                     uint32_t radio_id = 0,
                     uint32_t group_id = 0);

protected:
    ysf_encoder(const std::string& name,
                gr::io_signature::sptr input_signature,
                gr::io_signature::sptr output_signature,
                const std::string& source_callsign,
                const std::string& destination_callsign,
                uint32_t radio_id,
                uint32_t group_id)
        : sync_block(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_YSF_ENCODER_H */

