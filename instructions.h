#ifndef MIST32_INSTRUCTIONS_H
#define MIST32_INSTRUCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <err.h>

#include "common.h"
#include "debug.h"
#include "registers.h"
#include "load_store.h"
#include "tlb.h"
#include "fetch.h"
#include "interrupt.h"
#include "insn_format.h"

#include "flags.h"
#include "operands.h"

/* Arithmetic */
void i_add(const Instruction insn)
{
  int32_t *destptr, dest, src;

  DECODE_O2_I11(insn, destptr, dest, src);
  *destptr = dest + src;

  FLAGR = make_flags_add(*destptr, dest, src);
}

void i_sub(const Instruction insn)
{
  int32_t *destptr, dest, src;

  DECODE_O2_I11(insn, destptr, dest, src);
  *destptr = dest - src;

  FLAGR = make_flags_sub(*destptr, dest, src);
}

void i_mull(const Instruction insn)
{
  int32_t *destptr, dest, src;

  DECODE_O2_I11(insn, destptr, dest, src);
  *destptr = dest * src;

  /* FIXME: flags unavailable */
  FLAGR.flags = 0;
}

void i_mulh(const Instruction insn)
{ 
  int32_t *destptr, dest, src;
  int64_t result;

  DECODE_O2_I11(insn, destptr, dest, src);
  result = (int64_t)dest * (int64_t)src;
  *destptr = (uint64_t)result >> 32;

  /* FIXME: flags unavailable */
  FLAGR.flags = 0;
}

void i_umulh(const Instruction insn)
{
  uint32_t *destptr, dest, src;
  uint64_t result;

  DECODE_O2_UI11(insn, destptr, dest, src);
  result = (uint64_t)dest * (uint64_t)src;
  *destptr = result >> 32;

  /* FIXME: flags unavailable */
  FLAGR.flags = 0;
}

void i_udiv(const Instruction insn)
{
  uint32_t *destptr, dest, src;

  DECODE_O2_UI11(insn, destptr, dest, src);

  if(src == 0) {
    DEBUGINT("[INTERRUPT] Zero division error. PC: %08x\n", PCR);
    interrupt_dispatch_nonmask(IDT_DIVERROR_NUM);
    return;
  }

  *destptr = dest / src;

  /* FIXME: flags unavailable */
  FLAGR.flags = 0;
}

void i_umod(const Instruction insn)
{
  uint32_t *destptr, dest, src;

  DECODE_O2_UI11(insn, destptr, dest, src);

  if(src == 0) {
    DEBUGINT("[INTERRUPT] Zero division error. PC: %08x\n", PCR);
    interrupt_dispatch_nonmask(IDT_DIVERROR_NUM);
    return;
  }

  *destptr = dest % src;

  /* FIXME: flags unavailable */
  FLAGR.flags = 0;
}

void i_cmp(const Instruction insn)
{
  int32_t dest, src;

  dest = dest_o2_i11(insn);
  src = src_o2_i11(insn);
  FLAGR = make_flags_sub(dest - src, dest, src);
}

void i_div(const Instruction insn)
{
  int32_t *destptr, dest, src;

  DECODE_O2_I11(insn, destptr, dest, src);

  if(src == 0) {
    DEBUGINT("[INTERRUPT] Zero division error. PC: %08x\n", PCR);
    interrupt_dispatch_nonmask(IDT_DIVERROR_NUM);
    return;
  }

  *destptr = dest / src;

  /* FIXME: flags unavailable */
  FLAGR.flags = 0;
}

void i_mod(const Instruction insn)
{
  int32_t *destptr, dest, src;

  DECODE_O2_I11(insn, destptr, dest, src);

  if(src == 0) {
    DEBUGINT("[INTERRUPT] Zero division error. PC: %08x\n", PCR);
    interrupt_dispatch_nonmask(IDT_DIVERROR_NUM);
    return;
  }

  *destptr = dest % src;

  /* FIXME: flags unavailable */
  FLAGR.flags = 0;
}

void i_neg(const Instruction insn)
{
  GR[insn.o2.operand1] = -GR[insn.o2.operand2];

  FLAGR = make_flags(GR[insn.o2.operand1]);
  // if result == 0x80000000 then overflow
  FLAGR.overflow = ((uint32_t)GR[insn.o2.operand1] >> 31) & 1;
}

