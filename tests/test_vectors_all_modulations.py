#!/usr/bin/env python3
"""
Comprehensive Test Vectors for All Modulation Types

This module contains test vectors for:
- Digital Modulations: 2FSK, 4FSK, GMSK, BPSK, QPSK, DSSS
- Analog Modulations: AM, SSB (USB/LSB), NBFM
- Digital Voice: M17, DMR, FreeDV

Each modulation type includes:
- Valid test vectors
- Invalid test vectors (CRC errors, sync errors, etc.)
- Edge cases (minimum signal, frequency offset, etc.)
"""

# ============================================================================
# 2FSK Test Vectors
# ============================================================================

test_vector_2fsk_valid_1 = {
    "name": "2FSK Valid - Standard Voice Frame",
    "modulation_type": "2FSK",
    "validity": "VALID",
    "description": "Valid 2FSK frame with proper encoding",

    "frame_bits": {
        "preamble": "10101010" * 4,  # 32 bits preamble
        "sync": "0111101001011101",  # 16-bit sync word
        "payload": "1011001010110010" * 20,  # 320 bits payload
        "crc": "1100110011001100",  # 16-bit CRC
    },

    "modulation": {
        "type": "2FSK",
        "symbol_rate": 1200,  # baud
        "deviation": 2400,  # Hz (±2400 Hz)
        "bt": 0.5,  # Gaussian filter BT product
    },

    "symbol_map": {
        "0": -1.0,  # Mark frequency
        "1": +1.0,  # Space frequency
    },

    "validation": {
        "sync_detection": True,
        "crc_valid": True,
        "ber_expected": 0.0,
    }
}

test_vector_2fsk_invalid_sync = {
    "name": "2FSK Invalid - Corrupted Sync",
    "modulation_type": "2FSK",
    "validity": "INVALID",
    "error_type": "SYNC_NOT_FOUND",

    "frame_bits": {
        "preamble": "10101010" * 4,
        "sync": "0000000000000000",  # All zeros - invalid
        "payload": "1011001010110010" * 20,
        "crc": "1100110011001100",
    },

    "expected_behavior": {
        "sync_detection": False,
        "frame_accepted": False,
    }
}

# ============================================================================
# 4FSK Test Vectors
# ============================================================================

test_vector_4fsk_valid_1 = {
    "name": "4FSK Valid - Standard Data Frame",
    "modulation_type": "4FSK",
    "validity": "VALID",
    "description": "Valid 4FSK frame with 4-level encoding",

    "frame_bits": {
        "preamble": "01010101" * 4,
        "sync": "011110100101110101010111",  # 24-bit sync
        "payload": "10110010" * 40,  # 320 bits payload
        "crc": "1010101010101010",
    },

    "modulation": {
        "type": "4FSK",
        "symbol_rate": 2400,  # baud
        "deviation": 1200,  # Hz (±1200 Hz)
        "constellation": [-1.5, -0.5, 0.5, 1.5],
    },

    "symbol_map": {
        "00": -1.5,
        "01": -0.5,
        "10": 0.5,
        "11": 1.5,
    },

    "validation": {
        "sync_detection": True,
        "crc_valid": True,
        "ber_expected": 0.0,
    }
}

# ============================================================================
# GMSK Test Vectors
# ============================================================================

test_vector_gmsk_valid_1 = {
    "name": "GMSK Valid - Standard Frame",
    "modulation_type": "GMSK",
    "validity": "VALID",
    "description": "Valid GMSK frame (used in GSM, DMR)",

    "frame_bits": {
        "preamble": "10101010" * 4,
        "sync": "0111101001011101",
        "payload": "1011001010110010" * 20,
        "crc": "1100110011001100",
    },

    "modulation": {
        "type": "GMSK",
        "symbol_rate": 270833,  # baud (GSM rate)
        "bt": 0.3,  # Gaussian filter BT product
        "samples_per_symbol": 4,
    },

    "symbol_map": {
        "0": -1.0,
        "1": +1.0,
    },

    "validation": {
        "sync_detection": True,
        "crc_valid": True,
        "ber_expected": 0.0,
    }
}

