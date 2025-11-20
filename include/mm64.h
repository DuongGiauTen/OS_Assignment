#ifndef MM64_H
#define MM64_H

#include "mm.h"
#define MM64_BITS_PER_LONG 64

#define PAGING64_CPU_BUS_WIDTH 57 /* 57 bit bus - MAX SPACE 4MB */
#define PAGING64_PAGESZ  4096      /* 4KB or 12-bits PAGE NUMBER */

#define GENMASK64(h, l) \
	(((~0ULL) << (l)) & (~0ULL >> (MM64_BITS_PER_LONG  - (h) - 1)))

#define PAGING64_MAX_PGN  (DIV_ROUND_UP(BIT_ULL(21),PAGING64_PAGESZ))
#define PAGING64_PAGE_ALIGNSZ(sz) (DIV_ROUND_UP(sz,PAGING64_PAGESZ)*PAGING64_PAGESZ)

/* OFFSET */
#define PAGING64_ADDR_OFFST_HIBIT 11
#define PAGING64_ADDR_OFFST_LOBIT 0

/* PT */
#define PAGING64_ADDR_PT_HIBIT 20
#define PAGING64_ADDR_PT_LOBIT 12

/* PMD */
#define PAGING64_ADDR_PMD_HIBIT 29
#define PAGING64_ADDR_PMD_LOBIT 21

/* PUD */
#define PAGING64_ADDR_PUD_HIBIT 38
#define PAGING64_ADDR_PUD_LOBIT 30

/* P4D */
#define PAGING64_ADDR_P4D_HIBIT 47
#define PAGING64_ADDR_P4D_LOBIT 39

/* PGD */
#define PAGING64_ADDR_PGD_HIBIT 56
#define PAGING64_ADDR_PGD_LOBIT 48

/* Extract PGD Entry Macros */
#define PAGING64_ADDR_OFFST(addr) GETVAL(addr,PAGING_ADDR_OFFST_MASK,PAGING_ADDR_OFFST_LOBIT)
#define PAGING64_ADDR_PT(addr)   ((addr&PAGING64_ADDR_PT_MASK)>>PAGING64_ADDR_PT_LOBIT)
#define PAGING64_ADDR_PMD(addr)   ((addr&PAGING64_ADDR_PMD_MASK)>>PAGING64_ADDR_PMD_LOBIT)
#define PAGING64_ADDR_PUD(addr)   ((addr&PAGING64_ADDR_PUD_MASK)>>PAGING64_ADDR_PUD_LOBIT)
#define PAGING64_ADDR_P4D(addr)   ((addr&PAGING64_ADDR_P4D_MASK)>>PAGING64_ADDR_P4D_LOBIT)
#define PAGING64_ADDR_PGD(addr)   ((addr&PAGING64_ADDR_PGD_MASK)>>PAGING64_ADDR_PGD_LOBIT)

/* Masks */
#define PAGING64_ADDR_OFFST_MASK  GENMASK64(PAGING_ADDR_OFFST_HIBIT,PAGING_ADDR_OFFST_LOBIT)
#define PAGING64_ADDR_PT_MASK  GENMASK64(PAGING64_ADDR_PT_HIBIT,PAGING64_ADDR_PT_LOBIT)
#define PAGING64_ADDR_PMD_MASK  GENMASK64(PAGING64_ADDR_PMD_HIBIT,PAGING64_ADDR_PMD_LOBIT)
#define PAGING64_ADDR_PUD_MASK  GENMASK64(PAGING64_ADDR_PUD_HIBIT,PAGING64_ADDR_PUD_LOBIT)
#define PAGING64_ADDR_P4D_MASK  GENMASK64(PAGING64_ADDR_P4D_HIBIT,PAGING64_ADDR_P4D_LOBIT)
#define PAGING64_ADDR_PGD_MASK  GENMASK64(PAGING64_ADDR_PGD_HIBIT,PAGING64_ADDR_PGD_LOBIT)

/* Function Prototypes */
void translate_64bit_address(uint64_t addr,
                             uint32_t *pgd,
                             uint32_t *p4d,
                             uint32_t *pud,
                             uint32_t *pmd,
                             uint32_t *pt,
                             uint32_t *offset);

void get_pd_from_address(addr_t addr,
                         addr_t *pgd, addr_t *p4d, addr_t *pud,
                         addr_t *pmd, addr_t *pt);

void get_pd_from_pagenum(uint64_t pgn,
                         uint32_t *pgd,
                         uint32_t *p4d,
                         uint32_t *pud,
                         uint32_t *pmd,
                         uint32_t *pt);

int vmap_pgd_memset(struct pcb_t *caller, addr_t addr, int pgnum, addr_t pattern);

/* Bổ sung các prototype còn thiếu để test_64bit.c có thể gọi */
uint32_t pte_get_entry(struct pcb_t *caller, uint32_t pgn);
int pte_set_entry(struct pcb_t *caller, addr_t vaddr, uint32_t pte_val);
int pte_set_fpn(struct pcb_t *caller, addr_t pgn, addr_t fpn);
int pte_set_swap(struct pcb_t *caller, addr_t pgn, int swptyp, addr_t swpoff);
int init_mm(struct mm_struct *mm, struct pcb_t *caller);
int init_pte(addr_t *pte, int pre, addr_t fpn, int drt, int swp, int swptyp, addr_t swpoff);

#endif