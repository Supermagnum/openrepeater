/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_MOD_DSSS_H
#define INCLUDED_QRADIOLINK_MOD_DSSS_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/hier_block2.h>

namespace gr {
namespace qradiolink {

/*!
 * \brief DSSS (Direct Sequence Spread Spectrum) Modulator
 * \ingroup qradiolink
 *
 * This block implements a DSSS modulator with CCSDS encoding and
 * Barker code spreading.
 */
class QRADIOLINK_API mod_dsss : public hier_block2
{
public:
    typedef std::shared_ptr<mod_dsss> sptr;

    /*!
     * \brief Make a DSSS modulator block
     *
     * \param sps Samples per symbol (default: 25)
     * \param samp_rate Sample rate (default: 250000)
     * \param carrier_freq Carrier frequency in Hz (default: 1700)
     * \param filter_width Filter width in Hz (default: 8000)
     */
    static sptr make(int sps = 25,
                     int samp_rate = 250000,
                     int carrier_freq = 1700,
                     int filter_width = 8000);

    /*!
     * \brief Set baseband gain
     * \param value Gain value
     */
    virtual void set_bb_gain(float value);

protected:
    mod_dsss(const std::string& name,
             gr::io_signature::sptr input_signature,
             gr::io_signature::sptr output_signature)
        : hier_block2(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_MOD_DSSS_H */

