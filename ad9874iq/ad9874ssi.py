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
        serial_clock_in: Signal(), input
            SSI bit clock
        serial_data_in: Signal(), input
            SSI serial data
        iq_data_out: Signal(iq_width), out
            IQ data signal
        iq_strobe_out: Signal()
            reports iq_data_out available
        delayed_clock: Signal(), input
            use delayed clock_in
        delayed_data: Signal(), input
            use delayed data_in
 
        Parameters
        ----------
        frame_format: SSI_FORMAT
            choice of frame format
        up_sample: int
            ratio of system clock and SSI bit clock
    """
    def __init__(self, *, frame_format: AD9874SSI_FORMAT = AD9874SSI_FORMAT.IQ16, up_sample = 8):
        self._frame_format = frame_format
        self._up_sample = up_sample

        if frame_format == AD9874SSI_FORMAT.IQ16:
            iq_width = 16*2
        elif frame_format == AD9874SSI_FORMAT.IQ24:
            iq_width = 24*2
        else:
            raise ValueError("Unsupported frame format {}".format(frame_format))
        self.iq_width = iq_width

        self.serial_clock_in  = Signal()
        self.serial_data_in   = Signal()
        self.test_data_out    = Signal()
        self.iq_data_out      = Signal(iq_width)
        self.iq_strobe_out    = Signal()
        self.delayed_clock    = Signal()
        self.delayed_data     = Signal()

    def elaborate(self, platform: Platform) -> Module:
        m = Module()
        frame_format = self._frame_format
        up_sample = self._up_sample
        iq_width = self.iq_width

        bit_clock_raw  = Signal()
        data_clock_raw = Signal()
        m.submodules.bit_clock_synchronizer  = FFSynchronizer(self.serial_clock_in, bit_clock_raw)
        m.submodules.data_clock_synchronizer = FFSynchronizer(self.serial_data_in, data_clock_raw)

        bit_clock  = Signal()
        data_clock = Signal()
        bit_clock_delayed  = Signal()
        data_clock_delayed = Signal()
        m.d.sync += [
            bit_clock_delayed.eq(bit_clock_raw),
            data_clock_delayed.eq(data_clock_raw),
        ]
        with m.If(self.delayed_clock):
            m.d.comb += bit_clock.eq(bit_clock_delayed)
        with m.Else():
            m.d.comb += bit_clock.eq(bit_clock_raw)
        
        with m.If(self.delayed_data):
            m.d.comb += data_clock.eq(data_clock_delayed)
        with m.Else():
            m.d.comb += data_clock.eq(data_clock_raw)
        
        bit_clock_rose  = Signal()
        bit_clock_fell  = Signal()
        m.d.comb += [
            bit_clock_rose .eq(rising_edge_detected(m, bit_clock)),
            bit_clock_fell.eq(falling_edge_detected(m, bit_clock)),
        ]
        count_f  = Signal(range(0,up_sample+1))
        count_d  = Signal(range(0,up_sample+1))
        bit_data = Signal()

        frame_threshold = up_sample - 2
        data_threshold = up_sample - 3

        with m.If(count_d >= data_threshold):
            m.d.comb += bit_data.eq(1)
        with m.Else():
            m.d.comb += bit_data.eq(0)

        with m.If(bit_clock):
            m.d.sync += count_f.eq(count_f + 1)
        with m.If(bit_clock_fell):
            m.d.sync += count_f.eq(0)

        rx_buf = Signal(iq_width)
        rx_cnt = Signal(range(iq_width+1))

        with m.FSM(reset="IDLE"):
            with m.State("IDLE"):
                m.d.sync += [
                    self.iq_strobe_out.eq(0),
                    rx_buf.eq(0),
                ]
                with m.If(count_f >= frame_threshold):
                    m.next = "WAIT_FRAME_START"

            with m.State("WAIT_FRAME_START"):
                m.d.sync += rx_cnt.eq(iq_width),
                m.next = "WAIT_BIT"
                
            with m.State("WAIT_BIT"):
                with m.If(rx_cnt == 0):
                    m.next = "FRAME_END"
                with m.Else():
                    with m.If(bit_clock_rose):
                        m.d.sync += [
                            rx_buf.eq(Cat(bit_data, rx_buf[0:-1])),
                            rx_cnt.eq(rx_cnt - 1),
                            count_d.eq(0),
                        ]
                    with m.Else():
                        m.d.sync += [
                            count_d.eq(count_d + (data_clock != bit_clock))
                        ]

            with m.State("FRAME_END"):
                m.next = "IDLE"
                m.d.sync += [
                    self.iq_data_out.eq(rx_buf),
                    self.iq_strobe_out.eq(1),
                ]

        return m

_TEST_SAMPLES = [0x12345678, 0xabcdef01, 0x23456789, 0xaaaaaaaa]

class AD9874SSITest(GatewareTestCase):
    FRAGMENT_UNDER_TEST = AD9874SSIReceiver
    FRAGMENT_ARGUMENTS = {}

    @sync_test_case
    def test_basic(self):
        dut = self.dut
        NF = len(_TEST_SAMPLES)
        yield dut.serial_clock_in.eq(0)
        yield dut.serial_data_in.eq(0)
 
        for i in range(NF):
            yield dut.serial_clock_in.eq(1)
            yield dut.serial_data_in.eq(0)
            yield
            yield
            yield
            yield
            yield
            yield dut.serial_data_in.eq(1)
            yield
            yield
            yield
            yield
            for j in range(32):
                yield dut.serial_clock_in.eq(1)
                yield dut.serial_data_in.eq(~((_TEST_SAMPLES[i] >> (31-j)) & 1))
                yield
                yield
                yield
                yield
                yield dut.serial_clock_in.eq(0)
                yield dut.serial_data_in.eq((_TEST_SAMPLES[i] >> (31-j)) & 1)
                yield
                yield
                yield
                yield

            yield dut.serial_clock_in.eq(1)
            yield dut.serial_data_in.eq(0)
            yield
            yield
            yield
            yield
            yield dut.serial_clock_in.eq(0)
            yield dut.serial_data_in.eq(1)
            yield
            yield
            yield
            yield

if __name__ == "__main__":

    ssi = AD9874SSIReceiver()

    ports = [
        ssi.serial_clock_in,
        ssi.serial_data_in,
        ssi.iq_data_out,
        ssi.iq_strobe_out,
        ssi.delayed_clock,
        ssi.delayed_data,
    ]
    main(ssi, name="AD9874SSIReceiver", ports=ports)
