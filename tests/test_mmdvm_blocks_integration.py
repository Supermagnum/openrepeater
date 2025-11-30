#!/usr/bin/env python3

"""

MMDVM Protocol Blocks Integration Tests

Tests the actual GNU Radio blocks for all four MMDVM protocols

"""

import sys
import unittest
import numpy as np

try:
    from gnuradio import gr
    from gnuradio import blocks
    import gnuradio.qradiolink as qradiolink
except ImportError as e:
    print(f"ERROR: Could not import GNU Radio modules: {e}")
    print("Make sure GNU Radio and gr-qradiolink are installed")
    sys.exit(1)


class TestPOCSAGBlocks(unittest.TestCase):
    """Test POCSAG encoder and decoder blocks"""

    def test_pocsag_encoder_creation(self):
        """Test POCSAG encoder can be created"""
        try:
            encoder = qradiolink.pocsag_encoder(baud_rate=1200, address=0x123456, function_bits=0)
            self.assertIsNotNone(encoder)
            print("✓ POCSAG encoder created successfully")
        except Exception as e:
            self.fail(f"Failed to create POCSAG encoder: {e}")

    def test_pocsag_decoder_creation(self):
        """Test POCSAG decoder can be created"""
        try:
            decoder = qradiolink.pocsag_decoder(baud_rate=1200, sync_threshold=0.8)
            self.assertIsNotNone(decoder)
            print("✓ POCSAG decoder created successfully")
        except Exception as e:
            self.fail(f"Failed to create POCSAG decoder: {e}")

    def test_pocsag_encoder_decoder_roundtrip(self):
        """Test POCSAG encoder -> decoder round trip"""
        try:
            tb = gr.top_block()

            # Create test message
            message = b"TEST MESSAGE\x00"  # Null-terminated
            source = blocks.vector_source_b(message.tolist() if hasattr(message, 'tolist') else list(message), False)

            # Create encoder and decoder
            encoder = qradiolink.pocsag_encoder(baud_rate=1200, address=0x123456, function_bits=0)
            decoder = qradiolink.pocsag_decoder(baud_rate=1200, sync_threshold=0.8)

            # Create sink
            sink = blocks.vector_sink_b()

            # Connect: source -> encoder -> decoder -> sink
            tb.connect(source, encoder)
            tb.connect(encoder, decoder)
            tb.connect(decoder, sink)

            # Run flowgraph
            tb.start()
            tb.wait()
            tb.stop()
            tb.wait()

            # Check that we got some output
            output = sink.data()
            self.assertGreater(len(output), 0, "Should produce some output")
            print(f"✓ POCSAG round trip: {len(output)} bytes output")

        except Exception as e:
            self.fail(f"POCSAG round trip failed: {e}")


class TestDSTARBlocks(unittest.TestCase):
    """Test D-STAR encoder and decoder blocks"""

    def test_dstar_encoder_creation(self):
        """Test D-STAR encoder can be created"""
        try:
            encoder = qradiolink.dstar_encoder(
                my_callsign="KE7XYZ ",
                your_callsign="CQCQCQ  ",
                rpt1_callsign="        ",
                rpt2_callsign="        "
            )
            self.assertIsNotNone(encoder)
            print("✓ D-STAR encoder created successfully")
        except Exception as e:
            self.fail(f"Failed to create D-STAR encoder: {e}")

    def test_dstar_decoder_creation(self):
        """Test D-STAR decoder can be created"""
        try:
            decoder = qradiolink.dstar_decoder(sync_threshold=0.9)
            self.assertIsNotNone(decoder)
            print("✓ D-STAR decoder created successfully")
        except Exception as e:
            self.fail(f"Failed to create D-STAR decoder: {e}")


class TestYSFBlocks(unittest.TestCase):
    """Test YSF encoder and decoder blocks"""

    def test_ysf_encoder_creation(self):
        """Test YSF encoder can be created"""
        try:
            encoder = qradiolink.ysf_encoder(
                source_callsign="KE7XYZ    ",
                destination_callsign="CQCQCQ    ",
                radio_id=12345,
                group_id=0
            )
            self.assertIsNotNone(encoder)
            print("✓ YSF encoder created successfully")
        except Exception as e:
            self.fail(f"Failed to create YSF encoder: {e}")

    def test_ysf_decoder_creation(self):
        """Test YSF decoder can be created"""
        try:
            decoder = qradiolink.ysf_decoder(sync_threshold=0.9)
            self.assertIsNotNone(decoder)
            print("✓ YSF decoder created successfully")
        except Exception as e:
            self.fail(f"Failed to create YSF decoder: {e}")


class TestP25Blocks(unittest.TestCase):
    """Test P25 encoder and decoder blocks"""

    def test_p25_encoder_creation(self):
        """Test P25 encoder can be created"""
        try:
            encoder = qradiolink.p25_encoder(
                nac=0x293,
                source_id=12345,
                destination_id=0,
                talkgroup_id=100
            )
            self.assertIsNotNone(encoder)
            print("✓ P25 encoder created successfully")
        except Exception as e:
            self.fail(f"Failed to create P25 encoder: {e}")

    def test_p25_decoder_creation(self):
        """Test P25 decoder can be created"""
        try:
            decoder = qradiolink.p25_decoder(sync_threshold=0.9)
            self.assertIsNotNone(decoder)
            print("✓ P25 decoder created successfully")
        except Exception as e:
            self.fail(f"Failed to create P25 decoder: {e}")


def run_all_tests():
    """Run all protocol block tests"""
    print("=" * 70)
    print("MMDVM Protocol Blocks Integration Tests")
    print("=" * 70)
    print()

    # Create test suite
    loader = unittest.TestLoader()
    suite = unittest.TestSuite()

    # Add all test classes
    suite.addTests(loader.loadTestsFromTestCase(TestPOCSAGBlocks))
    suite.addTests(loader.loadTestsFromTestCase(TestDSTARBlocks))
    suite.addTests(loader.loadTestsFromTestCase(TestYSFBlocks))
    suite.addTests(loader.loadTestsFromTestCase(TestP25Blocks))

    # Run tests
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)

    # Summary
    print()
    print("=" * 70)
    print("Test Summary")
    print("=" * 70)
    print(f"Tests run: {result.testsRun}")
    print(f"Failures: {len(result.failures)}")
    print(f"Errors: {len(result.errors)}")
    print(f"Success rate: {((result.testsRun - len(result.failures) - len(result.errors)) / result.testsRun * 100):.1f}%")

    return 0 if result.wasSuccessful() else 1


if __name__ == '__main__':
    sys.exit(run_all_tests())

