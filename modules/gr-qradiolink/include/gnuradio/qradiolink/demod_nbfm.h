/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DEMOD_NBFM_H
#define INCLUDED_QRADIOLINK_DEMOD_NBFM_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/hier_block2.h>

namespace gr {
namespace qradiolink {

/*!
 * \brief NBFM Demodulator
 * \ingroup qradiolink
 *
 * This block implements a Narrow Band Frequency Modulation (NBFM) demodulator.
 */
class QRADIOLINK_API demod_nbfm : public hier_block2
{
public:
    typedef std::shared_ptr<demod_nbfm> sptr;

    /*!
     * \brief Make an NBFM demodulator block
     *
     * Outputs:
     *  - 0: Filtered complex signal
     *  - 1: Demodulated audio (float)
     *
     * \param sps Samples per symbol (default: 125)
     * \param samp_rate Sample rate (default: 250000)
     * \param carrier_freq Carrier frequency in Hz (default: 1700)
     * \param filter_width Filter width in Hz (default: 8000)
     */
    static sptr make(int sps = 125,
                     int samp_rate = 250000,
                     int carrier_freq = 1700,
                     int filter_width = 8000);

    /*!
     * \brief Set squelch level
     * \param value Squelch level
     */
    virtual void set_squelch(int value);

    /*!
     * \brief Set CTCSS tone frequency
     * \param value CTCSS frequency in Hz
     */
    virtual void set_ctcss(float value);

    /*!
     * \brief Set filter width
     * \param filter_width Filter width in Hz
     */
    virtual void set_filter_width(int filter_width);

protected:
    demod_nbfm(const std::string& name,
               gr::io_signature::sptr input_signature,
               gr::io_signature::sptr output_signature)
        : hier_block2(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DEMOD_NBFM_H */

