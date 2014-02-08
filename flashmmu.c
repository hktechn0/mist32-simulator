#include <stdio.h>
#include "common.h"

char *flashmmu_mem;

char *flashmmu_pagebuf;
char *flashmmu_objcache;
FLASHMMU_Object *flashmmu_objects;

static FLASHMMU_PagebufTag flashmmu_pagebuf_tag[FLASHMMU_PAGEBUF_PER_WAY];
static unsigned long flashmmu_tick;

void flashmmu_init(void)
{
  char *p;

  flashmmu_tick = 0;

  //p = malloc(FLASHMMU_AREA_SIZE);
  p = valloc(FLASHMMU_AREA_SIZE);

  flashmmu_mem = p;

  flashmmu_pagebuf = p;
  p += FLASHMMU_PAGEBUF_SIZE;
  flashmmu_objcache = p;
  p += FLASHMMU_OBJCACHE_SIZE;
  flashmmu_objects = (FLASHMMU_Object *)p;
}

static inline int flashmmu_lru_victim(unsigned int objid)
{
  uint32_t tag, hash;
  unsigned int i;
  unsigned int last, oldest, victim_way;

  hash = FLASHMMU_PAGEBUF_HASH(objid);
  oldest = 0;
  victim_way = 0;

  for(i = 0; i < FLASHMMU_PAGEBUF_WAY; i++) {
    tag = flashmmu_pagebuf_tag[hash].tag[i];

    if(!(FLASHMMU_PAGEBUF_FLAGS(tag) & FLASHMMU_FLAGS_VALID)) {
      /* invalid page buffer */
      return i;
    }
    else if(FLASHMMU_OBJID(tag) == objid){
      /* FIXME: cannot free pagebuf when obj_free */
      return i;
    }

    last = flashmmu_tick - flashmmu_pagebuf_tag[hash].last_access[i];

    if(last > oldest) {
      oldest = last;
      victim_way = i;
    }
  }

  return victim_way;
}

static inline int flashmmu_pagebuf_read(unsigned int objid)
{
  uint32_t tag, hash;
  unsigned int i;

  hash = FLASHMMU_PAGEBUF_HASH(objid);

  for(i = 0; i < FLASHMMU_PAGEBUF_WAY; i++) {
    tag = flashmmu_pagebuf_tag[hash].tag[i];

    if(FLASHMMU_OBJID(tag) == objid &&
       (FLASHMMU_PAGEBUF_FLAGS(tag) & FLASHMMU_FLAGS_VALID)) {
      return i;
    }
  }

  return -1;
}

static void flashmmu_fetch_objcache(unsigned int objid)
{
  FLASHMMU_Object *victim, *obj;
  unsigned int hash, victim_id, victim_way;

  hash = FLASHMMU_PAGEBUF_HASH(objid);

  /* find victim */
  victim_way = flashmmu_lru_victim(objid);
  victim_id = FLASHMMU_OBJID(flashmmu_pagebuf_tag[hash].tag[victim_way]);
  victim = &flashmmu_objects[victim_id];

  /* writeback victim object */
  if(victim->flags & FLASHMMU_FLAGS_DIRTYBUF) {
    memcpy(FLASHMMU_OBJCACHE_OBJ(flashmmu_objcache, victim->cache_offset),
	   FLASHMMU_PAGEBUF_OBJ(flashmmu_pagebuf, hash, victim_way), (size_t)victim->size);

    //DEBUGFLASH("[FLASHMMU] writeback %x\n", victim_id);

    victim->flags &= ~FLASHMMU_FLAGS_DIRTYBUF;
    victim->flags |= FLASHMMU_FLAGS_DIRTY;
  }

  victim->flags &= ~FLASHMMU_FLAGS_PAGEBUF;

  /* fetch object */
  obj = &flashmmu_objects[objid];
  flashmmu_pagebuf_tag[hash].tag[victim_way] = FLASHMMU_ADDR(objid) | FLASHMMU_FLAGS_VALID;

  memcpy(FLASHMMU_PAGEBUF_OBJ(flashmmu_pagebuf, hash, victim_way),
	 FLASHMMU_OBJCACHE_OBJ(flashmmu_objcache, obj->cache_offset), (size_t)obj->size);

  obj->flags |= FLASHMMU_FLAGS_PAGEBUF;

  DEBUGFLASH("[FLASHMMU] fetch IN %x %x OUT %x %x\n",
	     objid, obj->cache_offset, victim_id, victim->cache_offset);
}

Memory flashmmu_access(uint32_t pte, Memory vaddr, bool is_write)
{
  unsigned int objid, offset, index, way;
  FLASHMMU_Object *obj;

  objid = FLASHMMU_OBJID(pte);
  offset = FLASHMMU_OFFSET(vaddr);

  obj = &flashmmu_objects[objid];

  DEBUGFLASH("[FLASHMMU] access %x size:%x cache:%x flags:%x\n",
	     objid, obj->size, obj->cache_offset, obj->flags);

  if(!(obj->flags & FLASHMMU_FLAGS_VALID)) {
    /* protection error */
    errx(EXIT_FAILURE, "FlashMMU: invalid object.");
  }

  if(obj->size < offset) {
    /* protection error */
    errx(EXIT_FAILURE, "FlashMMU: protection error.");
  }

  /* is object on page buffer? */
  if(!(obj->flags & FLASHMMU_FLAGS_PAGEBUF)) {
    if(obj->flags & FLASHMMU_FLAGS_OBJCACHE) {
      /* refill page buffer */
      flashmmu_fetch_objcache(objid);
    }
    else {
      /* fault, fetch from flash */
      DEBUGFLASH("[FLASHMMU] page fault %08x\n", vaddr);
      return memory_page_fault(vaddr);
    }
  }

  if(is_write) {
    obj->flags |= FLASHMMU_FLAGS_DIRTYBUF;
  }

  way = flashmmu_pagebuf_read(objid);
  flashmmu_pagebuf_tag[FLASHMMU_PAGEBUF_HASH(objid)].last_access[way] = flashmmu_tick++;

  /* return pagebuf physical address */
  index = FLASHMMU_PAGEBUF_PER_WAY * way + FLASHMMU_PAGEBUF_HASH(objid);
  return FLASHMMU_PAGEBUF_ADDR + ((index << 12) | offset);
}
