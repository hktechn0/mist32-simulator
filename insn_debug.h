#ifndef MIST32_INSN_DEBUG_H
#define MIST32_INSN_DEBUG_H

static inline void debug_load_hw(Memory addr, unsigned int data)
{
  DEBUGLDHW("[L], %08x, %08x, %08x, %08x\n", PCR, SPR, addr, data);
  DEBUGLDPHY("[L], %08x(%08x), %08x(%08x), %08x(%08x), %08x\n",
	     PCR, memory_addr_virt2phy(PCR, false, false),
	     SPR, memory_addr_virt2phy(SPR, false, false),
	     addr, memory_addr_virt2phy(addr, false, false), data);
}

static inline void debug_store_hw(Memory addr, unsigned int data)
{
  DEBUGSTHW("[S], %08x, %08x, %08x, %08x\n", PCR, SPR, addr, data);
  DEBUGSTPHY("[S], %08x(%08x), %08x(%08x), %08x(%08x), %08x\n",
	     PCR, memory_addr_virt2phy(PCR, false, false),
	     SPR, memory_addr_virt2phy(SPR, false, false),
	     addr, memory_addr_virt2phy(addr, false, false), data);
}

static inline void debug_load8(Memory addr, unsigned char data)
{
  DEBUGLD("[Load ] Addr: 0x%08x, Data:       0x%02x, PC: 0x%08x\n", addr, data, PCR);
  debug_load_hw(addr, data);
}

static inline void debug_load16(Memory addr, unsigned short data)
{
  DEBUGLD("[Load ] Addr: 0x%08x, Data:     0x%04x, PC: 0x%08x\n", addr, data, PCR);
  debug_load_hw(addr, data);
}

static inline void debug_load32(Memory addr, unsigned int data)
{
  DEBUGLD("[Load ] Addr: 0x%08x, Data: 0x%08x, PC: 0x%08x\n", addr, data, PCR);
  debug_load_hw(addr, data);
}

static inline void debug_store8(Memory addr, unsigned char data)
{
  DEBUGST("[Store] Addr: 0x%08x, Data:       0x%02x, PC: 0x%08x\n", addr, data, PCR);
  debug_store_hw(addr, data);
}

static inline void debug_store16(Memory addr, unsigned short data)
{
  DEBUGST("[Store] Addr: 0x%08x, Data:     0x%04x, PC: 0x%08x\n", addr, data, PCR);
  debug_store_hw(addr, data);
}

static inline void debug_store32(Memory addr, unsigned int data)
{
  DEBUGST("[Store] Addr: 0x%08x, Data: 0x%08x, PC: 0x%08x\n", addr, data, PCR);
  debug_store_hw(addr, data);
}

#endif /* MIST32_INSN_DEBUG_H */
