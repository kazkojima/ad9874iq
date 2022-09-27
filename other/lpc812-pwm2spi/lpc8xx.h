/* Simplified lpc8xx.h */

typedef struct
{
  uint32_t SYSMEMREMAP;
  uint32_t PRESETCTRL;
  uint32_t SYSPLLCTRL;
  uint32_t SYSPLLSTAT;
  uint32_t RESERVED0[4];

  uint32_t SYSOSCCTRL;
  uint32_t WDTOSCCTRL;
  uint32_t RESERVED1[2];
  uint32_t SYSRSTSTAT;
  uint32_t RESERVED2[3];
  uint32_t SYSPLLCLKSEL;
  uint32_t SYSPLLCLKUEN;
  uint32_t RESERVED3[10];

  uint32_t MAINCLKSEL;
  uint32_t MAINCLKUEN;
  uint32_t SYSAHBCLKDIV;
  uint32_t RESERVED4[1];

  uint32_t SYSAHBCLKCTRL;
  uint32_t RESERVED5[4];
  uint32_t UARTCLKDIV;
  uint32_t RESERVED6[18];

  uint32_t CLKOUTSEL;
  uint32_t CLKOUTUEN;
  uint32_t CLKOUTDIV;
  uint32_t RESERVED7;
  uint32_t UARTFRGDIV;
  uint32_t UARTFRGMULT;
  uint32_t RESERVED8[1];
  uint32_t EXTTRACECMD;
  uint32_t PIOPORCAP0;
  uint32_t RESERVED9[12];

  uint32_t IOCONCLKDIV[7];
  uint32_t BODCTRL;
  uint32_t SYSTCKCAL;
  uint32_t RESERVED10[6];
  uint32_t IRQLATENCY;
  uint32_t NMISRC;
  uint32_t PINTSEL[8];
  uint32_t RESERVED11[27];
  uint32_t STARTERP0;
  uint32_t RESERVED12[3];
  uint32_t STARTERP1;
  uint32_t RESERVED13[6];
  uint32_t PDSLEEPCFG;
  uint32_t PDAWAKECFG;
  uint32_t PDRUNCFG;
  uint32_t RESERVED14[110];
  uint32_t DEVICE_ID;
} LPC_SYSCON_TypeDef;

typedef struct {
  uint32_t PIO0_17;
  uint32_t PIO0_13;
  uint32_t PIO0_12;
  uint32_t PIO0_5;
  uint32_t PIO0_4;
  uint32_t PIO0_3;
  uint32_t PIO0_2;
  uint32_t PIO0_11;
  uint32_t PIO0_10;
  uint32_t PIO0_16;
  uint32_t PIO0_15;
  uint32_t PIO0_1;
  uint32_t Reserved;
  uint32_t PIO0_9;
  uint32_t PIO0_8;
  uint32_t PIO0_7;
  uint32_t PIO0_6;
  uint32_t PIO0_0;
  uint32_t PIO0_14;
} LPC_IOCON_TypeDef;

typedef struct {
  uint32_t  RESERVED0[4];
  uint32_t  FLASHCFG;
  uint32_t  RESERVED1[3];
  uint32_t  FMSSTART;
  uint32_t  FMSSTOP;
  uint32_t  RESERVED2;
  uint32_t  FMSW0;
} LPC_FLASHCTRL_TypeDef;

typedef struct
{
  uint32_t PCON;
  uint32_t GPREG0;
  uint32_t GPREG1;
  uint32_t GPREG2;
  uint32_t GPREG3;
  uint32_t DPDCTRL;
} LPC_PMU_TypeDef;

typedef struct {
  uint8_t B0[18];
  uint16_t RESERVED0[2039];
  uint32_t W0[18];
  uint32_t RESERVED1[1006];
  uint32_t DIR0;
  uint32_t RESERVED2[31];
  uint32_t MASK0;
  uint32_t RESERVED3[31];
  uint32_t PIN0;
  uint32_t RESERVED4[31];
  uint32_t MPIN0;
  uint32_t RESERVED5[31];
  uint32_t SET0;
  uint32_t RESERVED6[31];
  uint32_t CLR0;
  uint32_t RESERVED7[31];
  uint32_t NOT0;
} LPC_GPIO_PORT_TypeDef;

typedef struct
{
  union {
    uint32_t PINASSIGN[9];
    struct {
      uint32_t PINASSIGN0;
      uint32_t PINASSIGN1;
      uint32_t PINASSIGN2;
      uint32_t PINASSIGN3;
      uint32_t PINASSIGN4;
      uint32_t PINASSIGN5;
      uint32_t PINASSIGN6;
      uint32_t PINASSIGN7;
      uint32_t PINASSIGN8;
    };
  };
  uint32_t RESERVED0[103];
  uint32_t PINENABLE0;
} LPC_SWM_TypeDef;

