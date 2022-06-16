#
# This file is part of AD9874IQ.
#
# Copyright (c) 2022 Kaz Kojima <kkojima@rr.iij4u.or.jp>
# SPDX-License-Identifier: BSD-2-Clause

from enum import Enum

from amaranth import *
from amaranth.build    import Platform
from amaranth.lib.cdc  import FFSynchronizer
from amaranth.lib.fifo import SyncFIFOBuffered
from amaranth.cli import main

from amlib.utils  import rising_edge_detected, falling_edge_detected
from amlib.test import GatewareTestCase, sync_test_case

class AD9874SSI_FORMAT(Enum):
    IQ16     = 1
    IQ24     = 2

class AD9874SSIReceiver(Elaboratable):
    """ AD9874 SSI Receiver

        Attributes
        ----------
        frame_start_in: Signal(), input
            frame start pulse
        serial_clock_in: Signal(), input
            SSI bit clock
        serial_data_in: Signal(), input
            SSI serial data
        iq_data_out: Signal(iq_width), out
            IQ data signal
        iq_strobe_out: Signal()
            reports iq_data_out available

        Parameters
        ----------
        frame_format: SSI_FORMAT
            choice of frame format
    """
    def __init__(self, *, frame_format: AD9874SSI_FORMAT = AD9874SSI_FORMAT.IQ16):
        self._frame_format = frame_format

        if frame_format == AD9874SSI_FORMAT.IQ16:
            iq_width = 16*2
        elif frame_format == AD9874SSI_FORMAT.IQ24:
            iq_width = 24*2
        else:
            raise ValueError("Unsupported frame format {}".format(frame_format))
        self.iq_width = iq_width

        self.serial_clock_in  = Signal()
        self.serial_data_in   = Signal()
        self.frame_start_in   = Signal()
        self.test_data_out    = Signal()
        self.iq_data_out      = Signal(iq_width)
        self.iq_strobe_out    = Signal()

    def elaborate(self, platform: Platform) -> Module:
        m = Module()
        frame_format = self._frame_format
        iq_width = self.iq_width

        bit_clock  = Signal()
        frame_clock = Signal()
        m.submodules.bit_clock_synchronizer  = FFSynchronizer(self.serial_clock_in, bit_clock)
        m.submodules.frame_clock_synchronizer = FFSynchronizer(self.frame_start_in, frame_clock)

        bit_clock_rose  = Signal()
        bit_clock_fell  = Signal()
        m.d.comb += [
            bit_clock_rose .eq(rising_edge_detected(m, bit_clock)),
            bit_clock_fell.eq(falling_edge_detected(m, bit_clock)),
        ]
        frame_clock_rose  = Signal()
        frame_clock_fell  = Signal()
        m.d.comb += [
            frame_clock_rose .eq(rising_edge_detected(m, frame_clock)),
            frame_clock_fell.eq(falling_edge_detected(m, frame_clock)),
        ]

        rx_buf = Signal(iq_width)
        rx_cnt = Signal(range(iq_width+1))

        with m.FSM(reset="IDLE"):
            with m.State("IDLE"):
                m.d.sync += [
                    self.iq_strobe_out.eq(0),
                    rx_buf.eq(0),
                ]
                with m.If(frame_clock_rose):
                    m.next = "WAIT_FRAME_START"

            with m.State("WAIT_FRAME_START"):
                m.d.sync += rx_cnt.eq(iq_width),
                with m.If(frame_clock_fell):
                    m.next = "WAIT_BIT"
                
            with m.State("WAIT_BIT"):
                with m.If(rx_cnt == 0):
                    m.next = "FRAME_END"
                with m.Else():
                    with m.If(bit_clock_fell):
                        m.d.sync += [
                            rx_buf.eq(Cat(self.serial_data_in, rx_buf[0:-1])),
                            rx_cnt.eq(rx_cnt - 1),
                        ]

            with m.State("FRAME_END"):
                m.next = "IDLE"
                m.d.sync += [
                    self.iq_data_out.eq(rx_buf),
                    self.iq_strobe_out.eq(1),
                ]

        return m

_TEST_SAMPLES = [0x12345678, 0xabcdef01, 0x23456789]

class AD9874SSITest(GatewareTestCase):
    FRAGMENT_UNDER_TEST = AD9874SSIReceiver
    FRAGMENT_ARGUMENTS = {}

    @sync_test_case
    def test_basic(self):
        dut = self.dut
        NF = len(_TEST_SAMPLES)
        yield dut.frame_start_in.eq(0)
        yield dut.serial_clock_in.eq(0)
        yield dut.serial_data_in.eq(0)
 
        for i in range(NF):
            yield dut.frame_start_in.eq(1)
            yield dut.serial_clock_in.eq(1)
            yield
            yield
            yield
            yield
            yield dut.serial_clock_in.eq(0)
            yield dut.frame_start_in.eq(0)
            yield
            yield
            yield
            yield
            for j in range(32):
                yield dut.serial_clock_in.eq(1)
                yield
                yield
                yield
                yield
                yield dut.serial_data_in.eq((_TEST_SAMPLES[i] >> (31-j)) & 1)
                yield dut.serial_clock_in.eq(0)
                yield
                yield
                yield
                yield
            yield dut.serial_clock_in.eq(1)
            yield
            yield
            yield
            yield
            yield dut.serial_data_in.eq(0)
            yield dut.serial_clock_in.eq(0)
            yield
            yield
            yield
            yield

if __name__ == "__main__":

    ssi = AD9874SSIReceiver()

    ports = [
        ssi.serial_clock_in,
        ssi.frame_start_in,
        ssi.serial_data_in,
        ssi.iq_data_out,
        ssi.iq_strobe_out
    ]
    main(ssi, name="AD9874SSIReceiver", ports=ports)