test_vector_gmsk_invalid_fec = {
    "name": "GMSK Invalid - FEC Uncorrectable",
    "modulation_type": "GMSK",
    "validity": "INVALID",
    "error_type": "FEC_UNCORRECTABLE",

    "frame_bits": {
        "preamble": "10101010" * 4,
        "sync": "0111101001011101",
        # Payload with too many errors
        "payload": "11111111" * 20,  # All ones - high error rate
        "crc": "1100110011001100",
    },

    "expected_behavior": {
        "sync_detection": True,
        "fec_decode": False,
        "frame_accepted": False,
    }
}

# ============================================================================
# BPSK Test Vectors
# ============================================================================

test_vector_bpsk_valid_1 = {
    "name": "BPSK Valid - Standard Frame",
    "modulation_type": "BPSK",
    "validity": "VALID",
    "description": "Valid BPSK frame with phase modulation",

    "frame_bits": {
        "preamble": "10101010" * 4,
        "sync": "0111101001011101",
        "payload": "1011001010110010" * 20,
        "crc": "1100110011001100",
    },

    "modulation": {
        "type": "BPSK",
        "symbol_rate": 1200,  # baud
        "constellation": [-1.0, 1.0],
        "rrc_alpha": 0.35,  # Root raised cosine rolloff
    },

    "symbol_map": {
        "0": -1.0,  # 180° phase
        "1": 1.0,   # 0° phase
    },

    "validation": {
        "sync_detection": True,
        "crc_valid": True,
        "ber_expected": 0.0,
    }
}

test_vector_bpsk_invalid_phase = {
    "name": "BPSK Invalid - Phase Ambiguity",
    "modulation_type": "BPSK",
    "validity": "INVALID",
    "error_type": "PHASE_AMBIGUITY",

    "frame_bits": {
        "preamble": "10101010" * 4,
        "sync": "0111101001011101",
        # Phase inverted payload
        "payload": "0100110101001101" * 20,  # Inverted bits
        "crc": "1100110011001100",
    },

    "expected_behavior": {
        "sync_detection": True,
        "phase_lock": False,
        "frame_accepted": False,
    }
}

# ============================================================================
# QPSK Test Vectors
# ============================================================================

test_vector_qpsk_valid_1 = {
    "name": "QPSK Valid - Standard Frame",
    "modulation_type": "QPSK",
    "validity": "VALID",
    "description": "Valid QPSK frame with 4-phase modulation",

    "frame_bits": {
        "preamble": "01010101" * 4,
        "sync": "011110100101110101010111",
        "payload": "10110010" * 40,
        "crc": "1010101010101010",
    },

    "modulation": {
        "type": "QPSK",
        "symbol_rate": 2400,  # baud
        "constellation": [
            complex(-0.707, -0.707),  # 225°
            complex(0.707, -0.707),   # 315°
            complex(-0.707, 0.707),   # 135°
            complex(0.707, 0.707)      # 45°
        ],
        "rrc_alpha": 0.35,
    },

    "symbol_map": {
        "00": complex(-0.707, -0.707),
        "01": complex(0.707, -0.707),
        "10": complex(-0.707, 0.707),
        "11": complex(0.707, 0.707),
    },

    "validation": {
        "sync_detection": True,
        "crc_valid": True,
        "ber_expected": 0.0,
    }
}

# ============================================================================
# DSSS Test Vectors
# ============================================================================

test_vector_dsss_valid_1 = {
    "name": "DSSS Valid - Standard Frame",
    "modulation_type": "DSSS",
    "validity": "VALID",
    "description": "Valid DSSS frame with Barker code spreading",

    "frame_bits": {
        "preamble": "10101010" * 4,
        "sync": "0111101001011101",
        "payload": "1011001010110010" * 20,
        "crc": "1100110011001100",
    },

    "modulation": {
        "type": "DSSS",
        "symbol_rate": 1200,  # baud
        "chip_rate": 12000,  # chips per second
        "spreading_factor": 10,  # chips per symbol
        "spreading_code": [1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1],  # Barker-13
    },

    "validation": {
        "sync_detection": True,
        "despreading": True,
        "crc_valid": True,
        "ber_expected": 0.0,
    }
}

