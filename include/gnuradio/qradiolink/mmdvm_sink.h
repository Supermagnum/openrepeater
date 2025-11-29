/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_MMDVM_SINK_H
#define INCLUDED_QRADIOLINK_MMDVM_SINK_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/sync_block.h>
#include <cstdint>

// Forward declaration - BurstTimer is application-level
class BurstTimer;

namespace gr {
namespace qradiolink {

/*!
 * \brief MMDVM Sink block
 * \ingroup qradiolink
 *
 * Writes audio data to ZMQ IPC sockets for MMDVM (Multi-Mode Digital Voice Modem).
 * Handles TDMA timing and RSSI tags.
 */
class QRADIOLINK_API mmdvm_sink : public gr::sync_block
{
public:
    typedef std::shared_ptr<mmdvm_sink> sptr;

    /*!
     * \brief Make an MMDVM sink block
     *
     * \param burst_timer Pointer to BurstTimer instance (application-level)
     * \param cn Number of input channels (default: 0)
     * \param multi_channel Multi-channel mode (default: true)
     * \param use_tdma Use TDMA timing (default: true)
     */
    static sptr make(BurstTimer* burst_timer,
                     uint8_t cn = 0,
                     bool multi_channel = true,
                     bool use_tdma = true);

protected:
    mmdvm_sink(const std::string& name,
               gr::io_signature::sptr input_signature,
               gr::io_signature::sptr output_signature)
        : sync_block(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_MMDVM_SINK_H */