typedef struct
{
  uint32_t CFG;
  uint32_t CTRL;
  uint32_t STAT;
  uint32_t INTENSET;
  uint32_t INTENCLR;
  uint32_t RXDATA;
  uint32_t RXDATA_STAT;
  uint32_t TXDATA;
  uint32_t BRG;
  uint32_t INTSTAT;
} LPC_USART_TypeDef;

typedef struct
{
  uint32_t  CFG;
  uint32_t  DLY;
  uint32_t  STAT;
  uint32_t  INTENSET;
  uint32_t  INTENCLR;
  uint32_t  RXDAT;
  uint32_t  TXDATCTL;
  uint32_t  TXDAT;
  uint32_t  TXCTRL;
  uint32_t  DIV;
  uint32_t  INTSTAT;
} LPC_SPI_TypeDef;

#define CONFIG_SCT_nEV   (6)             /* Number of events */
#define CONFIG_SCT_nRG   (5)             /* Number of match/compare registers */
#define CONFIG_SCT_nOU   (4)             /* Number of outputs */

typedef struct
{
  uint32_t CONFIG;
  union {
    uint32_t CTRL_U;
    struct {
      uint16_t CTRL_L;
      uint16_t CTRL_H;
    };
  };
  uint16_t LIMIT_L;
  uint16_t LIMIT_H;
  uint16_t HALT_L;
  uint16_t HALT_H;
  uint16_t STOP_L;
  uint16_t STOP_H;
  uint16_t START_L;
  uint16_t START_H;
  uint32_t RESERVED1[10];
  union {
    uint32_t COUNT_U;
    struct {
      uint16_t COUNT_L;
      uint16_t COUNT_H;
    };
  };
  uint16_t STATE_L;
  uint16_t STATE_H;
  uint32_t INPUT;
  uint16_t REGMODE_L;
  uint16_t REGMODE_H;
  uint32_t OUTPUT;
  uint32_t OUTPUTDIRCTRL;
  uint32_t RES;
  uint32_t RESERVED2[37];
  uint32_t EVEN;
  uint32_t EVFLAG;
  uint32_t CONEN;
  uint32_t CONFLAG;

  union {
    union {
      uint32_t U;
      struct {
	uint16_t L;
	uint16_t H;
      };
    } MATCH[CONFIG_SCT_nRG];
   union {
      uint32_t U;
      struct {
	uint16_t L;
	uint16_t H;
      };
    } CAP[CONFIG_SCT_nRG];
  };

  uint32_t RESERVED3[32-CONFIG_SCT_nRG];

  union {
    uint16_t MATCH_L[CONFIG_SCT_nRG];
    uint16_t CAP_L[CONFIG_SCT_nRG];
  };
  uint16_t RESERVED4[32-CONFIG_SCT_nRG];
  union {
    uint16_t MATCH_H[CONFIG_SCT_nRG];
    uint16_t CAP_H[CONFIG_SCT_nRG];
  };

  uint16_t RESERVED5[32-CONFIG_SCT_nRG];

  union {
    union {
      uint32_t U;
      struct {
	uint16_t L;
	uint16_t H;
      };
    } MATCHREL[CONFIG_SCT_nRG];
    union {
      uint32_t U;
      struct {
	uint16_t L;
	uint16_t H;
      };
    } CAPCTRL[CONFIG_SCT_nRG];
  };

  uint32_t RESERVED6[32-CONFIG_SCT_nRG];

  union {
    uint16_t MATCHREL_L[CONFIG_SCT_nRG];
    uint16_t CAPCTRL_L[CONFIG_SCT_nRG];
  };
  uint16_t RESERVED7[32-CONFIG_SCT_nRG];
  union {
    uint16_t MATCHREL_H[CONFIG_SCT_nRG];
    uint16_t CAPCTRL_H[CONFIG_SCT_nRG];
  };
  uint16_t RESERVED8[32-CONFIG_SCT_nRG];

  struct {
    uint32_t STATE;
    uint32_t CTRL;
  } EVENT[CONFIG_SCT_nEV];

  uint32_t RESERVED9[128-2*CONFIG_SCT_nEV];

  struct {
    uint32_t SET;
    uint32_t CLR;
  } OUT[CONFIG_SCT_nOU];

  uint32_t RESERVED10[191-2*CONFIG_SCT_nOU];

  uint32_t MODULECONTENT;

} LPC_SCT_TypeDef;

/* IO memory map */
#define LPC_FLASH_BASE        (0x00000000UL)
#define LPC_RAM_BASE          (0x10000000UL)
#define LPC_ROM_BASE          (0x1FFF0000UL)
#define LPC_APB0_BASE         (0x40000000UL)
#define LPC_AHB_BASE          (0x50000000UL)

/* APB0 peripherals */
#define LPC_WWDT_BASE         (LPC_APB0_BASE + 0x00000)
#define LPC_MRT_BASE          (LPC_APB0_BASE + 0x04000)
#define LPC_WKT_BASE          (LPC_APB0_BASE + 0x08000)
#define LPC_SWM_BASE          (LPC_APB0_BASE + 0x0C000)
#define LPC_PMU_BASE          (LPC_APB0_BASE + 0x20000)
#define LPC_CMP_BASE          (LPC_APB0_BASE + 0x24000)

