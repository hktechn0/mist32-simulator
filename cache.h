/* L1 Cache */
#define CACHE_L1_I_ENABLE 1
#define CACHE_L1_D_ENABLE 1
#define CACHE_L1_PROFILE 1

#define CACHE_L1_WAY 4
#define CACHE_L1_LINE_PER_WAY 16
#define CACHE_L1_LINE_SIZE 16 /* number of word */
#define CACHE_L1_LINE_MASK 0xffffffc0
#define CACHE_L1_TAG(addr) (addr & 0xfffffc00)
#define CACHE_L1_INDEX(addr) ((addr >> 6) & 0xf)
#define CACHE_L1_WORD(addr) ((addr >> 2) & 0xf)

typedef struct _cachelinel1 {
  bool valid;
  unsigned int miss;
  uint32_t tag;
  uint32_t line[CACHE_L1_LINE_SIZE];
} CacheLineL1;

extern CacheLineL1 cache_l1i[CACHE_L1_WAY][CACHE_L1_LINE_PER_WAY];
extern CacheLineL1 cache_l1d[CACHE_L1_WAY][CACHE_L1_LINE_PER_WAY];
extern unsigned long long cache_l1i_total, cache_l1i_hit;
extern unsigned long long cache_l1d_total, cache_l1d_hit;

#if CACHE_L1_I_ENABLE || CACHE_L1_D_ENABLE

static inline uint32_t memory_cache_l1_read(Memory paddr, int is_icache)
{
  CacheLineL1 (*cache)[CACHE_L1_LINE_PER_WAY];
  unsigned int w, i;
  unsigned int tag, index, word;
  unsigned int miss, maxmiss, target;

  if(paddr >= MEMORY_MAX_ADDR) {
    /* non-cache area */
    return *(uint32_t *)memory_addr_phy2vm(paddr, false);
  }

  if(is_icache) {
    cache = cache_l1i;
  }
  else {
    cache = cache_l1d;
  }

  tag = CACHE_L1_TAG(paddr);
  index = CACHE_L1_INDEX(paddr);
  word = CACHE_L1_WORD(paddr);

  for(w = 0; w < CACHE_L1_WAY; w++) {
    if(cache[w][index].tag == tag && cache[w][index].valid) {
      /* hit */
      for(i = 0; i < CACHE_L1_WAY; i++) {
	/* LRU */
	cache[i][index].miss++;
      }
      cache[w][index].miss = 0;

#if CACHE_L1_PROFILE
      if(is_icache) {
	cache_l1i_total++;
	cache_l1i_hit++;
      }
      else {
	cache_l1d_total++;
	cache_l1d_hit++;
      }
#endif

      return cache[w][index].line[word];
    }
  }

  /* miss */
  maxmiss = 0;
  target = 0;

#if CACHE_L1_PROFILE
      if(is_icache) {
	cache_l1i_total++;
      }
      else {
	cache_l1d_total++;
      }
#endif

  /* find victim by LRU */
  for(w = 0; w < CACHE_L1_WAY; w++) {
    if(!cache[w][index].valid) {
      target = w;
      break;
    }

    miss = cache[w][index].miss;
    if(maxmiss < miss) {
      maxmiss = miss;
      target = w;
    }
  }

  cache[target][index].valid = true;
  cache[target][index].miss = 0;
  cache[target][index].tag = tag;
  memcpy(&cache[target][index].line,
	 memory_addr_phy2vm(paddr & CACHE_L1_LINE_MASK, false),
	 CACHE_L1_LINE_SIZE * sizeof(uint32_t));

  return cache[target][index].line[word];
}

static inline void memory_cache_l1_write(Memory paddr, uint32_t data)
{
  int w;
  unsigned int tag, index;

  tag = CACHE_L1_TAG(paddr);
  index = CACHE_L1_INDEX(paddr);

  if(paddr >= MEMORY_MAX_ADDR) {
    /* non-cache area */
#if CACHE_L1_D_ENABLE
    *(uint32_t *)memory_addr_phy2vm(paddr, true) = data;
#endif
    return;
  }

#if CACHE_L1_I_ENABLE
  for(w = 0; w < CACHE_L1_WAY; w++) {
    if(cache_l1i[w][index].tag == tag) {
      /* hit */
      cache_l1i[w][index].valid = false;
      break;
    }
  }
#endif

#if CACHE_L1_D_ENABLE
  unsigned int word;

  /* writethrough */
  *(uint32_t *)memory_addr_phy2vm(paddr, true) = data;

  for(w = 0; w < CACHE_L1_WAY; w++) {
    if(cache_l1d[w][index].tag == tag && cache_l1d[w][index].valid) {
      /* hit */
      word = CACHE_L1_WORD(paddr);
      cache_l1d[w][index].line[word] = data;

#if CACHE_L1_PROFILE
      cache_l1d_total++;
      cache_l1d_hit++;
#endif
      return;
    }
  }

  /* miss */
#if CACHE_L1_PROFILE
  cache_l1d_total++;
#endif
  memory_cache_l1_read(paddr, 0);
#endif
}

#endif
