/* clear flags */
static inline void clr_flags(void)
{
  FLAGR.flags = 0;
}

/* set flags */
static inline void set_flags(const uint32_t value)
{
  FLAGR.zero = !value;
  FLAGR.parity = ~(value & 1);
  FLAGR.sign = msb(value);
}

static inline void set_flags_add(const uint32_t result, const uint32_t dest, const uint32_t src)
{
  clr_flags();

  FLAGR.carry = ((uint64_t)dest + (uint64_t)src) >> 32;
  //FLAGR.carry = msb((dest & src) | (~result & (dest | src)));
  FLAGR.overflow = msb(~(dest ^ src) & (dest ^ result));

  set_flags(result);
}

static inline void set_flags_sub(const uint32_t result, const uint32_t dest, const uint32_t src)
{
  set_flags_add(result, dest, (uint32_t)(-((int32_t)src)));
  FLAGR.carry |= !src;
}
