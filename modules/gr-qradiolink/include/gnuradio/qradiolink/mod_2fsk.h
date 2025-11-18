/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_MOD_2FSK_H
#define INCLUDED_QRADIOLINK_MOD_2FSK_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/hier_block2.h>

namespace gr {
namespace qradiolink {

/*!
 * \brief 2FSK Modulator with CCSDS encoding
 * \ingroup qradiolink
 *
 * This block implements a 2-level Frequency Shift Keying (2FSK) modulator
 * with CCSDS convolutional encoding and scrambling. Designed for amateur
 * radio and digital communications.
 */
class QRADIOLINK_API mod_2fsk : public hier_block2
{
public:
    typedef std::shared_ptr<mod_2fsk> sptr;

    /*!
     * \brief Make a 2FSK modulator block
     *
     * \param sps Samples per symbol (default: 125)
     * \param samp_rate Sample rate (default: 250000)
     * \param carrier_freq Carrier frequency in Hz (default: 1700)
     * \param filter_width Filter width in Hz (default: 8000)
     * \param fm Frequency modulation mode (default: false)
     */
    static sptr make(int sps = 125,
                     int samp_rate = 250000,
                     int carrier_freq = 1700,
                     int filter_width = 8000,
                     bool fm = false);

    /*!
     * \brief Set baseband gain
     * \param value Gain value
     */
    virtual void set_bb_gain(float value);

protected:
    mod_2fsk(const std::string& name,
             gr::io_signature::sptr input_signature,
             gr::io_signature::sptr output_signature)
        : hier_block2(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_MOD_2FSK_H */

