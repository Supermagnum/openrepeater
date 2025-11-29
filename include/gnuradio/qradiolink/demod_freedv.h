/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DEMOD_FREEDV_H
#define INCLUDED_QRADIOLINK_DEMOD_FREEDV_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/hier_block2.h>
#include <gnuradio/vocoder/freedv_api.h>

namespace gr {
namespace qradiolink {

/*!
 * \brief FreeDV Demodulator block
 * \ingroup qradiolink
 *
 * FreeDV digital voice demodulator with multiple output streams.
 */
class QRADIOLINK_API demod_freedv : public gr::hier_block2
{
public:
    typedef std::shared_ptr<demod_freedv> sptr;

    /*!
     * \brief Make a FreeDV demodulator block
     *
     * \param sps Samples per symbol (default: 125)
     * \param samp_rate Sample rate (default: 8000)
     * \param carrier_freq Carrier frequency (default: 1700)
     * \param filter_width Filter width (default: 2000)
     * \param low_cutoff Low cutoff frequency (default: 200)
     * \param mode FreeDV mode (default: MODE_1600)
     * \param sb Sideband (0=upper, 1=lower, default: 0)
     */
    static sptr make(int sps = 125,
                     int samp_rate = 8000,
                     int carrier_freq = 1700,
                     int filter_width = 2000,
                     int low_cutoff = 200,
                     int mode = gr::vocoder::freedv_api::MODE_1600,
                     int sb = 0);

    virtual void set_agc_attack(float value);
    virtual void set_agc_decay(float value);
    virtual void set_squelch(int value);

protected:
    demod_freedv(const std::string& name,
                 gr::io_signature::sptr input_signature,
                 gr::io_signature::sptr output_signature)
        : hier_block2(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DEMOD_FREEDV_H */

