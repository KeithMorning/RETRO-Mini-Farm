/*
 * Device layer for ELECFREAKS Retro Arcade.
 * Game logic lives in game.c, which is also built for the host simulator.
 */

#include <stdint.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/fsmc.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/pwr.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/scb.h>

#include "game.h"
#include "sound.h"

#define LCD_CMD  (*(volatile uint8_t *)0x60000000)
#define LCD_DATA (*(volatile uint8_t *)0x60000001)

#define PIN_LCD_CS   GPIO12 /* PB12 */
#define PIN_LCD_RST  GPIO7  /* PB7  */
#define PIN_LCD_BL   GPIO9  /* PB9  */

#define BTN_A     GPIO0  /* PC0  */
#define BTN_B     GPIO1  /* PC1  */
#define BTN_LEFT  GPIO15 /* PA15 */
#define BTN_RIGHT GPIO9  /* PC9  */
#define BTN_UP    GPIO5  /* PB5  */
#define BTN_DOWN  GPIO2  /* PB2  */
#define BTN_MENU  GPIO13 /* PC13 */

static uint16_t fb[GAME_W * GAME_H];
static int sys_mhz = 16;

/* --- delay / LCD ------------------------------------------------------- */

static void delay_loop(volatile uint32_t n)
{
    while (n--) {
        __asm__ volatile("nop");
    }
}

static void delay_ms(uint32_t ms)
{
    delay_loop(ms * (sys_mhz >= 80 ? 48000 : 8000));
}

/* --- audio ------------------------------------------------------------- */

static void tone(int hz, int ms)
{
    if (hz < 40) return;
    uint32_t period = 1000000u / (uint32_t)hz;
    if (period < 20) period = 20;
    timer_disable_counter(TIM1);
    timer_set_period(TIM1, period - 1);
    timer_set_oc_value(TIM1, TIM_OC1, period / 3);
    timer_enable_counter(TIM1);
    delay_ms(ms);
    timer_disable_counter(TIM1);
    timer_set_oc_value(TIM1, TIM_OC1, 0);
}

void sound_init(void)
{
    rcc_periph_clock_enable(RCC_TIM1);

    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO8);
    gpio_set_af(GPIOA, GPIO_AF1, GPIO8);
    gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO10);
    gpio_set(GPIOC, GPIO10);

    timer_set_mode(TIM1, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
    timer_set_oc_mode(TIM1, TIM_OC1, TIM_OCM_PWM1);
    timer_enable_oc_output(TIM1, TIM_OC1);
    timer_enable_break_main_output(TIM1);
    timer_set_prescaler(TIM1, (uint16_t)(sys_mhz - 1));
    timer_set_period(TIM1, 1000);
    timer_set_oc_value(TIM1, TIM_OC1, 0);
    timer_enable_counter(TIM1);
}

void sound_play(int id)
{
    switch (id) {
    case SFX_CLICK:   tone(880, 14); break;
    case SFX_PLANT:   tone(440, 24); tone(660, 32); break;
    case SFX_WATER:   tone(700, 20); tone(520, 28); break;
    case SFX_HARVEST: tone(880, 24); tone(1170, 34); break;
    case SFX_COIN:    tone(1180, 22); tone(1560, 30); break;
    case SFX_GIFT:    tone(660, 28); tone(880, 38); break;
    case SFX_DOOR:    tone(300, 28); tone(220, 40); break;
    case SFX_SLEEP:   tone(440, 35); tone(330, 40); tone(220, 60); break;
    case SFX_UPGRADE: tone(523, 35); tone(659, 35); tone(784, 60); break;
    case SFX_CATCH:   tone(660, 25); tone(880, 25); tone(1170, 45); break;
    case SFX_FAIL:    tone(300, 45); tone(180, 70); break;
    case SFX_ERROR:   tone(150, 55); break;
    case SFX_PET:     tone(900, 20); tone(1200, 30); break;
    default: break;
    }
}