void i_addc(const Instruction insn)
{
  int32_t *destptr, dest, src, result;

  DECODE_O2_I11(insn, destptr, dest, src);
  result = dest + src;

  FLAGR = make_flags_add(result, dest, src);
  *destptr = FLAGR.carry;
}

void i_inc(const Instruction insn)
{
  GR[insn.o2.operand1] = GR[insn.o2.operand2] + 1;
  FLAGR = make_flags_add(GR[insn.o2.operand1], GR[insn.o2.operand2], 1);
}

void i_dec(const Instruction insn)
{
  GR[insn.o2.operand1] = GR[insn.o2.operand2] - 1;
  FLAGR = make_flags_sub(GR[insn.o2.operand1], GR[insn.o2.operand2], 1);
}

void i_max(const Instruction insn)
{
  int32_t *destptr, dest, src;

  DECODE_O2_I11(insn, destptr, dest, src);
  *destptr = MAX(dest, src);
}

void i_min(const Instruction insn)
{
  int32_t *destptr, dest, src;

  DECODE_O2_I11(insn, destptr, dest, src);
  *destptr = MIN(dest, src);
}

void i_umax(const Instruction insn)
{
  uint32_t *destptr, dest, src;

  DECODE_O2_UI11(insn, destptr, dest, src);
  *destptr = MAX(dest, src);
}

void i_umin(const Instruction insn)
{
  uint32_t *destptr, dest, src;

  DECODE_O2_UI11(insn, destptr, dest, src);
  *destptr = MIN(dest, src);
}

static inline void i_sext8(const Instruction insn)
{
  GR[insn.o2.operand1] = SIGN_EXT8((uint32_t)GR[insn.o2.operand2]);
}

static inline void i_sext16(const Instruction insn)
{
  GR[insn.o2.operand1] = SIGN_EXT16((uint32_t)GR[insn.o2.operand2]);
}

/* Shift, Rotate */
void i_shl(const Instruction insn)
{
  uint32_t *destptr, dest, n;

  DECODE_O2_UI11(insn, destptr, dest, n);
  *destptr= dest << n;

  FLAGR = make_flags(*destptr);
  FLAGR.carry = dest >> (32 - n);
}

void i_shr(const Instruction insn)
{
  uint32_t *destptr, dest, n;

  DECODE_O2_UI11(insn, destptr, dest, n);
  *destptr = dest >> n;

  FLAGR = make_flags(*destptr);
  FLAGR.carry = (dest >> (n - 1)) & 0x00000001;
}

void i_sar(const Instruction insn)
{
  int32_t *destptr, dest;
  uint32_t n;

  DECODE_O2_I11(insn, destptr, dest, n);
  *destptr = dest >> n;

  FLAGR = make_flags(*destptr);
  FLAGR.carry = (dest >> (n - 1)) & 0x00000001;
}

void i_rol(const Instruction insn)
{
  uint32_t *destptr, dest, n;

  DECODE_O2_UI11(insn, destptr, dest, n);
  *destptr = (dest << n) | (dest >> (32 - n));

  FLAGR = make_flags(*destptr);
  FLAGR.carry = *destptr & 0x00000001;
}

void i_ror(const Instruction insn)
{
  uint32_t *destptr, dest, n;

  DECODE_O2_UI11(insn, destptr, dest, n);
  *destptr = (dest >> n) | (dest << (32 - n));
  
  FLAGR = make_flags(*destptr);
  FLAGR.carry = (*destptr & 0x80000000) >> 31;
}

/* Logic */
void i_and(const Instruction insn)
{
  GR[insn.o2.operand1] &= GR[insn.o2.operand2];
  FLAGR = make_flags(GR[insn.o2.operand1]);
}

void i_or(const Instruction insn)
{
  GR[insn.o2.operand1] |= GR[insn.o2.operand2];
  FLAGR = make_flags(GR[insn.o2.operand1]);
}

static inline void i_not(const Instruction insn)
{
  GR[insn.o2.operand1] = ~GR[insn.o2.operand2];
}

void i_xor(const Instruction insn)
{
  GR[insn.o2.operand1] ^= GR[insn.o2.operand2];
  FLAGR = make_flags(GR[insn.o2.operand1]);
}

void i_nand(const Instruction insn)
{
  GR[insn.o2.operand1] = ~(GR[insn.o2.operand1] & GR[insn.o2.operand2]);
  FLAGR = make_flags(GR[insn.o2.operand1]);
}

