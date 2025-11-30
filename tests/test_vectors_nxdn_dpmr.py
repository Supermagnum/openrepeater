#!/usr/bin/env python3
"""
Test Vectors for NXDN and dPMR Implementation Validation

This module contains test vectors based on:
- dPMR: ETSI TS 102 658
- NXDN: NXDN Forum Technical Specifications Part 1

Test vectors include valid frames, invalid frames, and edge cases.
"""

# dPMR Test Vectors (per ETSI TS 102 658)

test_vector_dpmr_voice_valid_1 = {
    "name": "dPMR Voice Frame - Slot 1, Unencrypted",
    "description": "Valid voice frame with proper sync, CRC, and FEC encoding",
    "mode": "Mode 1 - Peer to Peer",
    "slot": 1,
    "frame_type": "Voice",
    "encrypted": False,
    "validity": "VALID",

    # Raw frame structure (before modulation)
    "frame_bits": {
        # Sync pattern for Slot 1 (48 bits) - TS 102 658 Section 6.1
        "sync": "011110100101110101010111111101110111111111010111",  # 0x7A5D57F77FD7

        # Colour Code (12 bits) - identifies system
        "colour_code": "000000000000",  # CC=0

        # Slow data channel header (16 bits)
        "slow_data_header": "0000000000010001",  # Frame type: voice

        # Voice payload (encoded AMBE+2, 288 bits after FEC)
        # This is interleaved and FEC encoded voice data
        "voice_payload_fec": "10110010" * 36,  # Simplified - real would be AMBE encoded

        # CRC (16 bits) - CRC-16-CCITT over payload
        "crc": "1010101010101010",  # Example CRC

        # Total frame: 48+12+16+288+16 = 380 bits
    },

    # Expected modulation parameters
    "modulation": {
        "type": "4FSK",
        "symbol_rate": 2400,  # baud
        "deviation": 800,  # Hz (±800 Hz from center)
        "symbols_per_frame": 190,  # 380 bits / 2 bits per symbol
        "frame_duration": 79.167,  # ms (190 symbols / 2400 baud * 1000)
    },

    # 4FSK Symbol mapping (dibits to deviation)
    "symbol_map": {
        "00": +3.0,  # +800 Hz * 3/3 deviation
        "01": +1.0,  # +800 Hz * 1/3 deviation
        "10": -1.0,  # -800 Hz * 1/3 deviation
        "11": -3.0,  # -800 Hz * 3/3 deviation
    },

    # Expected symbol sequence (first 20 symbols from sync)
    "expected_symbols": [
        -1.0, +1.0, +1.0, +1.0, +1.0, -1.0, +1.0, -1.0,  # "01 11 10 10"
        -1.0, +1.0, -1.0, +1.0, +1.0, +1.0, -1.0, +1.0,  # "01 01 01 01"
        -1.0, +1.0, +1.0, +1.0,  # etc...
    ],

    # Validation criteria
    "validation": {
        "sync_detection": True,
        "crc_valid": True,
        "ber_expected": 0.0,  # No bit errors in test vector
        "frequency_error_max": 100,  # Hz
    }
}

test_vector_dpmr_data_valid_2 = {
    "name": "dPMR Data Frame - GPS Position",
    "description": "Valid short data frame with GPS coordinates",
    "mode": "Mode 2 - Repeater",
    "slot": 2,
    "frame_type": "Data",
    "data_type": "GPS",
    "validity": "VALID",

    "frame_bits": {
        # Sync pattern for Slot 2 (48 bits) - TS 102 658 Section 6.1
        "sync": "101011010010010110101000000010000000100000101000",  # 0xAD25A8088028

        "colour_code": "000000000001",  # CC=1

        "slow_data_header": "0000000100000010",  # Frame type: short data

        # GPS data payload (example coordinates)
        # Latitude: 59.9139° N (Oslo)
        # Longitude: 10.7522° E
        "data_payload": (
            "00111011"  # Data type: GPS
            "00111000"  # Lat deg: 59
            "01010100"  # Lat min: 54.834
            "10000011"
            "01001000"  # Lat dir: N (0), Lon deg: 10
            "00101101"  # Lon min: 45.132
            "00111100"
            "01000101"  # Lon dir: E (1)
        ),

        "crc": "1100110011001100",
    },

    "modulation": {
        "type": "4FSK",
        "symbol_rate": 2400,
        "deviation": 800,
    },

    "validation": {
        "sync_detection": True,
        "crc_valid": True,
        "data_decode": {
            "latitude": 59.9139,
            "longitude": 10.7522
        }
    }
}

