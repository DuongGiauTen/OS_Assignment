/*
 * Copyright (C) 2026 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* LamiaAtrium release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

/*
 * PAGING based Memory Management
 * Memory management unit mm/mm.c
 */

#include "mm64.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/* Hàm hỗ trợ dịch địa chỉ 64-bit */
void translate_64bit_address(uint64_t addr,
                             uint32_t *pgd,
                             uint32_t *p4d,
                             uint32_t *pud,
                             uint32_t *pmd,
                             uint32_t *pt,
                             uint32_t *offset)
{
    *offset = addr & 0xFFFULL;
    *pt  = (addr >> 12) & 0x1FFULL;
    *pmd = (addr >> 21) & 0x1FFULL;
    *pud = (addr >> 30) & 0x1FFULL;
    *p4d = (addr >> 39) & 0x1FFULL;
    *pgd = (addr >> 48) & 0x1FFULL;
}

/* Hàm lấy index bảng trang từ địa chỉ ảo */
void get_pd_from_address(addr_t addr,
                         addr_t *pgd, addr_t *p4d, addr_t *pud,
                         addr_t *pmd, addr_t *pt)
{
    uint32_t dummy;
    translate_64bit_address(addr,
                            (uint32_t *)pgd,
                            (uint32_t *)p4d,
                            (uint32_t *)pud,
                            (uint32_t *)pmd,
                            (uint32_t *)pt,
                            &dummy);
}

/* Hàm lấy index bảng trang từ số hiệu trang (PGN) */
void get_pd_from_pagenum(uint64_t pgn,
                         uint32_t *pgd,
                         uint32_t *p4d,
                         uint32_t *pud,
                         uint32_t *pmd,
                         uint32_t *pt)
{
    uint64_t addr = pgn << 12;
    uint32_t dummy;
    translate_64bit_address(addr, pgd, p4d, pud, pmd, pt, &dummy);
}

/*
 * init_pte - Initialize PTE entry
 */
int init_pte(addr_t *pte,
             int pre,    // present
             addr_t fpn,    // FPN
             int drt,    // dirty
             int swp,    // swap
             int swptyp, // swap type
             addr_t swpoff) // swap offset
{
  if (pre != 0) {
    if (swp == 0) { // Non swap ~ page online
      if (fpn == 0)
        return -1;  // Invalid setting

      /* Valid setting with FPN */
      SETBIT(*pte, PAGING_PTE_PRESENT_MASK);
      CLRBIT(*pte, PAGING_PTE_SWAPPED_MASK);
      CLRBIT(*pte, PAGING_PTE_DIRTY_MASK);

      SETVAL(*pte, fpn, PAGING_PTE_FPN_MASK, PAGING_PTE_FPN_LOBIT);
    }
    else
    { // page swapped
      SETBIT(*pte, PAGING_PTE_PRESENT_MASK);
      SETBIT(*pte, PAGING_PTE_SWAPPED_MASK);
      CLRBIT(*pte, PAGING_PTE_DIRTY_MASK);

      SETVAL(*pte, swptyp, PAGING_PTE_SWPTYP_MASK, PAGING_PTE_SWPTYP_LOBIT);
      SETVAL(*pte, swpoff, PAGING_PTE_SWPOFF_MASK, PAGING_PTE_SWPOFF_LOBIT);
    }
  }

  return 0;
}

/*
 * pte_set_swap - Set PTE entry for swapped page
 */
int pte_set_swap(struct pcb_t *caller, addr_t pgn, int swptyp, addr_t swpoff)
{
  addr_t *pte;
  addr_t pgd=0, p4d=0, pud=0, pmd=0, pt=0;
    
  // dummy pte alloc to avoid runtime error
  pte = malloc(sizeof(addr_t));

  /* Get value from the system */
  get_pd_from_pagenum(pgn, (uint32_t*)&pgd, (uint32_t*)&p4d, (uint32_t*)&pud, (uint32_t*)&pmd, (uint32_t*)&pt);
    
  SETBIT(*pte, PAGING_PTE_PRESENT_MASK);
  SETBIT(*pte, PAGING_PTE_SWAPPED_MASK);

  SETVAL(*pte, swptyp, PAGING_PTE_SWPTYP_MASK, PAGING_PTE_SWPTYP_LOBIT);
  SETVAL(*pte, swpoff, PAGING_PTE_SWPOFF_MASK, PAGING_PTE_SWPOFF_LOBIT);

  return 0;
}

/*
 * pte_set_fpn - Set PTE entry for on-line page
 */
