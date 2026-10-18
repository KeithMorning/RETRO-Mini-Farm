#include <stdint.h>

extern uint32_t _estack;
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

void reset_handler(void);
int main(void);

void default_handler(void)
{
    while (1) {
    }
}

__attribute__((section(".vectors"), used))
void (*const vector_table[114])(void) = {
    [0] = (void (*)(void))((uint32_t)&_estack),
    [1] = reset_handler,
};

void reset_handler(void)
{
    __asm__ volatile("cpsid i");

    /* Enable the FPU (CP10/CP11 full access). */
    *(volatile uint32_t *)0xE000ED88 |= (0xF << 20);
    __asm__ volatile("dsb");
    __asm__ volatile("isb");

    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    while (dst < &_edata) {
        *dst++ = *src++;
    }

    dst = &_sbss;
    while (dst < &_ebss) {
        *dst++ = 0;
    }

    main();

    while (1) {
    }
}
