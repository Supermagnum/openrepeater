/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DEMOD_DMR_H
#define INCLUDED_QRADIOLINK_DEMOD_DMR_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/hier_block2.h>

namespace gr {
namespace qradiolink {

/*!
 * \brief DMR Demodulator block
 * \ingroup qradiolink
 *
 * Digital Mobile Radio (DMR) 4FSK demodulator with multiple output streams.
 */
class QRADIOLINK_API demod_dmr : public gr::hier_block2
{
public:
    typedef std::shared_ptr<demod_dmr> sptr;

    /*!
     * \brief Make a DMR demodulator block
     *
     * \param sps Samples per symbol (default: 125)
     * \param samp_rate Sample rate (default: 1000000)
     */
    static sptr make(int sps = 125, int samp_rate = 1000000);

protected:
    demod_dmr(const std::string& name,
              gr::io_signature::sptr input_signature,
              gr::io_signature::sptr output_signature)
        : hier_block2(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DEMOD_DMR_H */