void i_nor(const Instruction insn)
{
  GR[insn.o2.operand1] = ~(GR[insn.o2.operand1] | GR[insn.o2.operand2]);
  FLAGR = make_flags(GR[insn.o2.operand1]);
}

void i_xnor(const Instruction insn)
{
  GR[insn.o2.operand1] = ~(GR[insn.o2.operand1] ^ GR[insn.o2.operand2]);
  FLAGR = make_flags(GR[insn.o2.operand1]);
}

void i_test(const Instruction insn)
{
  uint32_t result;

  result = GR[insn.o2.operand1] & GR[insn.o2.operand2];
  FLAGR = make_flags(result);
}

/* Register operations */
static inline void i_wl16(const Instruction insn)
{
  GR[insn.i16.operand] = ((uint32_t)GR[insn.i16.operand] & 0xffff0000) | (immediate_i16(insn) & 0xffff);
}

static inline void i_wh16(const Instruction insn)
{
  GR[insn.i16.operand] = ((uint32_t)GR[insn.i16.operand] & 0xffff) | ((immediate_i16(insn) & 0xffff) << 16);
}

static inline void i_clrb(const Instruction insn)
{
  GR[insn.i11.operand] &= ~((uint32_t)0x01 << immediate_i11(insn));
}

static inline void i_setb(const Instruction insn)
{
  GR[insn.i11.operand] |= (uint32_t)0x01 << immediate_i11(insn);
}

static inline void i_clr(const Instruction insn)
{
  GR[insn.o1.operand1] = 0x00000000;
}

static inline void i_set(const Instruction insn)
{
  GR[insn.o1.operand1] = (uint32_t)0xffffffff;
}

void i_revb(const Instruction insn)
{
  /* FIXME: not implement */
  fprintf(stderr, "[Error] %s not implemented yet.\n", "revb");
  exit(EXIT_FAILURE);
}

static inline void i_rev8(const Instruction insn)
{
  GR[insn.o2.operand1] = __builtin_bswap32(GR[insn.o2.operand2]);
}

void i_getb(const Instruction insn)
{
  /* FIXME: not implement */
  fprintf(stderr, "[Error] %s not implemented yet.\n", "getb");
  exit(EXIT_FAILURE);
}

static inline void i_get8(const Instruction insn)
{
  uint32_t *destptr, dest, src;

  DECODE_O2_UI11(insn, destptr, dest, src);
  *destptr = (dest >> (src * 8)) & 0xff;
}

static inline void i_lil(const Instruction insn)
{
  GR[insn.i16.operand] = immediate_i16(insn);
}

static inline void i_lih(const Instruction insn)
{
  GR[insn.i16.operand] = (uint32_t)immediate_ui16(insn) << 16;
}

static inline void i_ulil(const Instruction insn)
{
  GR[insn.i16.operand] = immediate_ui16(insn);
}

/* Load, Store */
void i_ld8(const Instruction insn)
{
  uint32_t *dest, src;

  dest = (uint32_t *)dest_o2_i11_ptr(insn);
  src = src_o2_ui11(insn);

  if(!insn.i11.is_immediate) {
    src += (int)SIGN_EXT6(insn.o2.displacement);
  }

  if(memory_ld8(dest, src)) {
    /* memory fault */
    return;
  }

  if(DEBUG_MEM) debug_load16(src, (unsigned char)*dest);
}

void i_ld16(const Instruction insn)
{
  uint32_t *dest, src;

  dest = (uint32_t *)dest_o2_i11_ptr(insn);
  src = src_o2_ui11(insn);

  if(insn.i11.is_immediate) {
    src <<= 1;
  }
#if !NO_DEBUG
  else if(src & 0x1) {
    abort_sim();
    errx(EXIT_FAILURE, "ld16: invalid alignment.");
  }
#endif
  else {
    src += (int)(SIGN_EXT6(insn.o2.displacement) << 1);
  }

  if(memory_ld16(dest, src)) {
    /* memory fault */
    return;
  }

  if(DEBUG_MEM) debug_load16(src, (unsigned short)*dest);
}

void i_ld32(const Instruction insn)
{
  uint32_t *dest, src;

  dest = (uint32_t *)dest_o2_i11_ptr(insn);
  src = src_o2_ui11(insn);

  if(insn.i11.is_immediate) {
    src <<= 2;
  }
#if !NO_DEBUG
  else if(src & 0x3) {
    abort_sim();
    errx(EXIT_FAILURE, "ld32: invalid alignment.");
  }
#endif
  else {
    src += (int)(SIGN_EXT6(insn.o2.displacement) << 2);
  }

  if(memory_ld32(dest, src)) {
    /* memory fault */
    return;
  }

  if(DEBUG_MEM) debug_load32(src, *dest);
}

