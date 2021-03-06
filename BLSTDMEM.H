/* Standard Banking Library (BLSTDMEM.H) */

struct bl_lmem_t {
	uint8_t page_tbl[64];		/* 16KB x 64page = 1024KB */
	uint8_t page_max;
	uint8_t sys_used;		/* system segment flag */
};

uint32_t bl_lmem_get_free(void);
struct bl_lmem_t *bl_lmem_alloc(uint32_t size);
struct bl_lmem_t *bl_lmem_alloc_sys(uint32_t size);
void bl_lmem_free(struct bl_lmem_t *ptr);
uint8_t bl_lmem_get_seg(struct bl_lmem_t *ptr, uint32_t addr32);
void bl_lmem_export(struct bl_lmem_t *ptr, char *name);
struct bl_lmem_t *bl_lmem_import(char *name);
void bl_lmem_write(struct bl_lmem_t *ptr, uint32_t addr32, uint8_t data);
uint8_t bl_lmem_read(struct bl_lmem_t *ptr, uint32_t addr32);
void bl_lmem_copy_to(struct bl_lmem_t *dest, uint32_t addr32, uint8_t *src, uint16_t size);
void bl_lmem_copy_from(uint8_t *dest, struct bl_lmem_t *src, uint32_t addr32, uint16_t size);
