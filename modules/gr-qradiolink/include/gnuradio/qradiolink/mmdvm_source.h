/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_MMDVM_SOURCE_H
#define INCLUDED_QRADIOLINK_MMDVM_SOURCE_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/sync_block.h>
#include <cstdint>

// Forward declaration - BurstTimer is application-level, not a block
class BurstTimer;

namespace gr {
namespace qradiolink {

/*!
 * \brief MMDVM Source block
 * \ingroup qradiolink
 *
 * Reads audio data from ZMQ IPC sockets for MMDVM (Multi-Mode Digital Voice Modem).
 * Handles TDMA timing and adds GNU Radio tags for timing coordination.
 */
class QRADIOLINK_API mmdvm_source : public gr::sync_block
{
public:
    typedef std::shared_ptr<mmdvm_source> sptr;

    /*!
     * \brief Make an MMDVM source block
     *
     * \param burst_timer Pointer to BurstTimer instance (application-level)
     * \param cn Number of output channels (default: 0)
     * \param multi_channel Multi-channel mode (default: false)
     * \param use_tdma Use TDMA timing (default: true)
     */
    static sptr make(BurstTimer* burst_timer,
                     uint8_t cn = 0,
                     bool multi_channel = false,
                     bool use_tdma = true);

protected:
    mmdvm_source(const std::string& name,
                 gr::io_signature::sptr input_signature,
                 gr::io_signature::sptr output_signature)
        : sync_block(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_MMDVM_SOURCE_H */

