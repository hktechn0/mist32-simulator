#include <stdio.h>
#include "common.h"

char *fmmu_mem;
char *fmmu_pagebuf;
char *fmmu_objcache;

uint32_t *fmmu_pagebuf_entry;
FLASHMMU_Object *fmmu_objects;

unsigned long fmmu_refcount;

void flashmmu_init(void)
{
  char *p;

  fmmu_refcount = false;

  printf("flashmmu size: 0x%lx\n", FLASHMMU_AREA_SIZE);
  //p = malloc(FLASHMMU_AREA_SIZE);
  p = valloc(FLASHMMU_AREA_SIZE);

  fmmu_mem = p;

  fmmu_pagebuf = p;
  p += FLASHMMU_PAGEBUF_SIZE;
  fmmu_objcache = p;
  p += FLASHMMU_OBJCACHE_SIZE;
  fmmu_pagebuf_entry = (uint32_t *)p;
  p += FLASHMMU_PAGEBUF_MAX * sizeof(uint32_t);
  fmmu_objects = (FLASHMMU_Object *)p;
}

uint16_t flashmmu_lru_victim(void)
{
  uint32_t oldref = 0;
  uint32_t entry;
  uint16_t victim, i;
  unsigned int ref;

  for(i = 0; i < FLASHMMU_PAGEBUF_MAX; i++) {
    entry = fmmu_pagebuf_entry[i];

    if(!FLASHMMU_PAGEBUF_FLAGS(entry)) {
      /* invalid buf */
      victim = i;
      break;
    }

    ref = fmmu_refcount - fmmu_objects[FLASHMMU_OBJID(entry)].ref;

    if(ref > oldref) {
      oldref = ref;
      victim = i;
    }
  }

  return victim;
}

void flashmmu_fetch_objcache(unsigned int objid)
{
  FLASHMMU_Object *victim, *obj;
  uint16_t victim_buf;
  uint32_t victim_entry, victim_id;

  victim_buf = flashmmu_lru_victim();
  victim_entry = fmmu_pagebuf_entry[victim_buf];
  victim_id = FLASHMMU_OBJID(victim_entry);

  victim = &fmmu_objects[victim_id];

  /* writeback victim object */
  if(victim->flags & FLASHMMU_FLAGS_DIRTYBUF) {
    memcpy(FLASHMMU_OBJCACHE_OBJ(fmmu_objcache, victim->cache_offset),
	   FLASHMMU_PAGEBUF_OBJ(fmmu_pagebuf, victim_buf), (size_t)victim->size);

    victim->flags &= ~FLASHMMU_FLAGS_DIRTYBUF;
    victim->flags |= FLASHMMU_FLAGS_DIRTY;

    printf("[FLASHMMU] writeback %d\n", victim_id);
  }

  victim->flags &= ~FLASHMMU_FLAGS_PAGEBUF;

  /* fetch object */
  obj = FLASHMMU_OBJ(objid);

  memcpy(FLASHMMU_PAGEBUF_OBJ(fmmu_pagebuf, victim_buf),
	 FLASHMMU_OBJCACHE_OBJ(fmmu_objcache, obj->cache_offset), (size_t)obj->size);

  obj->flags |= FLASHMMU_FLAGS_PAGEBUF;

  obj->buf_index = victim_buf;
  victim->buf_index = 0;
  fmmu_pagebuf_entry[victim_buf] = FLASHMMU_ADDR(objid) | FLASHMMU_FLAGS_VALID;

  printf("[FLASHMMU] fetch IN %d %x OUT %d %x\n",
	 objid, obj->cache_offset, victim_id, victim->cache_offset);
}

Memory flashmmu_access(uint32_t pte, Memory vaddr, bool is_write)
{
  unsigned int objid, offset;
  FLASHMMU_Object *obj;

  objid = FLASHMMU_OBJID(pte);
  offset = FLASHMMU_OFFSET(vaddr);

  obj = FLASHMMU_OBJ(objid);
  obj->ref = fmmu_refcount++;

  if(obj->size < offset) {
    /* protection error */
    errx(EXIT_FAILURE, "FlashMMU: protection error.");
  }

  if(is_write) {
    obj->flags |= FLASHMMU_FLAGS_DIRTYBUF;
  }

  /* is object on page buffer? */
  if(!(obj->flags & FLASHMMU_FLAGS_PAGEBUF)) {
    if(obj->flags & FLASHMMU_FLAGS_OBJCACHE) {
      /* refill page buffer */
      flashmmu_fetch_objcache(objid);
    }
    else {
      /* fault, fetch from flash */
      printf("[FLASHMMU] page fault %08x\n", vaddr);
      return memory_page_fault(vaddr);
    }
  }

  printf("[FLASHMMU] access %x %p size:%x buf:%x cache:%x ref:%x flags:%x\n",
	 objid, obj, obj->size, obj->buf_index, obj->cache_offset, obj->ref, obj->flags);

  /* return pagebuf physical address */
  return (FLASHMMU_PAGEBUF_ADDR + (obj->buf_index << 12)) | offset;
}
