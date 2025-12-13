/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_GR_EMPHASIS_H
#define INCLUDED_GR_EMPHASIS_H

#include <vector>
#include <cmath>

namespace gr {

/**
 * Calculate pre-emphasis filter taps for FM modulation
 * 
 * Pre-emphasis boosts high frequencies before transmission to improve SNR.
 * Standard FM uses a 50 microsecond time constant (75 microseconds in US).
 * 
 * @param sample_rate Sample rate in Hz
 * @param tau Time constant in seconds (typically 50e-6 for 50 microseconds)
 * @param btaps Output: denominator taps (feedforward coefficients)
 * @param ataps Output: numerator taps (feedback coefficients)
 */
void calculate_preemph_taps(double sample_rate, double tau, 
                             std::vector<double>& btaps, 
                             std::vector<double>& ataps);

/**
 * Calculate de-emphasis filter taps for FM demodulation
 * 
 * De-emphasis reduces high frequencies after demodulation to restore
 * original audio spectrum and reduce noise.
 * 
 * @param sample_rate Sample rate in Hz
 * @param tau Time constant in seconds (typically 50e-6 for 50 microseconds)
 * @param btaps Output: denominator taps (feedforward coefficients)
 * @param ataps Output: numerator taps (feedback coefficients)
 */
void calculate_deemph_taps(double sample_rate, double tau,
                           std::vector<double>& btaps,
                           std::vector<double>& ataps);

} // namespace gr

#endif /* INCLUDED_GR_EMPHASIS_H */