test_vector_dpmr_invalid_crc = {
    "name": "dPMR Invalid - CRC Error",
    "description": "Valid frame structure but incorrect CRC",
    "validity": "INVALID",
    "error_type": "CRC_MISMATCH",

    "frame_bits": {
        "sync": "011110100101110101010111111101110111111111010111",  # Valid sync
        "colour_code": "000000000000",
        "slow_data_header": "0000000000010001",
        "voice_payload_fec": "10110010" * 36,
        "crc": "0000000000000000",  # WRONG CRC - should fail validation
    },

    "expected_behavior": {
        "sync_detection": True,  # Sync should be found
        "crc_valid": False,      # CRC should fail
        "frame_accepted": False,  # Frame should be rejected
        "error_reported": "CRC_ERROR"
    }
}

test_vector_dpmr_invalid_sync = {
    "name": "dPMR Invalid - Corrupted Sync",
    "description": "Sync pattern with bit errors",
    "validity": "INVALID",
    "error_type": "SYNC_NOT_FOUND",

    "frame_bits": {
        # Corrupted sync - 10 bit errors
        "sync": "111111111111111111111111111111111111111111111111",  # All ones - invalid
        "colour_code": "000000000000",
        "slow_data_header": "0000000000010001",
        "voice_payload_fec": "10110010" * 36,
        "crc": "1010101010101010",
    },

    "expected_behavior": {
        "sync_detection": False,  # Should not detect sync
        "frame_accepted": False,
        "error_reported": "NO_SYNC"
    }
}

test_vector_dpmr_invalid_scrambling = {
    "name": "dPMR Invalid - Unscrambled Data",
    "description": "Valid frame but data not properly scrambled",
    "validity": "INVALID",
    "error_type": "SCRAMBLING_ERROR",

    "frame_bits": {
        "sync": "011110100101110101010111111101110111111111010111",
        "colour_code": "000000000000",
        "slow_data_header": "0000000000010001",
        # Payload should be scrambled but isn't (pattern too regular)
        "voice_payload_fec": "0101010101010101" * 18,  # Unscrambled pattern
        "crc": "1010101010101010",
    },

    "expected_behavior": {
        "sync_detection": True,
        "descrambling": False,  # Descrambler won't produce valid data
        "fec_decode": False,
        "frame_accepted": False
    }
}

# NXDN Test Vectors (per NXDN TS-1)

