/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DEMOD_4FSK_H
#define INCLUDED_QRADIOLINK_DEMOD_4FSK_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/hier_block2.h>

namespace gr {
namespace qradiolink {

/*!
 * \brief 4FSK Demodulator with CCSDS decoding
 * \ingroup qradiolink
 *
 * This block implements a 4-level Frequency Shift Keying (4FSK) demodulator
 * with CCSDS convolutional decoding and descrambling.
 */
class QRADIOLINK_API demod_4fsk : public hier_block2
{
public:
    typedef std::shared_ptr<demod_4fsk> sptr;

    /*!
     * \brief Make a 4FSK demodulator block
     *
     * Outputs:
     *  - 0: Filtered complex signal
     *  - 1: Constellation (complex)
     *  - 2: Decoded bytes
     *
     * \param sps Samples per symbol (default: 125)
     * \param samp_rate Sample rate (default: 250000)
     * \param carrier_freq Carrier frequency in Hz (default: 1700)
     * \param filter_width Filter width in Hz (default: 8000)
     * \param fm Frequency modulation mode (default: true)
     */
    static sptr make(int sps = 125,
                     int samp_rate = 250000,
                     int carrier_freq = 1700,
                     int filter_width = 8000,
                     bool fm = true);

protected:
    demod_4fsk(const std::string& name,
               gr::io_signature::sptr input_signature,
               gr::io_signature::sptr output_signature)
        : hier_block2(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DEMOD_4FSK_H */