/*
 * Try 96 MHz from HSI. If the PLL does not lock, fall back to 16 MHz.
 * Returns the resulting system clock in MHz.
 */
static int clock_init_96(void)
{
    rcc_osc_on(RCC_HSI);
    rcc_wait_for_osc_ready(RCC_HSI);
    rcc_set_sysclk_source(RCC_CFGR_SW_HSI);

    rcc_periph_clock_enable(RCC_PWR);
    pwr_set_vos_scale(PWR_SCALE1);

    rcc_set_hpre(RCC_CFGR_HPRE_DIV_NONE);
    rcc_set_ppre1(RCC_CFGR_PPRE_DIV_2);
    rcc_set_ppre2(RCC_CFGR_PPRE_DIV_NONE);

    rcc_osc_off(RCC_PLL);
    rcc_set_main_pll_hsi(8, 96, 2, 4, 0); /* 16/8 * 96 / 2 = 96 MHz */
    rcc_osc_on(RCC_PLL);

    int ready = 0;
    for (volatile int i = 0; i < 2000000; i++) {
        if (RCC_CR & RCC_CR_PLLRDY) {
            ready = 1;
            break;
        }
    }
    if (!ready) {
        rcc_osc_off(RCC_PLL);
        rcc_set_sysclk_source(RCC_CFGR_SW_HSI);
        return 16;
    }

    flash_dcache_enable();
    flash_icache_enable();
    flash_set_ws(FLASH_ACR_LATENCY_3WS);

    rcc_set_sysclk_source(RCC_CFGR_SW_PLL);
    for (volatile int i = 0; i < 100000; i++) {
        if (rcc_system_clock_source() == RCC_CFGR_SWS_PLL) {
            rcc_ahb_frequency = 96000000;
            rcc_apb1_frequency = 48000000;
            rcc_apb2_frequency = 96000000;
            return 96;
        }
    }

    rcc_set_sysclk_source(RCC_CFGR_SW_HSI);
    return 16;
}

static inline void lcd_cmd(uint8_t c)
{
    LCD_CMD = c;
}

static inline void lcd_data(uint8_t d)
{
    LCD_DATA = d;
}

static inline void lcd_data16(uint16_t v)
{
    LCD_DATA = (uint8_t)(v >> 8);
    LCD_DATA = (uint8_t)v;
}

#define DELAY 0x80

static const uint8_t init_cmds[] = {
    0x28, 0,
    0x20, 0,
    0xCF, 3, 0x00, 0xC1, 0x30,
    0xED, 4, 0x64, 0x03, 0x12, 0x81,
    0xE8, 3, 0x85, 0x00, 0x78,
    0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
    0xF7, 1, 0x20,
    0xEA, 2, 0x00, 0x00,
    0xC0, 1, 0x23,
    0xC1, 1, 0x10,
    0xC5, 2, 0x3E, 0x28,
    0xC7, 1, 0x86,
    0xB6, 3, 0x08, 0x82, 0x27,
    0xF2, 1, 0x00,
    0x26, 1, 0x01,
    0xE0, 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
              0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
    0xE1, 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
              0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
    0x3A, 1, 0x55,
    0x11, DELAY, 120,
    0x29, 0,
    0, 0
};

static void lcd_send_seq(const uint8_t *seq)
{
    while (seq[0]) {
        uint8_t cmd = *seq++;
        uint8_t v = *seq++;
        uint8_t len = v & (uint8_t)~DELAY;

        lcd_cmd(cmd);
        for (uint8_t i = 0; i < len; i++) {
            lcd_data(*seq++);
        }
        if (v & DELAY) {
            delay_ms(*seq++);
        }
    }
}

static void lcd_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    /* Panel is mounted rotated: CASET uses the y range, RASET the x range. */
    lcd_cmd(0x2A);
    lcd_data16(y0);
    lcd_data16(y1);

    lcd_cmd(0x2B);
    lcd_data16(x0);
    lcd_data16(x1);

    lcd_cmd(0x2C);
}