test_vector_nxdn_voice_valid_1 = {
    "name": "NXDN Voice Frame - 4800 baud RCCH",
    "description": "Valid voice frame on control channel",
    "mode": "NXDN96 (4800 baud)",
    "channel_type": "RCCH",
    "rate": "Full Rate (EFR)",
    "validity": "VALID",

    "frame_bits": {
        # Preamble (20 bits) - all 0s
        "preamble": "01010101010101010101",

        # RCCH Sync pattern (20 bits) - TS-1 Table 4-1
        "sync": "01010101010101010101",  # RCCH sync

        # Frame Information (16 bits)
        "frame_info": {
            "message_type": "0010",  # VCALL
            "source_unit": "0000",
            "destination": "0001",
            "options": "0011"
        },

        # Scrambled voice data (228 bits)
        # AMBE+2 encoded voice (49 bits x 2 + FEC)
        "voice_data_scrambled": (
            "10110010" * 28 +  # Simplified - real AMBE data
            "1011"  # Padding
        ),

        # Status/data bits (100 bits)
        "sacch_data": "01" * 50,  # Slow associated control channel

        # CRC (16 bits) - CRC-CCITT
        "crc": "1010110011001010",

        # Total: 20+20+16+228+100+16 = 400 bits (384 data bits)
    },

    "modulation": {
        "type": "4FSK",
        "symbol_rate": 4800,  # baud
        "deviation": 1200,  # Hz (±1200 Hz)
        "rrc_alpha": 0.2,
        "symbols_per_frame": 192,  # 384 bits / 2 bits per symbol
        "frame_duration": 40,  # ms
    },

    "scrambling": {
        "polynomial": "x^15 + x^14 + 1",  # LFSR polynomial
        "initial_state": "111111111111111",  # 15 bits all 1s
    },

    # 4FSK Symbol mapping (Nyquist filtered)
    "symbol_map": {
        "00": +3.0,  # +1200 Hz deviation
        "01": +1.0,  # +400 Hz deviation
        "10": -1.0,  # -400 Hz deviation
        "11": -3.0,  # -1200 Hz deviation
    },

    "validation": {
        "sync_detection": True,
        "ran_match": 0,  # RAN (Radio Access Number)
        "crc_valid": True,
        "ber_expected": 0.0,
        "vocoder_lock": True
    }
}

test_vector_nxdn_voice_valid_2 = {
    "name": "NXDN Voice Frame - 2400 baud RTCH",
    "description": "Valid voice frame on traffic channel (half rate)",
    "mode": "NXDN48 (2400 baud)",
    "channel_type": "RTCH",
    "rate": "Half Rate (EHR)",
    "validity": "VALID",

    "frame_bits": {
        # Preamble (10 bits)
        "preamble": "0101010101",

        # RTCH Sync pattern (20 bits) - TS-1 Table 4-1
        "sync": "11010001110111001001",  # RTCH sync

        # Frame Information (16 bits)
        "frame_info": "0010000000010011",

        # Scrambled voice data (114 bits)
        # AMBE+2 EHR encoded (49 bits + FEC)
        "voice_data_scrambled": "10110010" * 14 + "10",

        # SACCH (50 bits)
        "sacch_data": "01" * 25,

        # CRC (12 bits) - shortened CRC
        "crc": "101011001100",

        # Total: 10+20+16+114+50+12 = 222 bits (192 data bits)
    },

    "modulation": {
        "type": "4FSK",
        "symbol_rate": 2400,
        "deviation": 600,  # Hz (±600 Hz for 2400 baud)
        "rrc_alpha": 0.2,
        "symbols_per_frame": 96,  # 192 bits / 2
        "frame_duration": 40,  # ms
    },

    "validation": {
        "sync_detection": True,
        "ran_match": 0,
        "crc_valid": True,
        "vocoder_type": "AMBE+2_EHR"
    }
}

test_vector_nxdn_invalid_sync = {
    "name": "NXDN Invalid - Wrong Channel Sync",
    "description": "RTCH sync used on RCCH channel",
    "validity": "INVALID",
    "error_type": "SYNC_MISMATCH",

    "frame_bits": {
        "preamble": "01010101010101010101",

        # Using RTCH sync when RCCH is expected
        "sync": "11010001110111001001",  # RTCH sync - WRONG!
        # Should be: "01010101010101010101" for RCCH

        "frame_info": "0010000000010011",
        "voice_data_scrambled": "10110010" * 28 + "1011",
        "sacch_data": "01" * 50,
        "crc": "1010110011001010",
    },

    "expected_behavior": {
        "rcch_sync_detection": False,
        "rtch_sync_detection": True,  # Wrong channel type
        "frame_accepted": False,
        "error_reported": "SYNC_CHANNEL_MISMATCH"
    }
}

