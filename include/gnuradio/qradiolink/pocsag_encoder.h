/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_POCSAG_ENCODER_H
#define INCLUDED_QRADIOLINK_POCSAG_ENCODER_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace qradiolink {

/*!
 * \brief POCSAG Encoder
 * \ingroup qradiolink
 *
 * Encodes text messages into POCSAG bitstream according to ITU-R M.584-2.
 * Supports baud rates of 512, 1200, and 2400 bps.
 *
 * POCSAG encoding includes:
 * - Preamble: 576 bits alternating 1010...
 * - Batch structure: 1 sync codeword + 8 frames
 * - Sync codeword: 0x7CD215D8
 * - BCH(31,21) error correction with even parity
 * - Idle codeword: 0x7A89C197
 *
 * Input: Text messages (bytes) with address and function bits
 * Output: POCSAG bitstream (unpacked bits, 0 or 1 per byte)
 */
class QRADIOLINK_API pocsag_encoder : public gr::sync_block
{
public:
    typedef std::shared_ptr<pocsag_encoder> sptr;

    /*!
     * \brief Make a POCSAG encoder block
     *
     * \param baud_rate Baud rate: 512, 1200, or 2400 (default: 1200)
     * \param address 21-bit address (default: 0)
     * \param function_bits 2-bit function code (default: 0)
     */
    static sptr make(int baud_rate = 1200, uint32_t address = 0, int function_bits = 0);

protected:
    pocsag_encoder(const std::string& name,
                   gr::io_signature::sptr input_signature,
                   gr::io_signature::sptr output_signature,
                   int baud_rate,
                   uint32_t address,
                   int function_bits)
        : sync_block(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_POCSAG_ENCODER_H */