static void lcd_init(void)
{
    rcc_peripheral_enable_clock(&RCC_AHB3ENR, RCC_AHB3ENR_FSMCEN);

    FSMC_BCR1 = FSMC_BCR_EXTMOD | FSMC_BCR_WREN | 0x80;
    /* Slower than the bootloader's timing, safe at both 16 and 96 MHz. */
    FSMC_BWTR1 = 0xff00010 | (8 << 8) | 2;
    FSMC_BCR1 |= FSMC_BCR_MBKEN;

    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2 | GPIO3 | GPIO4 | GPIO5);
    gpio_set_af(GPIOA, GPIO_AF12, GPIO2 | GPIO3 | GPIO4 | GPIO5);

    gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO14);
    gpio_set_af(GPIOB, GPIO_AF10, GPIO14);

    gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6 | GPIO11 | GPIO12);
    gpio_set_af(GPIOC, GPIO_AF10, GPIO6 | GPIO11 | GPIO12);

    gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2 | GPIO3 | GPIO5);
    gpio_set_af(GPIOC, GPIO_AF12, GPIO2 | GPIO3 | GPIO5);

    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
                    PIN_LCD_CS | PIN_LCD_RST | PIN_LCD_BL);
    gpio_clear(GPIOB, PIN_LCD_CS);

    gpio_set(GPIOB, PIN_LCD_RST);
    delay_ms(20);
    gpio_clear(GPIOB, PIN_LCD_RST);
    delay_ms(20);
    gpio_set(GPIOB, PIN_LCD_RST);
    delay_ms(20);

    lcd_send_seq(init_cmds);

    lcd_cmd(0x36);
    lcd_data(0x08);

    lcd_cmd(0xB1);
    lcd_data(0x10);
    lcd_data(0xFF);

    gpio_set(GPIOB, PIN_LCD_BL);
    lcd_set_window(0, 0, GAME_W - 1, GAME_H - 1);
}

static void lcd_blit(void)
{
    lcd_cmd(0x2C);
    for (int x = 0; x < GAME_W; x++) {
        for (int y = 0; y < GAME_H; y++) {
            uint16_t v = fb[x * GAME_H + y];
            LCD_DATA = (uint8_t)(v >> 8);
            LCD_DATA = (uint8_t)v;
        }
    }
}

/* --- buttons ----------------------------------------------------------- */

static void buttons_init(void)
{
    gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, BTN_LEFT);
    gpio_mode_setup(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, BTN_UP | BTN_DOWN);
    gpio_mode_setup(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP,
                    BTN_A | BTN_B | BTN_RIGHT | BTN_MENU);
}

static inline int down(uint32_t port, uint16_t pin)
{
    return gpio_get(port, pin) == 0;
}

static void maybe_reboot(void)
{
    if (down(GPIOC, BTN_MENU) && down(GPIOC, BTN_B)) {
        delay_ms(300);
        scb_reset_system();
    }
}

/* --- main -------------------------------------------------------------- */

int main(void)
{
    sys_mhz = clock_init_96();
    game_clock_mhz = sys_mhz;

    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_GPIOC);

    buttons_init();
    sound_init();
    lcd_init();
    game_init();
    sound_play(SFX_UPGRADE);

    game_render(fb);
    lcd_blit();

    for (;;) {
        maybe_reboot();

        Input in;
        in.left = down(GPIOA, BTN_LEFT);
        in.right = down(GPIOC, BTN_RIGHT);
        in.up = down(GPIOB, BTN_UP);
        in.down = down(GPIOB, BTN_DOWN);
        in.a = down(GPIOC, BTN_A);
        in.b = down(GPIOC, BTN_B);
        in.menu = down(GPIOC, BTN_MENU);

        if (game_update(&in)) {
            game_render(fb);
            lcd_blit();
        } else {
            delay_ms(5);
        }
    }

    return 0;
}
