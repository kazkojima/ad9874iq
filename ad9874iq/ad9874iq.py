#
# This file is part of AD9874IQ.
#
# Copyright (c) 2022 Kaz Kojima <kkojima@rr.iij4u.or.jp>
# SPDX-License-Identifier: BSD-2-Clause

from migen import *

from litex.gen import *

from litex.soc.interconnect import stream
from litex.soc.interconnect.csr import *

from . import data_file

class AD9874IQ(Module, AutoCSR):
    def __init__(self, platform, pads):

        platform.add_source(data_file("ssi.v"), "verilog")

        fifo = stream.AsyncFIFO([("data", 32)], 1024)
        fifo = ClockDomainsRenamer({"write": "ssi", "read": "sys"})(fifo)
        self.submodules.fifo = fifo

        # CPU side
        self.ready = CSRStatus(1)
        self.data = CSRStatus(32)
        self.comb += [
            fifo.source.ready.eq(self.data.we),
            self.data.status.eq(fifo.source.data),
            self.ready.status.eq(fifo.source.valid)
        ]

        # SSI side
        iq = Signal(32)
        iq_strobe_out = Signal()
        self.comb += [
            fifo.sink.data.eq(iq),
            fifo.sink.valid.eq(iq_strobe_out)
        ]

        self.specials += Instance("AD9874SSIReceiver",
                                  i_clk = ClockSignal("ssi"),
                                  i_rst = ResetSignal(),
                                  i_serial_data_in = pads.cdata,
                                  i_serial_clock_in = pads.cclk,
                                  o_iq_data_out = iq,
                                  o_iq_strobe_out = iq_strobe_out)
