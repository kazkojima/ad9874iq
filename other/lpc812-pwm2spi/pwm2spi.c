/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   Under Section 7 of GPL version 3, you are granted additional
   permissions described in EXCEPTION.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <stdbool.h>

extern void _start (void);
static void sct_handler (void);

static void __attribute__ ((naked))
reset (void)
{
  asm volatile ("cpsid	i\n\t"		/* Mask all interrupts. */
		"ldr	r0, 0f\n\t"	/* Go to entry. */
		"bx	r0\n\t"
		".align	2\n"
	"0:	.word	_start"
		: /* no output */ : /* no input */ : "memory");
  /* Never reach here. */
}

static void
unexpected (void)
{
  for (;;);
}

typedef void (*handler)(void);
extern uint8_t __ram_end__;

#if defined (ENABLE_ISP)
uint32_t crp __attribute__ ((section(".crp"))) = 0;
#else
// Code Red protection: NO_ISP
uint32_t crp __attribute__ ((section(".crp"))) = 0x4E697370;
#endif

handler vector[48] __attribute__ ((section(".vectors"))) = {
  (handler)&__ram_end__,
  reset,
  unexpected,	/*  2: NMI */
  unexpected,	/*  3: Hard fault */
  unexpected, unexpected, unexpected, unexpected, /* 4-7 */
  unexpected, unexpected, unexpected, /* 8-10 */
  unexpected,	/* 11: SVC */
  unexpected, unexpected, /* 12-13 */
  unexpected,	/* 14: Pend SV */
  unexpected,	/* 15: SysTick */
  unexpected,	/* 16: SPI0 */
  unexpected,	/* 17: SPI1 */
  unexpected,	/* 18: Reserve */
  unexpected,	/* 19: UART0 */
  unexpected,	/* 20: UART1 */
  unexpected,	/* 21: UART2 */
  unexpected,	/* 22: Reserve */
  unexpected,	/* 23: Reserve */
  unexpected,	/* 24: I2C */
  sct_handler,	/* 25: SCT */
  unexpected,	/* 26: MRT */
  unexpected,	/* 27: CMP */
  unexpected,	/* 28: WDT */
  unexpected,	/* 29: BOD */
  unexpected,	/* 30: Reserve */
  unexpected,	/* 31: WKT */
  unexpected, unexpected, unexpected, unexpected, /* 32-35 */
  unexpected, unexpected, unexpected, unexpected, /* 36-39 */
  unexpected, unexpected, unexpected, unexpected, /* 40-43 PININT0-3 */
  unexpected, unexpected, unexpected, unexpected, /* 44-47 PININT4-7 */
};

#include "lpc8xx.h"

#define SPI0_IRQn (16-16)
#define SCT_IRQn (25-16)

// SCT counter no prescale 1/1 which means 30MHz clock
#define SCT_CTRL_PRESCALE	(0 << 5)

// SPI registers bit definitions
#define SPI_CFG_ENABLE		(0x1 << 0)
#define SPI_CFG_MASTER		(0x1 << 2)
#define SPI_CFG_LSBF		(0x1 << 3)
#define SPI_CFG_CPHA		(0x1 << 4)
#define SPI_CFG_CPOL		(0x1 << 5)
#define SPI_CFG_SPOL		(0x1 << 8)
#define SPI_STAT_RXRDY		(0x1 << 0)
#define SPI_STAT_TXRDY		(0x1 << 1)
#define SPI_STAT_RXOV		(0x1 << 2)
#define SPI_STAT_TXUR		(0x1 << 3)
#define SPI_STAT_SSA		(0x1 << 4)
#define SPI_STAT_SSD		(0x1 << 5)
#define SPI_STAT_STALLED	(0x1 << 6)
#define SPI_STAT_ENDTRANSFER	(0x1 << 7)
#define SPI_STAT_MSTIDLE	(0x1 << 8)
#define SPI_RXDAT_RXSSEL_N	(0x1 <<16)
#define SPI_RXDAT_SOT		(0x1 <<20)
#define SPI_TXDATCTL_RXIGNORE	(0x1 << 22)
#define SPI_TXDATCTL_FLEN(n)	((n & 0xf) << 24)	

static void main_loop (void);

extern char _edata[], _data[], _textdata[];
extern char _bss_start[], _bss_end[];

