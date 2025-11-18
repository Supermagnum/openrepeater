/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DEMOD_DSSS_H
#define INCLUDED_QRADIOLINK_DEMOD_DSSS_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/hier_block2.h>

namespace gr {
namespace qradiolink {

/*!
 * \brief DSSS Demodulator with CCSDS decoding
 * \ingroup qradiolink
 *
 * This block implements a Direct Sequence Spread Spectrum (DSSS) demodulator
 * with CCSDS convolutional decoding and descrambling.
 */
class QRADIOLINK_API demod_dsss : public hier_block2
{
public:
    typedef std::shared_ptr<demod_dsss> sptr;

    /*!
     * \brief Make a DSSS demodulator block
     *
     * Outputs:
     *  - 0: Filtered complex signal
     *  - 1: Constellation (complex)
     *  - 2: Decoded bytes (primary path)
     *  - 3: Decoded bytes (delayed path)
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

protected:
    demod_dsss(const std::string& name,
               gr::io_signature::sptr input_signature,
               gr::io_signature::sptr output_signature)
        : hier_block2(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DEMOD_DSSS_H */

