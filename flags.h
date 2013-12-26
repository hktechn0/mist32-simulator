/* clear flags */
static inline void clr_flags(void)
{
  FLAGR.flags = 0;
}

/* set flags */
static inline void set_flags(int32_t value)
{
  FLAGR.zero = !value;
  FLAGR.parity = !(value & 0x00000001);
  FLAGR.sign = (value < 0);
}

static inline void set_flags_add(uint32_t result, uint32_t dest, uint32_t src)
{
  clr_flags();

  FLAGR.overflow = msb(~(dest ^ src) & (dest ^ result));
  FLAGR.carry = msb((dest & src) | (~result & (dest | src)));

  set_flags(result);
}

static inline void set_flags_sub(uint32_t result, uint32_t dest, uint32_t src)
{
  set_flags_add(result, dest, (uint32_t)(-((int32_t)src)));

  if(src == 0) {
    FLAGR.carry = 1;
  }
}