test_vector_dsss_invalid_code = {
    "name": "DSSS Invalid - Wrong Spreading Code",
    "modulation_type": "DSSS",
    "validity": "INVALID",
    "error_type": "SPREADING_CODE_MISMATCH",

    "frame_bits": {
        "preamble": "10101010" * 4,
        "sync": "0111101001011101",
        "payload": "1011001010110010" * 20,
        "crc": "1100110011001100",
    },

    "modulation": {
        "spreading_code": [0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0],  # Inverted
    },

    "expected_behavior": {
        "sync_detection": True,
        "despreading": False,
        "frame_accepted": False,
    }
}

# ============================================================================
# AM Test Vectors
# ============================================================================

test_vector_am_valid_1 = {
    "name": "AM Valid - Standard Audio Frame",
    "modulation_type": "AM",
    "validity": "VALID",
    "description": "Valid AM modulated audio signal",

    "audio_data": {
        "sample_rate": 8000,  # Hz
        "duration": 0.1,  # seconds
        "frequency": 1000,  # Hz tone
        "modulation_index": 0.5,  # 50% modulation
    },

    "modulation": {
        "type": "AM",
        "carrier_freq": 1700,  # Hz
        "bandwidth": 6000,  # Hz
    },

    "validation": {
        "carrier_detection": True,
        "audio_quality": "good",
        "snr_expected": 20,  # dB
    }
}

test_vector_am_invalid_overmod = {
    "name": "AM Invalid - Overmodulation",
    "modulation_type": "AM",
    "validity": "INVALID",
    "error_type": "OVERMODULATION",

    "audio_data": {
        "modulation_index": 1.5,  # 150% - overmodulated
    },

    "expected_behavior": {
        "carrier_detection": True,
        "distortion": True,
        "audio_quality": "poor",
    }
}

# ============================================================================
# SSB Test Vectors
# ============================================================================

test_vector_ssb_valid_usb = {
    "name": "SSB Valid - Upper Sideband",
    "modulation_type": "SSB",
    "validity": "VALID",
    "description": "Valid SSB USB modulated signal",

    "audio_data": {
        "sample_rate": 8000,
        "frequency": 1000,
        "sideband": "USB",
    },

    "modulation": {
        "type": "SSB",
        "carrier_freq": 1700,
        "bandwidth": 3000,
        "sideband": "USB",
    },

    "validation": {
        "carrier_suppression": True,
        "sideband_presence": True,
        "audio_quality": "good",
    }
}

test_vector_ssb_valid_lsb = {
    "name": "SSB Valid - Lower Sideband",
    "modulation_type": "SSB",
    "validity": "VALID",
    "description": "Valid SSB LSB modulated signal",

    "audio_data": {
        "sample_rate": 8000,
        "frequency": 1000,
        "sideband": "LSB",
    },

    "modulation": {
        "type": "SSB",
        "carrier_freq": 1700,
        "bandwidth": 3000,
        "sideband": "LSB",
    },

    "validation": {
        "carrier_suppression": True,
        "sideband_presence": True,
        "audio_quality": "good",
    }
}

# ============================================================================
# NBFM Test Vectors
# ============================================================================

test_vector_nbfm_valid_1 = {
    "name": "NBFM Valid - Standard Audio Frame",
    "modulation_type": "NBFM",
    "validity": "VALID",
    "description": "Valid narrowband FM modulated signal",

    "audio_data": {
        "sample_rate": 8000,
        "frequency": 1000,
        "deviation": 2500,  # Hz
    },

    "modulation": {
        "type": "NBFM",
        "carrier_freq": 1700,
        "bandwidth": 6000,
        "max_deviation": 2500,  # Hz
    },

    "validation": {
        "carrier_detection": True,
        "audio_quality": "good",
        "snr_expected": 20,
    }
}

test_vector_nbfm_invalid_excess_dev = {
    "name": "NBFM Invalid - Excessive Deviation",
    "modulation_type": "NBFM",
    "validity": "INVALID",
    "error_type": "EXCESSIVE_DEVIATION",

    "audio_data": {
        "deviation": 10000,  # Hz - too high for NBFM
    },

    "expected_behavior": {
        "carrier_detection": True,
        "bandwidth_violation": True,
        "audio_quality": "poor",
    }
}

# ============================================================================
# M17 Test Vectors
# ============================================================================

