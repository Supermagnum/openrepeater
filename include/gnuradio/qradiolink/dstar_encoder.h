/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DSTAR_ENCODER_H
#define INCLUDED_QRADIOLINK_DSTAR_ENCODER_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/sync_block.h>
#include <string>

namespace gr {
namespace qradiolink {

/*!
 * \brief D-STAR Encoder
 * \ingroup qradiolink
 *
 * Encodes voice data and metadata into D-STAR DV frames according to JARL specification.
 *
 * D-STAR encoding includes:
 * - Frame sync: 0x55 0x2D 0x16
 * - Header: 41 bytes with Golay(24,12) FEC
 * - Voice frames: 96 bits voice + 24 bits slow data per 20ms frame
 * - Slow data rate: 1200 bps (for text, GPS, etc.)
 * - End pattern: 0x55 0xC8 0x7A
 *
 * Input: Voice data (bytes) + metadata
 * Output: D-STAR frame bytes
 */
class QRADIOLINK_API dstar_encoder : public gr::sync_block
{
public:
    typedef std::shared_ptr<dstar_encoder> sptr;

    /*!
     * \brief Make a D-STAR encoder block
     *
     * \param my_callsign Your callsign (8 characters, space-padded)
     * \param your_callsign Destination callsign (8 characters)
     * \param rpt1_callsign Repeater 1 callsign (8 characters)
     * \param rpt2_callsign Repeater 2 callsign (8 characters)
     * \param message_text Message text (up to 20 characters, optional)
     */
    static sptr make(const std::string& my_callsign = "N0CALL ",
                     const std::string& your_callsign = "CQCQCQ  ",
                     const std::string& rpt1_callsign = "        ",
                     const std::string& rpt2_callsign = "        ",
                     const std::string& message_text = "");

protected:
    dstar_encoder(const std::string& name,
                  gr::io_signature::sptr input_signature,
                  gr::io_signature::sptr output_signature,
                  const std::string& my_callsign,
                  const std::string& your_callsign,
                  const std::string& rpt1_callsign,
                  const std::string& rpt2_callsign,
                  const std::string& message_text)
        : sync_block(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DSTAR_ENCODER_H */