int pte_set_fpn(struct pcb_t *caller, addr_t pgn, addr_t fpn)
{
  addr_t *pte;
  addr_t pgd=0, p4d=0, pud=0, pmd=0, pt=0;
    
  // dummy pte alloc to avoid runtime error
  pte = malloc(sizeof(addr_t));

  /* Get value from the system */
  get_pd_from_pagenum(pgn, (uint32_t*)&pgd, (uint32_t*)&p4d, (uint32_t*)&pud, (uint32_t*)&pmd, (uint32_t*)&pt);

  SETBIT(*pte, PAGING_PTE_PRESENT_MASK);
  CLRBIT(*pte, PAGING_PTE_SWAPPED_MASK);

  SETVAL(*pte, fpn, PAGING_PTE_FPN_MASK, PAGING_PTE_FPN_LOBIT);

  return 0;
}


/* Get PTE page table entry */
uint32_t pte_get_entry(struct pcb_t *caller, uint32_t pgn)
{
    uint32_t pgd_i, p4d_i, pud_i, pmd_i, pt_i, off;

    /* chuyển pagenum thành địa chỉ ảo */
    uint64_t vaddr = ((uint64_t)pgn << 12);

    translate_64bit_address(vaddr, &pgd_i, &p4d_i, &pud_i, &pmd_i, &pt_i, &off);

    struct mm_struct *mm = caller->krnl->mm;

    if (!mm->pgd) return 0;

    /* Truy cập PGD như mảng 64-bit */
    uint64_t *pgd_ptr = (uint64_t *)mm->pgd;
    
    if (!pgd_ptr[pgd_i]) return 0;
    uint64_t *p4d = (uint64_t *)(uintptr_t)pgd_ptr[pgd_i];

    if (!p4d[p4d_i]) return 0;
    uint64_t *pud = (uint64_t *)(uintptr_t)p4d[p4d_i];

    if (!pud[pud_i]) return 0;
    uint64_t *pmd = (uint64_t *)(uintptr_t)pud[pud_i];

    if (!pmd[pmd_i]) return 0;
    uint64_t *pt = (uint64_t *)(uintptr_t)pmd[pmd_i];

    return (uint32_t)pt[pt_i];
}

/* Set PTE page table entry */
int pte_set_entry(struct pcb_t *caller, addr_t vaddr, uint32_t pte_val)
{
    uint32_t pgd_i, p4d_i, pud_i, pmd_i, pt_i, off;
    struct mm_struct *mm = caller->krnl->mm;

    /* Tách địa chỉ ảo thành index 5 cấp */
    translate_64bit_address((uint64_t)vaddr,
                             &pgd_i, &p4d_i, &pud_i,
                             &pmd_i, &pt_i, &off);

    /* Cấp phát PGD nếu chưa có */
    if (!mm->pgd)
        mm->pgd = (addr_t*)calloc(512, sizeof(uint64_t));

    /* Ép kiểu pgd về uint64_t* để thao tác đúng kích thước pointer */
    uint64_t *pgd_ptr = (uint64_t *)mm->pgd;

    if (!pgd_ptr[pgd_i])
        pgd_ptr[pgd_i] = (uint64_t)(uintptr_t)calloc(512, sizeof(uint64_t));

    uint64_t *p4d = (uint64_t *)(uintptr_t)pgd_ptr[pgd_i];

    if (!p4d[p4d_i])
        p4d[p4d_i] = (uint64_t)(uintptr_t)calloc(512, sizeof(uint64_t));

    uint64_t *pud = (uint64_t *)(uintptr_t)p4d[p4d_i];

    if (!pud[pud_i])
        pud[pud_i] = (uint64_t)(uintptr_t)calloc(512, sizeof(uint64_t));

    uint64_t *pmd = (uint64_t *)(uintptr_t)pud[pud_i];

    if (!pmd[pmd_i])
        pmd[pmd_i] = (uint64_t)(uintptr_t)calloc(512, sizeof(uint64_t));

    uint64_t *pt = (uint64_t *)(uintptr_t)pmd[pmd_i];

    /* Gán entry cuối cùng */
    pt[pt_i] = pte_val;

    return 0;
}

/*
 * vmap_pgd_memset - map a range of page at aligned address
 */
int vmap_pgd_memset(struct pcb_t *caller, addr_t addr, int pgnum, addr_t pattern)
{
  addr_t *pgd_table = caller->krnl->mm->pgd;
  addr_t pgd_idx;
  int i;

  for(i = 0; i < pgnum; i++) {
      pgd_idx = PAGING64_ADDR_PGD(addr + i * PAGING64_PAGESZ);
      if (caller->krnl->mm->pgd != NULL) {
          pgd_table[pgd_idx] = pattern;
      }
  }
  return 0;
}

/* vmap_page_range */
addr_t vmap_page_range(struct pcb_t *caller, addr_t addr, int pgnum, 
                       struct framephy_struct *frames, struct vm_rg_struct *ret_rg)
{
    struct mm_struct *mm = caller->krnl->mm;
    struct framephy_struct *fpit = frames;

    ret_rg->rg_start = addr;
    ret_rg->rg_end   = addr + pgnum * PAGING_PAGESZ;

