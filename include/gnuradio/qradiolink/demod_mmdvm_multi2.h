/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DEMOD_MMDVM_MULTI2_H
#define INCLUDED_QRADIOLINK_DEMOD_MMDVM_MULTI2_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/hier_block2.h>
#include <cstdint>

// Forward declaration - BurstTimer is application-level
class BurstTimer;

namespace gr {
namespace qradiolink {

/*!
 * \brief MMDVM Multi-Channel Demodulator block (PFB version)
 * \ingroup qradiolink
 *
 * Multi-channel MMDVM demodulator using polyphase filter bank channelizer.
 */
class QRADIOLINK_API demod_mmdvm_multi2 : public gr::hier_block2
{
public:
    typedef std::shared_ptr<demod_mmdvm_multi2> sptr;

    /*!
     * \brief Make an MMDVM multi-channel demodulator block (PFB version)
     *
     * \param burst_timer Pointer to BurstTimer instance (application-level)
     * \param num_channels Number of channels (default: 3)
     * \param channel_separation Channel separation in Hz (default: 25000)
     * \param use_tdma Use TDMA timing (default: true)
     * \param sps Samples per symbol (default: 125)
     * \param samp_rate Sample rate (default: 250000)
     * \param carrier_freq Carrier frequency (default: 1700)
     * \param filter_width Filter width (default: 5000)
     */
    static sptr make(BurstTimer* burst_timer,
                    int num_channels = 3,
                    int channel_separation = 25000,
                    bool use_tdma = true,
                    int sps = 125,
                    int samp_rate = 250000,
                    int carrier_freq = 1700,
                    int filter_width = 5000);

    virtual void set_filter_width(int filter_width);
    virtual void calibrate_rssi(float level);

    virtual ~demod_mmdvm_multi2();

protected:
    demod_mmdvm_multi2(const std::string& name,
                      gr::io_signature::sptr input_signature,
                      gr::io_signature::sptr output_signature)
        : hier_block2(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DEMOD_MMDVM_MULTI2_H */

