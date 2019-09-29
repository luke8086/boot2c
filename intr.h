/*
 * intr.h - A generic function for triggering software interrupts
 *
 * author: luke8086
 * license: GPL-2
 */

#ifndef _INTR_H_
#define _INTR_H_

#include <stdint.h>

/* Container for values of the CPU registers */
struct regs {
    union {
        struct {
            int32_t eax, ebx, ecx, edx;
            uint32_t ebp, edi, esi;
        };

        struct {
            int16_t ax, _eax_fill;
            int16_t bx, _ebx_fill;
            int16_t cx, _ecx_fill;
            int16_t dx, _edx_fill;
            uint16_t bp, _ebp_fill;
            uint16_t di, _edi_fill;
            uint16_t si, _esi_fill;
        };

        struct {
            int8_t al, ah; int16_t _eax_fill_;
            int8_t bl, bh; int16_t _ebx_fill_;
            int8_t cl, ch; int16_t _ecx_fill_;
            int8_t dl, dh; int16_t _edx_fill_;
        };
    };

    uint32_t eflags;
};

/* Trigger software interrupt */
void intr(int int_no, struct regs *);

#endif /* _INTR_H_ */
