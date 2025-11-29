/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_M17_DEFRAMER_H
#define INCLUDED_QRADIOLINK_M17_DEFRAMER_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace qradiolink {

/*!
 * \brief M17 Deframer
 * \ingroup qradiolink
 *
 * Extracts M17 frames from a byte stream. Searches for M17 sync words
 * and extracts frame data including Link Setup Frames (LSF), Stream frames,
 * and Packet frames.
 *
 * Outputs:
 *  - 0: Frame payload bytes (variable length, tagged with frame type)
 */
class QRADIOLINK_API m17_deframer : public gr::sync_block
{
public:
    typedef std::shared_ptr<m17_deframer> sptr;

    /*!
     * \brief Make an M17 deframer block
     *
     * \param max_frame_length Maximum frame length in bytes (default: 330)
     */
    static sptr make(int max_frame_length = 330);

protected:
    m17_deframer(const std::string& name,
                 gr::io_signature::sptr input_signature,
                 gr::io_signature::sptr output_signature,
                 int max_frame_length)
        : sync_block(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_M17_DEFRAMER_H */