void
_start (void)
{
  // Copy .data section from flash.  All are word aligned.  See linker script.
  uint32_t *p = (uint32_t *) _data;
  uint32_t *q = (uint32_t *) _textdata;
  uint32_t size = (uint32_t *) _edata - (uint32_t *) _data;

  while (size--)
    *p++ = *q++;

  // Clear .bss.  Also word aligned.
  p = (uint32_t *) _bss_start;
  size = (uint32_t *) _bss_end - (uint32_t *) _bss_start;
  while (size--)
    *p++ = 0;

  // 30Mhz: main 60Mhz, P=2, M=(4+1), 60*2*2=240Mhz 240/(2*2)/5=12Mz
  LPC_SYSCON->SYSPLLCTRL = 0x24;
  LPC_SYSCON->SYSAHBCLKDIV = 0x02;

  LPC_SYSCON->PDRUNCFG &= ~(0x80);
  while (!(LPC_SYSCON->SYSPLLSTAT & 0x01))
    ;

  LPC_SYSCON->MAINCLKSEL = 0x03;
  LPC_SYSCON->MAINCLKUEN = 0x01;
  while (!(LPC_SYSCON->MAINCLKUEN & 0x01))
    ;

  // 1 wait state for flash
  LPC_FLASHCTRL->FLASHCFG &= ~(0x03);

  // Enable GPIO(6), SPI0(11), SCT(8) clock
  LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 6)|(1 << 8)|(1 << 11);

  // Reset GPIO(10), SPI0(0) and SCT(6)
  LPC_SYSCON->PRESETCTRL &= ~((1 << 0)|(1 << 8)|(1 << 10));
  LPC_SYSCON->PRESETCTRL |= ((1 << 0)|(1 << 8)|(1 << 10));

  // Enable WAKEUP on PIO0_4 (default)
  // LPC_PMU->DPDCTRL = 0x0;

  // Enable SPI0_SCK on PIO0_8 PINASSIGN3(31:24)
  LPC_SWM->PINASSIGN3 = 0x08ffffff;
  // Enable SPI0_MOSI on PIO0_9 PINASSIGN4(7:0)
  // Enable SPI0_SSEL on PIO0_1 PINASSIGN4(23:16)
  LPC_SWM->PINASSIGN4 = 0xff01ff09;
  // Enable SCIN_0 on PIO0_0 PINASSIGN5(31:24)
  LPC_SWM->PINASSIGN5 = 0x00ffffff;

  // PIO0_6 is an output, default High
  LPC_GPIO_PORT->B0[6] = 1;
  LPC_GPIO_PORT->DIR0 |= (1 << 6);

  // Configure SPI0 as master
  LPC_SPI0->CFG = SPI_CFG_ENABLE|SPI_CFG_MASTER;
  // 30/(5+1) = 5MHz SPI clock speed
  LPC_SPI0->DIV = 4;

  // [0]=0:16-bit op, [2:1]=0:Prescaled bus clock, [16:9]=1:INSYNC, other 0
  LPC_SCT->CONFIG = 0x0|(1 << 9);
  // As capture registers
  LPC_SCT->REGMODE_L = 0x1f;
  LPC_SCT->REGMODE_H = 0x1f;
  // Enable event 0 in state 0
  LPC_SCT->EVENT[0].STATE = 0x1;
  // [5]=0:Input, [9:6]=0:CTIN_0, [11:10]=1: Rise, [13:12]=2: IO
  LPC_SCT->EVENT[0].CTRL = (1 << 10)|(2 << 12);
  // Enable event 1 in state 0
  LPC_SCT->EVENT[1].STATE = 0x1;
  // [5]=0:Input, [9:6]=0:CTIN_0, [11:10]=2: Fall, [13:12]=2: IO
  LPC_SCT->EVENT[1].CTRL = (2 << 10)|(2 << 12);
  // Reset counters for event 0
  LPC_SCT->LIMIT_L = 1;
  LPC_SCT->LIMIT_H = 1;
  // Capture 0.L for event 0
  LPC_SCT->CAPCTRL[0].L = 1;
  // Capture 0.H for event 1
  LPC_SCT->CAPCTRL[0].H = 2;

  // Enable SCT intr with NVIC
  NVIC_ISER = (1 << SCT_IRQn);

  if (!(LPC_PMU->PCON & (1<<11))) {
    // Wait 20ms not to get noise when going on hot start
    *SYST_RVR = 6000000-1;
    *SYST_CVR = 0;
    *SYST_CSR = 5;
    while (!(*SYST_CSR & (1 << 16)))
      ;
  } else {
    // Clear the DPDFLAG bit when waken from deep power-down
    LPC_PMU->PCON = (1<<11);
  }

  // Start, clear counter
  LPC_SCT->CTRL_L = (1 << 3)|SCT_CTRL_PRESCALE;
  LPC_SCT->CTRL_H = (1 << 3)|SCT_CTRL_PRESCALE;

  // Enable SCT event 0 (rise) to request interrupt.
  LPC_SCT->EVEN = 0x1;

  main_loop ();
}

