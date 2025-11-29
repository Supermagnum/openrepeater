/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_MOD_DPMR_H
#define INCLUDED_QRADIOLINK_MOD_DPMR_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/hier_block2.h>

namespace gr {
namespace qradiolink {

/*!
 * \brief dPMR Modulator
 * \ingroup qradiolink
 *
 * This block implements a dPMR (Digital Private Mobile Radio) 4FSK modulator.
 * dPMR uses 2400 baud symbol rate with 6.25 kHz channel spacing.
 * Standard: ETSI TS 102 658
 */
class QRADIOLINK_API mod_dpmr : public hier_block2
{
public:
    typedef std::shared_ptr<mod_dpmr> sptr;

    /*!
     * \brief Make a dPMR modulator block
     *
     * \param sps Samples per symbol (default: 125)
     * \param samp_rate Sample rate (default: 1000000)
     * \param carrier_freq Carrier frequency in Hz (default: 1700)
     * \param filter_width Filter width in Hz (default: 6000)
     */
    static sptr make(int sps = 125,
                     int samp_rate = 1000000,
                     int carrier_freq = 1700,
                     int filter_width = 6000);

    /*!
     * \brief Set baseband gain
     * \param value Gain value
     */
    virtual void set_bb_gain(float value);

protected:
    mod_dpmr(const std::string& name,
            gr::io_signature::sptr input_signature,
            gr::io_signature::sptr output_signature)
        : hier_block2(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_MOD_DPMR_H */