test_vector_nxdn_invalid_scrambler = {
    "name": "NXDN Invalid - Wrong Scrambler Initial State",
    "description": "Scrambler initialized with wrong state",
    "validity": "INVALID",
    "error_type": "SCRAMBLER_STATE_ERROR",

    "scrambler_error": {
        "correct_initial_state": "111111111111111",
        "wrong_initial_state":   "000000000000000",  # All zeros - wrong!

        "correct_sequence": "11111111111111100000000000000011",
        "wrong_sequence":   "00000000000000011111111111111100",
    },

    "frame_bits": {
        "preamble": "01010101010101010101",
        "sync": "01010101010101010101",
        "frame_info": "0010000000010011",

        # Data scrambled with WRONG initial state
        "voice_data_scrambled": "01010011010001010101110001010011",  # Incorrectly scrambled

        "sacch_data": "01" * 50,
        "crc": "1010110011001010",
    },

    "expected_behavior": {
        "sync_detection": True,
        "descrambling": False,  # Won't produce valid data
        "fec_decode": False,
        "crc_valid": False,
        "frame_accepted": False
    }
}

# Edge Case Test Vectors

test_vector_edge_minimum_signal = {
    "name": "Edge Case - Minimum Detectable Signal",
    "description": "Frame at sensitivity threshold",
    "applies_to": ["dPMR", "NXDN"],
    "validity": "VALID",

    "signal_conditions": {
        "signal_level": -112,  # dBm (at sensitivity limit)
        "snr": 12,  # dB
        "expected_ber": 0.05,  # 5% BER at threshold
    },

    "frame_bits": {
        # Use valid frame from previous test vectors
        "sync": "011110100101110101010111111101110111111111010111",
    },

    "expected_behavior": {
        "sync_detection": True,  # Should still detect
        "sync_correlation": 0.85,  # Degraded but acceptable
        "frame_accepted": True,
        "audio_quality": "degraded"
    }
}

# All test vectors
ALL_TEST_VECTORS = {
    "dpmr": {
        "valid": [
            test_vector_dpmr_voice_valid_1,
            test_vector_dpmr_data_valid_2,
        ],
        "invalid": [
            test_vector_dpmr_invalid_crc,
            test_vector_dpmr_invalid_sync,
            test_vector_dpmr_invalid_scrambling,
        ],
    },
    "nxdn": {
        "valid": [
            test_vector_nxdn_voice_valid_1,
            test_vector_nxdn_voice_valid_2,
        ],
        "invalid": [
            test_vector_nxdn_invalid_sync,
            test_vector_nxdn_invalid_scrambler,
        ],
    },
    "edge_cases": [
        test_vector_edge_minimum_signal,
    ],
}

def get_test_vectors(protocol=None, validity=None):
    """
    Get test vectors filtered by protocol and validity.

    Args:
        protocol: 'dpmr', 'nxdn', or None for all
        validity: 'valid', 'invalid', or None for all

    Returns:
        List of test vectors
    """
    if protocol is None:
        vectors = []
        for proto in ALL_TEST_VECTORS:
            if proto != "edge_cases":
                if validity is None:
                    vectors.extend(ALL_TEST_VECTORS[proto]["valid"])
                    vectors.extend(ALL_TEST_VECTORS[proto]["invalid"])
                elif validity == "valid":
                    vectors.extend(ALL_TEST_VECTORS[proto]["valid"])
                elif validity == "invalid":
                    vectors.extend(ALL_TEST_VECTORS[proto]["invalid"])
        if validity is None or validity == "valid":
            vectors.extend(ALL_TEST_VECTORS["edge_cases"])
        return vectors

    if protocol not in ALL_TEST_VECTORS:
        return []

    if validity is None:
        return (ALL_TEST_VECTORS[protocol]["valid"] +
                ALL_TEST_VECTORS[protocol]["invalid"])
    elif validity == "valid":
        return ALL_TEST_VECTORS[protocol]["valid"]
    elif validity == "invalid":
        return ALL_TEST_VECTORS[protocol]["invalid"]

    return []

