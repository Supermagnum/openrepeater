/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_DSSS_DSSS_DECODER_CC_H
#define INCLUDED_DSSS_DSSS_DECODER_CC_H

#include <gnuradio/sync_block.h>
#include <gnuradio/dsss/api.h>

namespace gr {
namespace dsss {

/*!
 * \brief DSSS Decoder - correlates received signal with spreading code
 * \ingroup dsss
 *
 * This block correlates the received complex signal with a spreading code
 * to recover the original bits. It performs matched filtering with the
 * spreading code, averaging samples over each chip period and correlating
 * with the known spreading sequence (e.g., Barker-13).
 */
class DSSS_API dsss_decoder_cc : public sync_block
{
public:
    typedef std::shared_ptr<dsss_decoder_cc> sptr;

    /*!
     * \brief Make a DSSS decoder block
     *
     * \param spreading_code Vector of spreading code chips (0s and 1s)
     * \param samples_per_symbol Number of samples per symbol (for correlation)
     */
    static sptr make(const std::vector<int>& spreading_code, int samples_per_symbol);

protected:
    dsss_decoder_cc(const std::string& name,
                    gr::io_signature::sptr input_signature,
                    gr::io_signature::sptr output_signature,
                    const std::vector<int>& spreading_code, int samples_per_symbol)
        : sync_block(name, input_signature, output_signature)
    {
        // Base class constructor - implementation in derived class
    }
};

} // namespace dsss
} // namespace gr

#endif /* INCLUDED_DSSS_DSSS_DECODER_CC_H */

