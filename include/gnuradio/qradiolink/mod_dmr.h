/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_MOD_DMR_H
#define INCLUDED_QRADIOLINK_MOD_DMR_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/hier_block2.h>

namespace gr {
namespace qradiolink {

/*!
 * \brief DMR Modulator
 * \ingroup qradiolink
 *
 * This block implements a DMR (4FSK) modulator for digital mobile radio.
 */
class QRADIOLINK_API mod_dmr : public hier_block2
{
public:
    typedef std::shared_ptr<mod_dmr> sptr;

    /*!
     * \brief Make a DMR modulator block
     *
     * \param sps Samples per symbol (default: 125)
     * \param samp_rate Sample rate (default: 1000000)
     * \param carrier_freq Carrier frequency in Hz (default: 1700)
     * \param filter_width Filter width in Hz (default: 9000)
     */
    static sptr make(int sps = 125,
                     int samp_rate = 1000000,
                     int carrier_freq = 1700,
                     int filter_width = 9000);

    /*!
     * \brief Set baseband gain
     * \param value Gain value
     */
    virtual void set_bb_gain(float value);

protected:
    mod_dmr(const std::string& name,
            gr::io_signature::sptr input_signature,
            gr::io_signature::sptr output_signature)
        : hier_block2(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_MOD_DMR_H */