    addr_t curr_va = addr;
    for (int i = 0; i < pgnum && fpit != NULL; i++)
    {
        addr_t pfn = fpit->fpn;
        uint64_t pte_val = ((uint64_t)pfn << 12) | 1; 
        pte_set_entry(caller, curr_va, pte_val);
        enlist_pgn_node(&mm->fifo_pgn, curr_va >> 12);

        curr_va += PAGING_PAGESZ;
        fpit = fpit->fp_next;
    }
    return 0;
}

/* alloc_pages_range */
addr_t alloc_pages_range(struct pcb_t *caller, int req_pgnum, struct framephy_struct **frm_lst)
{
    // Placeholder logic for compilation
    return 0;
}

/* vm_map_ram */
addr_t vm_map_ram(struct pcb_t *caller, addr_t astart, addr_t aend, addr_t mapstart, int incpgnum, struct vm_rg_struct *ret_rg)
{
  return 0;
}

/* __swap_cp_page */
int __swap_cp_page(struct memphy_struct *mpsrc, addr_t srcfpn,
                   struct memphy_struct *mpdst, addr_t dstfpn)
{
  return 0;
}

/* init_mm */
int init_mm(struct mm_struct *mm, struct pcb_t *caller)
{
  struct vm_area_struct *vma0 = malloc(sizeof(struct vm_area_struct));

  /* Cấp phát PGD */
  mm->pgd = calloc(512, sizeof(uint64_t));
  
  /* XÓA CÁC DÒNG mm->p4d, mm->pud... VÌ STRUCT KHÔNG CÓ */
  
  vma0->vm_id = 0;
  vma0->vm_start = 0;
  vma0->vm_end = vma0->vm_start;
  vma0->sbrk = vma0->vm_start;
  struct vm_rg_struct *first_rg = init_vm_rg(vma0->vm_start, vma0->vm_end);
  enlist_vm_rg_node(&vma0->vm_freerg_list, first_rg);

  vma0->vm_next = NULL; 
  vma0->vm_mm = mm; 
  mm->mmap = vma0;
  
  return 0;
}

struct vm_rg_struct *init_vm_rg(addr_t rg_start, addr_t rg_end)
{
  struct vm_rg_struct *rgnode = malloc(sizeof(struct vm_rg_struct));
  rgnode->rg_start = rg_start;
  rgnode->rg_end = rg_end;
  rgnode->rg_next = NULL;
  return rgnode;
}

int enlist_vm_rg_node(struct vm_rg_struct **rglist, struct vm_rg_struct *rgnode)
{
  rgnode->rg_next = *rglist;
  *rglist = rgnode;
  return 0;
}

int enlist_pgn_node(struct pgn_t **plist, addr_t pgn)
{
  struct pgn_t *pnode = malloc(sizeof(struct pgn_t));
  pnode->pgn = pgn;
  pnode->pg_next = *plist;
  *plist = pnode;
  return 0;
}

int print_list_fp(struct framephy_struct *ifp)
{
  struct framephy_struct *fp = ifp;
  printf("print_list_fp: \n");
  if (fp == NULL) { printf("NULL list\n"); return -1;}
  while (fp != NULL) {
    printf("fp[%d]\n", fp->fpn);
    fp = fp->fp_next;
  }
  return 0;
}

int print_list_rg(struct vm_rg_struct *irg)
{
  struct vm_rg_struct *rg = irg;
  printf("print_list_rg: \n");
  if (rg == NULL) { printf("NULL list\n"); return -1; }
  while (rg != NULL) {
    printf("rg[%ld->%ld]\n", rg->rg_start, rg->rg_end);
    rg = rg->rg_next;
  }
  return 0;
}

int print_list_vma(struct vm_area_struct *ivma)
{
  struct vm_area_struct *vma = ivma;
  printf("print_list_vma: \n");
  if (vma == NULL) { printf("NULL list\n"); return -1; }
  while (vma != NULL) {
    printf("va[%ld->%ld]\n", vma->vm_start, vma->vm_end);
    vma = vma->vm_next;
  }
  return 0;
}

int print_list_pgn(struct pgn_t *ip)
{
  printf("print_list_pgn: \n");
  if (ip == NULL) { printf("NULL list\n"); return -1; }
  while (ip != NULL) {
    printf("pgn[%d]\n", ip->pgn);
    ip = ip->pg_next;
  }
  return 0;
}

int print_pgtbl(struct pcb_t *caller, addr_t start, addr_t end)
{
  addr_t pgd=0, p4d=0, pud=0, pmd=0, pt=0;
  printf("--- Dumping Page Table ---\n");
  get_pd_from_address(start, &pgd, &p4d, &pud, &pmd, &pt);
  printf("Addr: %ld -> PGD: %04ld | P4D: %04ld | PUD: %04ld | PMD: %04ld | PT: %04ld\n", 
         start, (long)pgd, (long)p4d, (long)pud, (long)pmd, (long)pt);
  return 0;
}