#ifndef MIST32_FLAGS_H
#define MIST32_FLAGS_H

/* make generic FLAGS */
static inline FLAGS make_flags(const uint32_t value)
{
  FLAGS f = { .flags = 0 };

  f.zero = !value;
  f.parity = ~(value & 1);
  f.sign = msb(value);

  return f;
}

/* make FLAGS for add */
static inline FLAGS make_flags_add(const uint32_t result, const uint32_t dest, const uint32_t src)
{
  FLAGS f;

  f = make_flags(result);
  f.overflow = msb((dest ^ result) & (src ^ result));
  f.carry = ((uint64_t)dest + (uint64_t)src) >> 32;

  return f;
}

/* make FLAGS for sub */
static inline FLAGS make_flags_sub(const uint32_t result, const uint32_t dest, const uint32_t src)
{
  FLAGS f;
  uint32_t neg_src;

  neg_src = -((int32_t)src);

  f = make_flags(result);
  f.overflow = msb((dest ^ result) & (dest ^ src));
  f.carry = (!src) | (((uint64_t)dest + (uint64_t)neg_src) >> 32);

  return f;
}

#endif /* MIST32_FLAGS_H */
