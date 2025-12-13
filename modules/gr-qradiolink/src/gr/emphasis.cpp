/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "emphasis.h"
#include <cmath>

namespace gr {

void calculate_preemph_taps(double sample_rate, double tau,
                            std::vector<double>& btaps,
                            std::vector<double>& ataps)
{
    // Pre-emphasis is a high-pass filter
    // Transfer function: H(s) = (1 + s*tau) / 1
    // Bilinear transform: s = 2*fs * (z-1)/(z+1)
    // where fs is the sample rate
    
    // For a first-order high-pass filter with time constant tau:
    // H(z) = (1 + 2*fs*tau) / (1 + 2*fs*tau) * (1 - z^-1) / (1 - alpha*z^-1)
    // where alpha = (1 - 2*fs*tau) / (1 + 2*fs*tau)
    
    double fs = sample_rate;
    double alpha = (1.0 - 2.0 * fs * tau) / (1.0 + 2.0 * fs * tau);
    double gain = (1.0 + 2.0 * fs * tau) / (1.0 + 2.0 * fs * tau);
    
    // For IIR filter: y[n] = b0*x[n] + b1*x[n-1] - a1*y[n-1]
    // Numerator (feedforward): b0, b1
    // Denominator (feedback): a0=1, a1
    btaps.clear();
    ataps.clear();
    
    btaps.push_back(gain);        // b0
    btaps.push_back(-gain);       // b1
    ataps.push_back(1.0);         // a0 (always 1)
    ataps.push_back(-alpha);      // a1
}

void calculate_deemph_taps(double sample_rate, double tau,
                           std::vector<double>& btaps,
                           std::vector<double>& ataps)
{
    // De-emphasis is a low-pass filter (inverse of pre-emphasis)
    // Transfer function: H(s) = 1 / (1 + s*tau)
    // Bilinear transform: s = 2*fs * (z-1)/(z+1)
    
    // For a first-order low-pass filter with time constant tau:
    // H(z) = (1 + z^-1) / (1 + 2*fs*tau + (1 - 2*fs*tau)*z^-1)
    // Normalized: H(z) = (1 + z^-1) / ((1 + 2*fs*tau) + (1 - 2*fs*tau)*z^-1)
    
    double fs = sample_rate;
    double denominator = 1.0 + 2.0 * fs * tau;
    double alpha = (1.0 - 2.0 * fs * tau) / denominator;
    double gain = 1.0 / denominator;
    
    // For IIR filter: y[n] = b0*x[n] + b1*x[n-1] - a1*y[n-1]
    // Numerator (feedforward): b0, b1
    // Denominator (feedback): a0=1, a1
    btaps.clear();
    ataps.clear();
    
    btaps.push_back(gain);        // b0
    btaps.push_back(gain);        // b1
    ataps.push_back(1.0);         // a0 (always 1)
    ataps.push_back(-alpha);      // a1
}

} // namespace gr