#define LPC_FLASHCTRL_BASE    (LPC_APB0_BASE + 0x40000)
#define LPC_IOCON_BASE        (LPC_APB0_BASE + 0x44000)
#define LPC_SYSCON_BASE       (LPC_APB0_BASE + 0x48000)
#define LPC_I2C_BASE          (LPC_APB0_BASE + 0x50000)
#define LPC_SPI0_BASE         (LPC_APB0_BASE + 0x58000)
#define LPC_SPI1_BASE         (LPC_APB0_BASE + 0x5C000)
#define LPC_USART0_BASE       (LPC_APB0_BASE + 0x64000)
#define LPC_USART1_BASE       (LPC_APB0_BASE + 0x68000)
#define LPC_USART2_BASE       (LPC_APB0_BASE + 0x6C000)

/* AHB peripherals */
#define LPC_CRC_BASE          (LPC_AHB_BASE + 0x00000)
#define LPC_SCT_BASE          (LPC_AHB_BASE + 0x04000)

#define LPC_GPIO_PORT_BASE    (0xA0000000)
#define LPC_PIN_INT_BASE      (LPC_GPIO_PORT_BASE  + 0x4000)

#define LPC_WWDT	((volatile LPC_WWDT_TypeDef *) LPC_WWDT_BASE)
#define LPC_MRT         ((volatile LPC_MRT_TypeDef *) LPC_MRT_BASE)
#define LPC_WKT         ((volatile LPC_WKT_TypeDef *) LPC_WKT_BASE)
#define LPC_SWM         ((volatile LPC_SWM_TypeDef *) LPC_SWM_BASE)
#define LPC_PMU         ((volatile LPC_PMU_TypeDef *) LPC_PMU_BASE)
#define LPC_CMP         ((volatile LPC_CMP_TypeDef *) LPC_CMP_BASE)

#define LPC_FLASHCTRL	((volatile LPC_FLASHCTRL_TypeDef *) LPC_FLASHCTRL_BASE)
#define LPC_IOCON       ((volatile LPC_IOCON_TypeDef *) LPC_IOCON_BASE)
#define LPC_SYSCON      ((volatile LPC_SYSCON_TypeDef *) LPC_SYSCON_BASE)
#define LPC_I2C         ((volatile LPC_I2C_TypeDef *) LPC_I2C_BASE)
#define LPC_SPI0        ((volatile LPC_SPI_TypeDef *) LPC_SPI0_BASE)
#define LPC_SPI1        ((volatile LPC_SPI_TypeDef *) LPC_SPI1_BASE)
#define LPC_USART0      ((volatile LPC_USART_TypeDef *) LPC_USART0_BASE)
#define LPC_USART1      ((volatile LPC_USART_TypeDef *) LPC_USART1_BASE)
#define LPC_USART2      ((volatile LPC_USART_TypeDef *) LPC_USART2_BASE)

#define LPC_CRC         ((volatile LPC_CRC_TypeDef *) LPC_CRC_BASE)
#define LPC_SCT         ((volatile LPC_SCT_TypeDef *) LPC_SCT_BASE)

#define LPC_GPIO_PORT   ((volatile LPC_GPIO_PORT_TypeDef *) LPC_GPIO_PORT_BASE)
#define LPC_PIN_INT     ((volatile LPC_PIN_INT_TypeDef *) LPC_PIN_INT_BASE)

/* Common M0+ stuff */

/* NVIC: Nested Vectored Interrupt Controller.  */
typedef struct NVIC {
  uint32_t ISER[1];
  uint32_t unused1[31];
  uint32_t ICER[1];
  uint32_t unused2[31];
  uint32_t ISPR[1];
  uint32_t unused3[31];
  uint32_t ICPR[1];
  uint32_t unused4[31];
  uint32_t unused5[64];
  uint32_t IPR[8];
} NVIC_TypeDef;

#define NVIC		((volatile NVIC_TypeDef * const) 0xE000E100)
#define NVIC_ISER	(NVIC->ISER[0])
#define NVIC_ICER	(NVIC->ICER[0])
#define NVIC_ISPR	(NVIC->ISPR[0])
#define NVIC_ICPR	(NVIC->ICPR[0])
#define NVIC_IPR	(NVIC->IPR[n >> 2])

/* SysTick registers.  */
#define SYST_CSR	((volatile uint32_t *const) 0xE000E010)
#define SYST_RVR	((volatile uint32_t *const) 0xE000E014)
#define SYST_CVR	((volatile uint32_t *const) 0xE000E018)

/* System control register. */
#define SCR		((volatile uint32_t *const) 0xE000ED10)
#define SCR_SLEEPONEXIT	(1<<1)
#define SCR_SLEEPDEEP	(1<<2)
#define SCR_SEVONPEND	(1<<4)
