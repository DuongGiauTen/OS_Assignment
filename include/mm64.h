/* include/mm64.h */
#ifndef MM64_H
#define MM64_H

#include "mm.h"

/* ... Các macro PAGING64_... giữ nguyên ... */

/* Function Prototypes */
void translate_64bit_address(uint64_t addr,
                             uint32_t *pgd,
                             uint32_t *p4d,
                             uint32_t *pud,
                             uint32_t *pmd,
                             uint32_t *pt,
                             uint32_t *offset);

int get_pd_from_address(addr_t addr,
                         addr_t *pgd, addr_t *p4d, addr_t *pud,
                         addr_t *pmd, addr_t *pt);

int get_pd_from_pagenum(uint64_t pgn,
                         uint32_t *pgd,
                         uint32_t *p4d,
                         uint32_t *pud,
                         uint32_t *pmd,
                         uint32_t *pt);

int vmap_pgd_memset(struct pcb_t *caller, addr_t addr, int pgnum, addr_t pattern);

/* Prototype cho các hàm khác */
uint32_t pte_get_entry(struct pcb_t *caller, addr_t pgn);
int pte_set_entry(struct pcb_t *caller, addr_t vaddr, uint32_t pte_val);
int pte_set_fpn(struct pcb_t *caller, addr_t pgn, addr_t fpn);
int pte_set_swap(struct pcb_t *caller, addr_t pgn, int swptyp, addr_t swpoff);
int init_mm(struct mm_struct *mm, struct pcb_t *caller);
int init_pte(addr_t *pte, int pre, addr_t fpn, int drt, int swp, int swptyp, addr_t swpoff);

#endif