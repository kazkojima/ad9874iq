#
# This file is part of AD9874IQ.
#
# Copyright (c) 2022 Kaz Kojima <kkojima@rr.iij4u.or.jp>
# SPDX-License-Identifier: BSD-2-Clause

import math

from migen import *

from litex.gen import *

from litex.soc.interconnect import stream
from litex.soc.interconnect.csr import *

class PWMI(Module, AutoCSR):
    def __init__(self, pads, data_width, sys_clk_freq, base_clk_freq, with_csr=True):

        self.pads       = pads
        self.length     = data_width

        self.start       = Signal()
        self.done        = Signal()
        self.data        = Signal(data_width)
        self.clk_divider = Signal(16, reset=math.ceil(sys_clk_freq/base_clk_freq))

        if with_csr:
            self.add_csr()

        pp_enable   = Signal()
        xfer_enable = Signal()
        count       = Signal(16)
        wcount      = Signal(2)
        bitsel      = Signal(max=data_width)
        self.comb += wcount.eq(count[0:2])

        length = self.length
        pre_length = 8
        post_length = 8

        # PWM base clock generation
        clk_divider = Signal(16)
        clk_rise    = Signal()
        clk_fall    = Signal()
        self.comb += clk_rise.eq(clk_divider == (self.clk_divider[1:] - 1))
        self.comb += clk_fall.eq(clk_divider == (self.clk_divider     - 1))
        self.sync += [
            clk_divider.eq(clk_divider + 1),
            If(clk_fall,
                clk_divider.eq(0)
            )
        ]

        # Control FSM
        self.submodules.fsm = fsm = FSM(reset_state="IDLE")
        fsm.act("IDLE",
            self.done.eq(1),
            If(self.start,
                self.done.eq(0),
                NextState("START")
            )
        )
        fsm.act("START",
            NextValue(count, 0),
            If(clk_rise,
                NextState("PREAMBLE")
            )
        )
        fsm.act("PREAMBLE",
            pp_enable.eq(1),
            If(clk_rise,
                NextValue(count, count + 1),
                If(count == (4*pre_length - 1),
                   NextValue(count, 0),
                   NextValue(bitsel, length - 1),
                   NextState("RUN")
                )
            )
        )
        fsm.act("RUN",
            xfer_enable.eq(1),
            If(clk_rise,
                NextValue(count, count + 1),
                If(wcount == 3,
                   NextValue(bitsel, bitsel - 1),
                ),
                If(count == (4*length - 1),
                   NextValue(count, 0),
                   NextState("POSTAMBLE")
                )
            )
        )
        fsm.act("POSTAMBLE",
            pp_enable.eq(1),
            If(clk_rise,
                NextValue(count, count + 1),
                If(count == (4*post_length - 1),
                   NextValue(count, 0),
                   NextState("STOP")
                )
            )
        )
        fsm.act("STOP",
            If(clk_rise,
                NextState("IDLE")
            )
        )

        # PWMOUT generation
        data_array = Array(self.data[i] for i in range(length))

        self.sync += [
            If(xfer_enable,
                If(wcount < (2*data_array[bitsel] + 1),
                    pads.pwmout.eq(1)
                ).Else(
                    pads.pwmout.eq(0)
                )
            ).Elif(pp_enable,
                If(wcount < 2,
                    pads.pwmout.eq(1)
                ).Else(
                    pads.pwmout.eq(0)
                )
            ),
        ]

    def add_csr(self):
        # Control / Status
        self._control = CSRStorage(description="PWMI Control.", fields=[
            CSRField("start",  size=1, offset=0, pulse=True, description="PWMI Xfer Start (Write ``1`` to start Xfer)."),
        ])
        self._status = CSRStatus(description="PWMI Status.", fields=[
            CSRField("done", size=1, offset=0, description="PWMI Xfer Done (when read as ``1``).")
        ])
        self.comb += [
            self.start.eq(self._control.fields.start),
            self._status.fields.done.eq(self.done),
        ]

        # PWMOUT
        self._data = CSRStorage(self.length, reset_less=True, description="PWMI output data (MSB-first serialization).")
        self.comb += [
            self.data.eq(self._data.storage),
        ]

    def add_clk_divider(self):
        self._clk_divider = CSRStorage(16, description="PWMI Clk Divider.", reset=self.clk_divider.reset)
        self.comb += self.clk_divider.eq(self._clk_divider.storage)