test_vector_m17_valid_1 = {
    "name": "M17 Valid - Voice Frame",
    "modulation_type": "M17",
    "validity": "VALID",
    "description": "Valid M17 voice frame",

    "frame_bits": {
        "sync": "1101011111010101",  # M17 sync word (0xDF55)
        "link_setup": "0000000000000000" * 8,  # LSF frame
        "voice_payload": "10110010" * 32,  # Voice data
        "crc": "1010101010101010",
    },

    "modulation": {
        "type": "4FSK",
        "symbol_rate": 4800,  # baud
        "deviation": 2400,  # Hz
    },

    "validation": {
        "sync_detection": True,
        "crc_valid": True,
        "ber_expected": 0.0,
    }
}

# ============================================================================
# DMR Test Vectors
# ============================================================================

test_vector_dmr_valid_1 = {
    "name": "DMR Valid - Voice Frame",
    "modulation_type": "DMR",
    "validity": "VALID",
    "description": "Valid DMR voice frame",

    "frame_bits": {
        "sync": "0101010101010101",  # DMR sync
        "slot_type": "0001",  # Voice slot
        "voice_payload": "10110010" * 36,  # AMBE+2 encoded
        "crc": "1010101010101010",
    },

    "modulation": {
        "type": "4FSK",
        "symbol_rate": 4800,  # baud
        "deviation": 1200,  # Hz
    },

    "validation": {
        "sync_detection": True,
        "crc_valid": True,
        "ber_expected": 0.0,
    }
}

# ============================================================================
# FreeDV Test Vectors
# ============================================================================

test_vector_freedv_valid_1 = {
    "name": "FreeDV Valid - Mode 1600",
    "modulation_type": "FreeDV",
    "validity": "VALID",
    "description": "Valid FreeDV 1600 mode frame",

    "audio_data": {
        "sample_rate": 8000,
        "mode": "MODE_1600",
        "bandwidth": 1600,  # Hz
    },

    "modulation": {
        "type": "SSB",
        "carrier_freq": 1700,
        "bandwidth": 1600,
        "mode": "MODE_1600",
    },

    "validation": {
        "carrier_detection": True,
        "vocoder_lock": True,
        "audio_quality": "good",
    }
}

# ============================================================================
# Edge Case Test Vectors (apply to all modulations)
# ============================================================================

test_vector_edge_minimum_signal = {
    "name": "Edge Case - Minimum Detectable Signal",
    "description": "Frame at sensitivity threshold",
    "applies_to": "all",
    "validity": "VALID",

    "signal_conditions": {
        "signal_level": -112,  # dBm
        "snr": 12,  # dB
        "expected_ber": 0.05,  # 5% BER
    },

    "expected_behavior": {
        "sync_detection": True,
        "sync_correlation": 0.85,
        "frame_accepted": True,
        "audio_quality": "degraded",
    }
}

test_vector_edge_freq_offset = {
    "name": "Edge Case - Maximum Frequency Offset",
    "modulation_type": "2FSK",
    "description": "Frame with 1 ppm frequency error",
    "applies_to": "all",
    "validity": "VALID",

    "signal_conditions": {
        "carrier_frequency": 450000000,  # Hz
        "frequency_error": 450,  # Hz (1 ppm)
        "symbol_rate_error": 2.4,  # Hz
    },

    "frame_bits": {
        "preamble": "10101010" * 4,
        "sync": "0111101001011101",
        "payload": "1011001010110010" * 20,
        "crc": "1100110011001100",
    },

    "modulation": {
        "type": "2FSK",
        "symbol_rate": 1200,
        "deviation": 2400,
    },

    "symbol_map": {
        "0": -1.0,
        "1": 1.0,
    },

    "validation": {
        "sync_detection": True,
        "crc_valid": True,
        "ber_expected": 0.0,
    },

    "expected_behavior": {
        "afc_required": True,
        "afc_range": 500,  # Hz
        "sync_detection": True,
        "frame_accepted": True,
    }
}