void i_st8(const Instruction insn)
{
  uint32_t *dest, src;

  dest = (uint32_t *)dest_o2_i11_ptr(insn);
  src = src_o2_ui11(insn);

  if(!insn.i11.is_immediate) {
    src += (int)SIGN_EXT6(insn.o2.displacement);
  }

  if(memory_st8(src, (unsigned char)*dest)) {
    /* memory fault */
    return;
  }

  if(DEBUG_MEM) debug_store8(src, (unsigned char)*dest);
}

void i_st16(const Instruction insn)
{
  uint32_t *dest, src;

  dest = (uint32_t *)dest_o2_i11_ptr(insn);
  src = src_o2_ui11(insn);

  if(insn.i11.is_immediate) {
    src <<= 1;
  }
#if !NO_DEBUG
  else if(src & 0x1) {
    abort_sim();
    errx(EXIT_FAILURE, "st16: invalid alignment.");
  }
#endif
  else {
    src += (int)(SIGN_EXT6(insn.o2.displacement) << 1);
  }

  if(memory_st16(src, (unsigned short)*dest)) {
    /* memory fault */
    return;
  }

  if(DEBUG_MEM) debug_store16(src, (unsigned short)*dest);
}

void i_st32(const Instruction insn)
{
  uint32_t *dest, src;

  dest = (uint32_t *)dest_o2_i11_ptr(insn);
  src = src_o2_ui11(insn);

  if(insn.i11.is_immediate) {
    src <<= 2;
  }
#if !NO_DEBUG
  else if(src & 0x3) {
    abort_sim();
    errx(EXIT_FAILURE, "st32: invalid alignment.");
  }
#endif
  else {
    src += (int)(SIGN_EXT6(insn.o2.displacement) << 2);
  }

  if(memory_st32(src, *dest)) {
    /* memory fault */
    return;
  }

  if(DEBUG_MEM) debug_store32(src, *dest);
}

/* Stack */
void i_push(const Instruction insn)
{
  uint32_t src;

  SPR -= 4;
  src = insn.c.is_immediate ? insn.c.immediate : GR[insn.o1.operand1];

  if(memory_st32(SPR, src)) {
    /* memory fault */
    return;
  }

  if(DEBUG_MEM) debug_push(SPR, src);
}

void i_pushpc(const Instruction insn)
{
  SPR -= 4;

  if(memory_st32(SPR, PCR)) {
    /* memory fault */
    return;
  }
}

void i_pop(const Instruction insn)
{
  uint32_t *dest;

  dest = (uint32_t *)&(GR[insn.o1.operand1]);

  if(memory_ld32(dest, SPR)) {
    /* memory fault */
    return;
  }

  SPR += 4;

  if(DEBUG_MEM) debug_pop(SPR - 4, *dest);
}

/* Branch */
void i_bur(const Instruction insn)
{
  if(check_condition(insn)) {
    DEBUGJMP("[Branch] URE: 0x%08x, Cond: %X,          PC: 0x%08x\n", PCR + src_jo1_jui16(insn), insn.ji16.condition, PCR);
    next_PCR = PCR + src_jo1_jui16(insn);
  }
}

void i_br(const Instruction insn)
{
  if(check_condition(insn)) {
    DEBUGJMP("[Branch] REL: 0x%08x, Cond: %X,          PC: 0x%08x\n", PCR + src_jo1_ji16(insn), insn.ji16.condition, PCR);
    next_PCR = PCR + src_jo1_ji16(insn);
  }
}

void i_b(const Instruction insn)
{
  if(check_condition(insn)) {
    DEBUGJMP("[Branch]  D : 0x%08x, Cond: %X,          PC: 0x%08x\n", src_jo1_jui16(insn), insn.ji16.condition, PCR);
    next_PCR = src_jo1_jui16(insn);
  }

  /* instruction_prefetch_flush(); */

#if !NO_DEBUG
  /* for traceback */
  if(!insn.jo1.is_immediate && insn.jo1.operand1 == GR_RET) {
    if(traceback > 0) {
      traceback_next--;
    }
  }
#endif
}

void i_ib(const Instruction insn)
{
  interrupt_exit();
}

static inline void i_srspr(const Instruction insn)
{
  GR[insn.o1.operand1] = SPR;
}

