/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_MOD_FREEDV_H
#define INCLUDED_QRADIOLINK_MOD_FREEDV_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/hier_block2.h>
#include <gnuradio/vocoder/freedv_api.h>

namespace gr {
namespace qradiolink {

/*!
 * \brief FreeDV Modulator
 * \ingroup qradiolink
 *
 * This block implements a FreeDV modulator using GNU Radio's FreeDV vocoder.
 */
class QRADIOLINK_API mod_freedv : public hier_block2
{
public:
    typedef std::shared_ptr<mod_freedv> sptr;

    /*!
     * \brief Make a FreeDV modulator block
     *
     * \param sps Samples per symbol (default: 125)
     * \param samp_rate Sample rate (default: 8000)
     * \param carrier_freq Carrier frequency in Hz (default: 1700)
     * \param filter_width Filter width in Hz (default: 2000)
     * \param low_cutoff Low cutoff frequency in Hz (default: 200)
     * \param mode FreeDV mode (default: MODE_1600)
     * \param sb Sideband (0=USB, 1=LSB, default: 0)
     */
    static sptr make(int sps = 125,
                     int samp_rate = 8000,
                     int carrier_freq = 1700,
                     int filter_width = 2000,
                     int low_cutoff = 200,
                     int mode = gr::vocoder::freedv_api::MODE_1600,
                     int sb = 0);

    /*!
     * \brief Set baseband gain
     * \param value Gain value
     */
    virtual void set_bb_gain(float value);

protected:
    mod_freedv(const std::string& name,
               gr::io_signature::sptr input_signature,
               gr::io_signature::sptr output_signature)
        : hier_block2(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_MOD_FREEDV_H */