test_vector_edge_continuous_frames = {
    "name": "Edge Case - Continuous Frame Stream",
    "modulation_type": "2FSK",  # Use 2FSK as default for testing
    "description": "Multiple frames with no gaps",
    "applies_to": "all",
    "validity": "VALID",

    "frame_bits": {
        "preamble": "10101010" * 4,
        "sync": "0111101001011101",
        "payload": "1011001010110010" * 20,
        "crc": "1100110011001100",
    },

    "modulation": {
        "type": "2FSK",
        "symbol_rate": 1200,
        "deviation": 2400,
    },

    "symbol_map": {
        "0": -1.0,
        "1": 1.0,
    },

    "validation": {
        "sync_detection": True,
        "crc_valid": True,
        "ber_expected": 0.0,
    },

    "expected_behavior": {
        "all_syncs_detected": True,
        "frame_boundary_detection": True,
        "no_frame_loss": True,
        "continuous_audio": True,
    }
}

# ============================================================================
# Test Vector Collections
# ============================================================================

ALL_MODULATION_TEST_VECTORS = {
    "2FSK": {
        "valid": [test_vector_2fsk_valid_1],
        "invalid": [test_vector_2fsk_invalid_sync],
    },
    "4FSK": {
        "valid": [test_vector_4fsk_valid_1],
        "invalid": [],
    },
    "GMSK": {
        "valid": [test_vector_gmsk_valid_1],
        "invalid": [test_vector_gmsk_invalid_fec],
    },
    "BPSK": {
        "valid": [test_vector_bpsk_valid_1],
        "invalid": [test_vector_bpsk_invalid_phase],
    },
    "QPSK": {
        "valid": [test_vector_qpsk_valid_1],
        "invalid": [],
    },
    "DSSS": {
        "valid": [test_vector_dsss_valid_1],
        "invalid": [test_vector_dsss_invalid_code],
    },
    "AM": {
        "valid": [test_vector_am_valid_1],
        "invalid": [test_vector_am_invalid_overmod],
    },
    "SSB": {
        "valid": [test_vector_ssb_valid_usb, test_vector_ssb_valid_lsb],
        "invalid": [],
    },
    "NBFM": {
        "valid": [test_vector_nbfm_valid_1],
        "invalid": [test_vector_nbfm_invalid_excess_dev],
    },
    "M17": {
        "valid": [test_vector_m17_valid_1],
        "invalid": [],
    },
    "DMR": {
        "valid": [test_vector_dmr_valid_1],
        "invalid": [],
    },
    "FreeDV": {
        "valid": [test_vector_freedv_valid_1],
        "invalid": [],
    },
    "edge_cases": [
        test_vector_edge_minimum_signal,
        test_vector_edge_freq_offset,
        test_vector_edge_continuous_frames,
    ],
}

def get_test_vectors_by_modulation(modulation_type=None, validity=None):
    """
    Get test vectors filtered by modulation type and validity.

    Args:
        modulation_type: Modulation type string or None for all
        validity: 'valid', 'invalid', or None for all

    Returns:
        List of test vectors
    """
    if modulation_type is None:
        vectors = []
        for mod_type in ALL_MODULATION_TEST_VECTORS:
            if mod_type != "edge_cases":
                if validity is None:
                    vectors.extend(ALL_MODULATION_TEST_VECTORS[mod_type]["valid"])
                    vectors.extend(ALL_MODULATION_TEST_VECTORS[mod_type]["invalid"])
                elif validity == "valid":
                    vectors.extend(ALL_MODULATION_TEST_VECTORS[mod_type]["valid"])
                elif validity == "invalid":
                    vectors.extend(ALL_MODULATION_TEST_VECTORS[mod_type]["invalid"])
        if validity is None or validity == "valid":
            vectors.extend(ALL_MODULATION_TEST_VECTORS["edge_cases"])
        return vectors

    if modulation_type not in ALL_MODULATION_TEST_VECTORS:
        return []

    if validity is None:
        return (ALL_MODULATION_TEST_VECTORS[modulation_type]["valid"] +
                ALL_MODULATION_TEST_VECTORS[modulation_type]["invalid"])
    elif validity == "valid":
        return ALL_MODULATION_TEST_VECTORS[modulation_type]["valid"]
    elif validity == "invalid":
        return ALL_MODULATION_TEST_VECTORS[modulation_type]["invalid"]

    return []

def get_all_modulation_types():
    """Get list of all supported modulation types."""
    return [k for k in ALL_MODULATION_TEST_VECTORS.keys() if k != "edge_cases"]