static inline void i_srpdtr(const Instruction insn)
{
  GR[insn.o1.operand1] = PDTR;
}

static inline void i_srkpdtr(const Instruction insn)
{
  GR[insn.o1.operand1] = KPDTR;
}

void i_srpidr(const Instruction insn)
{
  /* FIXME: not implemented */
  GR[insn.o1.operand1] = 0;
}

void i_srcidr(const Instruction insn)
{
  /* FIXME: not implemented */
  GR[insn.o1.operand1] = 0;
}

void i_srmoder(const Instruction insn)
{
  /* FIXME: not implemented */
  GR[insn.o1.operand1] = 0;
}

static inline void i_srieir(const Instruction insn)
{
  GR[insn.o1.operand1] = (PSR & PSR_IM_ENABLE) >> 2;
}

static inline void i_srmmur(const Instruction insn)
{
  GR[insn.o1.operand1] = (PSR & PSR_MMUMOD_MASK);
}

static inline void i_sriosr(const Instruction insn)
{
  GR[insn.o1.operand1] = IOSR;
}

static inline void i_srtidr(const Instruction insn)
{
  GR[insn.o1.operand1] = TIDR;
}

static inline void i_srppsr(const Instruction insn)
{
  GR[insn.o1.operand1] = PPSR;
}

static inline void i_srppcr(const Instruction insn)
{
  GR[insn.o1.operand1] = PPCR;
}

static inline void i_sruspr(const Instruction insn)
{
  GR[insn.o1.operand1] = USPR;
}

static inline void i_srppdtr(const Instruction insn)
{
  GR[insn.o1.operand1] = PPDTR;
}

static inline void i_srptidr(const Instruction insn)
{
  GR[insn.o1.operand1] = PTIDR;
}

static inline void i_srpsr(const Instruction insn)
{
  GR[insn.o1.operand1] = PSR;
}

static inline void i_srfrcr(const Instruction insn)
{
  FRCR = (unsigned long long)clock() * (HARDWARE_CLOCK_HZ / CLOCKS_PER_SEC);
}

static inline void i_srfrclr(const Instruction insn)
{
  GR[insn.o1.operand1] = (uint32_t)(FRCR & 0xffffffff);
}

static inline void i_srfrchr(const Instruction insn)
{
  GR[insn.o1.operand1] = (uint32_t)(FRCR >> 32);
}

static inline void i_srpflagr(const Instruction insn)
{
  GR[insn.o1.operand1] = PFLAGR.flags;
}

static inline void i_srfi0r(const Instruction insn)
{
  GR[insn.o1.operand1] = FI0R;
}

static inline void i_srfi1r(const Instruction insn)
{
  GR[insn.o1.operand1] = FI1R;
}

static inline void i_srspw(const Instruction insn)
{
  SPR = GR[insn.o1.operand1];
}

void i_srpdtw(const Instruction insn)
{
  PDTR = GR[insn.o1.operand1];
  DEBUGMMU("[MMU] SRPDTW: 0x%08x\n", PDTR);

  memory_tlb_flush();
  instruction_prefetch_flush();
}

void i_srkpdtw(const Instruction insn)
{
  KPDTR = GR[insn.o1.operand1];
  DEBUGMMU("[MMU] SRKPDTW: 0x%08x\n", KPDTR);

  memory_tlb_flush();
  instruction_prefetch_flush();
}

void i_srieiw(const Instruction insn)
{
  if(src_o1_i11(insn) & 1) {
    PSR |= PSR_IM_ENABLE;

    if(!(PSR & PSR_IM_ENABLE)) {
      DEBUGINT("[INTERRUPT] SRIEIW: Interrupt Enabled.\n");
    }
  }
  else {
    PSR &= ~PSR_IM_ENABLE;

    if(PSR & PSR_IM_ENABLE) {
      DEBUGINT("[INTERRUPT] SRIEIW: Interrupt Disabled.\n");
    }
  }
}

void i_srmmuw(const Instruction insn)
{
  PSR = (PSR & ~PSR_MMUMOD_MASK) | (src_o1_i11(insn) & PSR_MMUMOD_MASK);
  DEBUGMMU("[MMU] SRMMUW: MMUMOD %d\n", PSR & PSR_MMUMOD_MASK);

  memory_tlb_flush();
  instruction_prefetch_flush();
}

static inline void i_srppsw(const Instruction insn)
{
  PPSR = (Memory)GR[insn.o1.operand1];
}

