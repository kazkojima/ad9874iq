[> Intro
--------
AD9874IQ provides a simple AD9874 SSI receiver.

Only simple SSI configurations are supported ATM.

examples/colorlight_i5.py is a running example using LiteX on Colorlight-i5 board. To make this example work, you need to add a few lines for the AD9874 SSI like:
```
--- a/litex_boards/platforms/colorlight_i5.py
+++ b/litex_boards/platforms/colorlight_i5.py
@@ -109,6 +109,13 @@ _io_v7_0 = [ # Documented by @smunaut
         Subsignal("data2_p", Pins("C20"), IOStandard("LVCMOS33D"), Misc("DRIVE=4")),
         #Subsignal("data2_n", Pins("D19"), IOStandard("LVCMOS33D"), Misc("DRIVE=4")),
     ),
+
+    ("ssi", 0,
+        Subsignal("data", Pins("D1")),
+        Subsignal("clk", Pins("C1")),
+        Subsignal("frame", Pins("C2")),
+        IOStandard("LVCMOS33")
+    ),
 ]
 
 # From https://github.com/wuxx/Colorlight-FPGA-Projects/blob/master/schematic/i5_v6.0-extboard.pdf and
```
to the litex platform description file platform/colorlight_i5.py.

The following code snippet added to litex bios will send 16-bit IQ samples to host udp 6000 port forever. Instead of patching bios, you can use ad9874iq example app [calling litex bios functions](https://github.com/kazkojima/litex-bioscall-examples).
```
diff --git a/litex/soc/software/bios/boot.c b/litex/soc/software/bios/boot.c
index aeaa8665..7eed65e1 100644
--- a/litex/soc/software/bios/boot.c
+++ b/litex/soc/software/bios/boot.c
@@ -576,6 +576,42 @@ void netboot(int nb_params, char **params)
 	printf("Network boot failed.\n");
 }
 
+#ifdef CSR_AD9874IQ_BASE
+unsigned int test_ad9874()
+{
+       unsigned int ip;
+
+       printf("Generate packets with IQ data\n");
+       printf("Local IP : %d.%d.%d.%d\n", LOCALIP1, LOCALIP2, LOCALIP3, LOCALIP4);
+       printf("Remote IP: %d.%d.%d.%d\n", REMOTEIP1, REMOTEIP2, REMOTEIP3, REMOTEIP4);
+
+       ip = IPTOINT(REMOTEIP1, REMOTEIP2, REMOTEIP3, REMOTEIP4);
+       udp_start(macadr, IPTOINT(LOCALIP1, LOCALIP2, LOCALIP3, LOCALIP4));
+
+        if(!udp_arp_resolve(ip)) {
+               printf("arp resolve fail\n");
+                return -1;
+       }
+
+       int j;
+       unsigned int iqdat[256];
+       char *buf;
+
+       for (;;) {
+               for (j = 0; j < 256; j++) {
+                       while (*(volatile char *)CSR_AD9874IQ_READY_ADDR == 0) ;
+                       iqdat[j] = *(volatile int *)CSR_AD9874IQ_DATA_ADDR;
+               }
+               buf = udp_get_tx_buffer();
+               for (j = 0; j < 256*4; j++)
+                       buf[j] = ((char *)iqdat)[j];
+               udp_send(6001, 6000, 256*4);
+       }
+       return 0;
+}
+
+#endif
+
 unsigned int tftp_load(int nb_params, char **params)
 {
 	unsigned int ip;
diff --git a/litex/soc/software/bios/main.c b/litex/soc/software/bios/main.c
index 93fa9271..97faf3d2 100644
--- a/litex/soc/software/bios/main.c
+++ b/litex/soc/software/bios/main.c
@@ -50,6 +50,12 @@
 #include <liblitesdcard/sdcard.h>
 #include <liblitesata/sata.h>
 
+#if defined(CSR_AD9874IQ_BASE) && defined(CSR_ETHMAC_BASE)
+extern unsigned int test_ad9874();
+
+define_command(test_ad9874, test_ad9874, "Generate IQ packets", LITEETH_CMDS);
+#endif
+
 #if defined(CSR_ETHMAC_BASE)
 extern unsigned int tftp_load(int nb_params, char **params);
 

```

[> Updates
----------

[> Features
-----------
**TODO**

[> Getting started
------------------
**TODO**

AD9874IQ module is writen with Amaranth and its verilog output is needed to use it from LiteX. Also it uses amaranth-community-unofficial's amlib. The command below will generate the verilog file. 
```
pushd ad9874iq/verilog
make
popd
```

[> Tests
--------
**TODO**

![AD9874IQ test](https://github.com/kazkojima/ad9874iq/blob/main/doc/ad9874iq-test.png)

[> Links
-------------

* [ANALOG DEVICES, AD9874 Data Sheet](https://www.analog.com/media/en/technical-documentation/data-sheets/AD9874.pdf)
* [LiteX](https://github.com/enjoy-digital/litex)
* [Amaranth HDL](https://github.com/amaranth-lang)
* [amlib: Assorted library of utility cores for amaranth HDL](https://github.com/amaranth-community-unofficial/amlib)
