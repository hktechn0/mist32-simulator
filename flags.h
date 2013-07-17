/* clear flags */
static inline void clr_flags(void)
{
  FLAGR.sign = 0;
  FLAGR.overflow = 0;
  FLAGR.carry = 0;
  FLAGR.parity = 0;
  FLAGR.zero = 0;
}

/* set flags */
static inline void set_flags(int value)
{
  FLAGR.zero = !value;
  FLAGR.parity = !(value & 0x00000001);
  FLAGR.sign = (value < 0);
}

static inline void set_flags_add(unsigned int result, unsigned int dest, unsigned int src)
{
  clr_flags();

  FLAGR.overflow = msb(~(dest ^ src) & (dest ^ result));
  FLAGR.carry = msb((dest & src) | (~result & (dest | src)));

  set_flags(result);

  /*
  printf("r:%d d:%d s:%d c:%d o:%d\n",
	 (int)result, (int)dest, (int)src, FLAGR.carry, FLAGR.overflow);
  */
}

static inline void set_flags_sub(unsigned int result, unsigned int dest, unsigned int src)
{
  set_flags_add(result, dest, (unsigned int)(-((int)src)));

  if(src == 0) {
    FLAGR.carry = 1;
  }
}
