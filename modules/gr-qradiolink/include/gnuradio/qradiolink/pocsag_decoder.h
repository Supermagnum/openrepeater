/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_POCSAG_DECODER_H
#define INCLUDED_QRADIOLINK_POCSAG_DECODER_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace qradiolink {

/*!
 * \brief POCSAG Decoder
 * \ingroup qradiolink
 *
 * Decodes POCSAG bitstream according to ITU-R M.584-2.
 * Supports baud rates of 512, 1200, and 2400 bps.
 *
 * Features:
 * - Sync word detection with tolerance
 * - BCH(31,21) error correction
 * - Address and function extraction
 * - Numeric/alphanumeric decode (7-bit ASCII)
 * - Message assembly from multiple codewords
 *
 * Input: Soft or hard bits at specified baud rate (unpacked: 0 or 1 per byte)
 * Output: Decoded messages with metadata (address, function, timestamp)
 */
class QRADIOLINK_API pocsag_decoder : public gr::sync_block
{
public:
    typedef std::shared_ptr<pocsag_decoder> sptr;

    /*!
     * \brief Make a POCSAG decoder block
     *
     * \param baud_rate Baud rate: 512, 1200, or 2400 (default: 1200)
     * \param sync_threshold Sync word detection threshold (0.0-1.0, default: 0.8)
     */
    static sptr make(int baud_rate = 1200, float sync_threshold = 0.8f);

protected:
    pocsag_decoder(const std::string& name,
                   gr::io_signature::sptr input_signature,
                   gr::io_signature::sptr output_signature,
                   int baud_rate,
                   float sync_threshold)
        : sync_block(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_POCSAG_DECODER_H */