static void
deep_powerdown (void)
{
  // Wait for count overrun 1000 which means no rising edge for 33us
  while (LPC_SCT->COUNT_L < 1000)
    ;
  // Assume WAKEUP pin is high
  // Ensure the bit 3 of PCON is cleared
  // Write 0x3 to the PM bits of PCON
  LPC_PMU->PCON = 0x3;
  // Write one to the SLEEPDEEP bit of SCR
  *SCR = SCR_SLEEPDEEP;
  asm volatile ("wfi" : : : "memory");
}

#define POWERDOWN 0xfe

#define BUFSZ 64
#define BUFSZMASK (BUFSZ-1)
static uint16_t buf[BUFSZ];
static volatile int bufin, bufout, bufcnt;

static void
pushbuf(uint16_t elm)
{
  if (bufcnt == BUFSZ)
    return;

  buf[bufin] = elm;
  bufin = (bufin+1) & BUFSZMASK;
  bufcnt++;
}

static uint16_t
popbuf(void)
{
  uint16_t elm = buf[bufout];
  bufout = (bufout+1) & BUFSZMASK;
  bufcnt--;
  return elm;
}

static bool
emptybuf(void)
{
  return (bufcnt == 0);
}

static int bitcount = 0;
static uint16_t pwm_word = 0;

// Jitter limit ~130ns
#define JITTER 4

//static uint16_t tlast;

static void
sct_handler (void)
{
  uint16_t width, high;
  uint32_t ev;
  int dt;

  // This handler should be called only on event 0 i.e. rising edge of SCIN.
  ev = LPC_SCT->EVFLAG;
  LPC_SCT->EVFLAG = (ev & 3);
#if 0
  if ((ev & 3) != 3)
    return;
#endif

  width = LPC_SCT->CAP[0].L;
  high = LPC_SCT->CAP[0].H;

  dt = (width>>1) - high;
  if (-JITTER < dt && dt < JITTER)
    {
      // Mark
      if (bitcount == 16)
	{
	  pushbuf(pwm_word);
	}
      bitcount = 0;
      pwm_word = 0;
    }
  else
    {
      dt = (width>>2) - high;
      if (-JITTER < dt && dt < JITTER)
	{
	  // 0
	  pwm_word = (pwm_word << 1);
	  bitcount++;
	}
      else
	{
	  dt += (width>>1);
	  if (-JITTER < dt && dt < JITTER)
	    {
	      // 1
	      pwm_word = (pwm_word << 1)|1;
	      bitcount++;
	    }
#if 0
	  else
	    {
	      LPC_GPIO_PORT->B0[6] = 0;
	      LPC_GPIO_PORT->B0[6] = 1;
	    }
#endif
	}
    }

  return;
}

static void
main_loop (void)
{
  uint32_t data = 0;

#if 0
  pushbuf(0xdead);
  pushbuf(0xbeef);
#endif

  // Unmask all interrupts
  asm volatile ("cpsie      i" : : : "memory");

  for (;;)
    {
      if (!emptybuf())
	{
	  asm volatile ("cpsid      i" : : : "memory");
	  data = (uint32_t) popbuf();
	  asm volatile ("cpsie      i" : : : "memory");

	  if ((data >> 8) == POWERDOWN)
	    deep_powerdown ();

	  while(~LPC_SPI0->STAT & SPI_STAT_TXRDY)
	    ;
	  // 16-bit, Ignore RX, EOT, assert SSEL
	  LPC_SPI0->TXDATCTL = SPI_TXDATCTL_FLEN(15)|(1<<22)|(1<<20)|(0xe<<16)|data;
#if 0
	  while(~LPC_SPI0->STAT & SPI_STAT_MSTIDLE)
	    ;
#endif
	}

    }
}