void i_srppcw(const Instruction insn)
{
  PPCR = (Memory)GR[insn.o1.operand1];

  if(PPCR & 0x3) {
    errx(EXIT_FAILURE, "srppcw: invalid alignment. %08x PC: %08x", PPCR, PCR);
  }
}

static inline void i_sruspw(const Instruction insn)
{
  USPR = (Memory)GR[insn.o1.operand1];
}

static inline void i_srppdtw(const Instruction insn)
{
  PPDTR = (Memory)GR[insn.o1.operand1];
}

static inline void i_srptidw(const Instruction insn)
{
  PTIDR = (Memory)GR[insn.o1.operand1];
}

void i_sridtw(const Instruction insn)
{
  IDTR = (Memory)GR[insn.o1.operand1];
  DEBUGINT("[INTERRUPT] SRIDTW: idtr <= 0x%08x\n", IDTR);
}

void i_srpsw(const Instruction insn)
{
  if(((GR[insn.o1.operand1] & PSR_MMUMOD_MASK) != (PSR & PSR_MMUMOD_MASK)) ||
     ((GR[insn.o1.operand1] & PSR_CMOD_MASK) != (PSR & PSR_CMOD_MASK))) {
    memory_tlb_flush();
    instruction_prefetch_flush();
  }

  PSR = GR[insn.o1.operand1];
  DEBUGMMU("[MMU] SRPSW: MMUMOD %d MMUPS %d\n", PSR_MMUMOD, PSR_MMUPS);

  if(PSR_MMUMOD && PSR_MMUPS != PSR_MMUPS_4KB) {
    abort_sim();
    errx(EXIT_FAILURE, "MMU page size (%d) not supported.", PSR_MMUPS);
  }
}

static inline void i_srpflagw(const Instruction insn)
{
  PFLAGR.flags = GR[insn.o1.operand1];
}

static inline void i_srspadd(const Instruction insn)
{
  SPR += (int)SIGN_EXT16(insn.c.immediate) << 2;
}

static inline void i_nop(const Instruction insn)
{
  // NOTHING TO DO
}

void i_halt(const Instruction insn)
{
  exit(EXIT_FAILURE);
}

static inline void i_move(const Instruction insn)
{
  GR[insn.o2.operand1] = GR[insn.o2.operand2];
}

void i_movepc(const Instruction insn)
{
  int32_t src;

  src = src_o2_ui11(insn);
  GR[insn.o2.operand1] = PCR + (src << 2);

  /* for traceback */
#if !NO_DEBUG
  if(insn.o2.operand1 == GR_RET) {
    if(traceback_next < TRACEBACK_MAX) {
      traceback[traceback_next++] = PCR + 4;
    }
  }
#endif
}

void i_swi(const Instruction insn)
{
  unsigned int num;

  num = immediate_ui11(insn);

  if(num < IDT_SWIRQ_START_NUM) {
    /* software irq number should be 64 or above */
    errx(EXIT_FAILURE, "swi: invalid swi (%d)", num);
  }

  interrupt_dispatch_nonmask(num);
}

void i_tas(const Instruction insn)
{
  uint32_t *dest, src;

  dest = (uint32_t *)dest_o2_i11_ptr(insn);
  src = src_o2_ui11(insn);

  if(insn.i11.is_immediate) {
    src <<= 2;
  }
  else if(src & 0x3) {
    errx(EXIT_FAILURE, "tas: invalid alignment.");
  }

  /* load flag */
  if(memory_ld32(dest, src)) {
    /* memory fault */
    return;
  }

  /* test */
  if(*dest == 0) {
    /* success, and set */
    if(memory_st32(src, 1)) {
      /* memory fault */
      return;
    }
  }
  else {
    /* fail, do nothing */
  }

  DEBUGST("[TAS] Addr: 0x%08x, %s, PC: 0x%08x\n", src, *dest ? "fail" : "success", PCR);
  /* DEBUGSTHW("[S], %08x, %08x, %08x, %08x\n", PCR, SPR, src, *dest); */
}

void i_idts(const Instruction insn)
{
  interrupt_idt_store();
}

void i_invalid(const Instruction insn)
{
  print_instruction(insn);
  abort_sim();
  errx(EXIT_FAILURE, "invalid opcode. (pc:%08x op:%x)", PCR, insn.base.opcode);
}

#endif /* MIST32_INSTRUCTIONS_H */
