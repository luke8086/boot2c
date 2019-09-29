/*
 * util.h - Misc helpers
 *
 * author: luke8086
 * license: GPL-2
 */

#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdint.h>

typedef __SIZE_TYPE__ size_t;

/* Function attribute moving it to the .start section */
#define ENTRY_POINT __attribute__((section(".start")))

/* Macros for declaring far pointers relative to FS and GS */
#ifdef __SEG_FS /* GCC */
#define FS_RELATIVE __seg_fs
#define GS_RELATIVE __seg_gs
#else /* clang */
#define FS_RELATIVE __attribute__((address_space(257)))
#define GS_RELATIVE __attribute__((address_space(256)))
#endif /* SEG_FS */

/* Set values of the FS / GS registers */
void set_fs(uint16_t);
void set_gs(uint16_t);

/* Standard library functions called implicitly by clang */
void *memcpy(void *, const void *, size_t);

/* Write a null-terminated string at the cursor position */
void put_string(char *);

/* Return a pseudo-random number between 0 - 65534 */
uint16_t rand(void);

/* Breakpoint trigger for Bochs */
#define BOCHS_BREAKPOINT  __asm__ volatile ("xchg %bx, %bx")

#endif /* _UTIL_H_ */
