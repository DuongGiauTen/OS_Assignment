/*
 * Copyright (C) 2026 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* LamiaAtrium release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */
 
 /* NOTICE this moudle is deprecated in LamiaAtrium release
  *        the structure is maintained for future 64bit-32bit
  *        backward compatible feature or PAE feature 
  */
 
#include "mm.h"
#include <stdlib.h>
#include <stdio.h>

/* Helper function to free a frame list */
int free_frm_lst(struct framephy_struct **frm_lst, struct memphy_struct *mp) {
    struct framephy_struct *current = *frm_lst;
    struct framephy_struct *next;

    while (current != NULL) {
        next = current->fp_next;
        // Trả lại frame cho bộ nhớ vật lý
        MEMPHY_put_freefp(mp, current->fpn); 
        free(current); // Giải phóng node
        current = next;
    }
    *frm_lst = NULL;
    return 0;
}

#if !defined(MM64)
/*
 * PAGING based Memory Management
 * Memory management unit mm/mm.c
 */

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
 * get_pd_from_pagenum - Parse address to 5 page directory level
 * @pgn   : pagenumer
 * @pgd   : page global directory
 * @p4d   : page level directory
 * @pud   : page upper directory
 * @pmd   : page middle directory
 * @pt    : page table 
 */
int get_pd_from_address(addr_t addr, addr_t* pgd, addr_t* p4d, addr_t* pud, addr_t* pmd, addr_t* pt)
{
  //printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  
  // need to modify more ----------------------------------------------------------
  const unsigned PAGE_SHIFT = 12;    /* 4KB pages */
  const unsigned LEVEL_BITS  = 9;    /* 5-level, 9 bits/level */
  const unsigned long long LEVEL_MASK = ((1ULL << LEVEL_BITS) - 1ULL);

  unsigned long long v = (unsigned long long)addr;
  unsigned long long pgn = v >> PAGE_SHIFT; /* page number */

  unsigned long long idx_pgd = (pgn >> (LEVEL_BITS * 4)) & LEVEL_MASK;
  unsigned long long idx_p4d = (pgn >> (LEVEL_BITS * 3)) & LEVEL_MASK;
  unsigned long long idx_pud = (pgn >> (LEVEL_BITS * 2)) & LEVEL_MASK;
  unsigned long long idx_pmd = (pgn >> (LEVEL_BITS * 1)) & LEVEL_MASK;
  unsigned long long idx_pt  = (pgn >> (LEVEL_BITS * 0)) & LEVEL_MASK;

  if (pgd) *pgd = (addr_t)idx_pgd;
  if (p4d) *p4d = (addr_t)idx_p4d;
  if (pud) *pud = (addr_t)idx_pud;
  if (pmd) *pmd = (addr_t)idx_pmd;
  if (pt)  *pt  = (addr_t)idx_pt;
  return 0;
}

/*
 * get_pd_from_pagenum - Parse page number to 5 page directory level
 * @pgn   : pagenumer
 * @pgd   : page global directory
 * @p4d   : page level directory
 * @pud   : page upper directory
 * @pmd   : page middle directory
 * @pt    : page table 
 */
int get_pd_from_pagenum(addr_t pgn, addr_t* pgd, addr_t* p4d, addr_t* pud, addr_t* pmd, addr_t* pt)
{
  printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  return 0;
}

/*
 * pte_set_swap - Set PTE entry for swapped page
 * @pte    : target page table entry (PTE)
 * @swptyp : swap type
 * @swpoff : swap offset
 */
int pte_set_swap(struct pcb_t *caller, addr_t pgn, int swptyp, addr_t swpoff)
{
  struct krnl_t *krnl = caller->krnl;
  addr_t *pte = &krnl->mm->pgd[pgn];
	
  SETBIT(*pte, PAGING_PTE_PRESENT_MASK);
  SETBIT(*pte, PAGING_PTE_SWAPPED_MASK);

  SETVAL(*pte, swptyp, PAGING_PTE_SWPTYP_MASK, PAGING_PTE_SWPTYP_LOBIT);
  SETVAL(*pte, swpoff, PAGING_PTE_SWPOFF_MASK, PAGING_PTE_SWPOFF_LOBIT);

  return 0;
}

/*
 * pte_set_swap - Set PTE entry for on-line page
 * @pte   : target page table entry (PTE)
 * @fpn   : frame page number (FPN)
 */
int pte_set_fpn(struct pcb_t *caller, addr_t pgn, addr_t fpn)
{
  struct krnl_t *krnl = caller->krnl;
  addr_t *pte = &krnl->mm->pgd[pgn];

  SETBIT(*pte, PAGING_PTE_PRESENT_MASK);
  CLRBIT(*pte, PAGING_PTE_SWAPPED_MASK);

  SETVAL(*pte, fpn, PAGING_PTE_FPN_MASK, PAGING_PTE_FPN_LOBIT);

  return 0;
}


/* Get PTE page table entry
 * @caller : caller
 * @pgn    : page number
 * @ret    : page table entry
 **/
uint32_t pte_get_entry(struct pcb_t *caller, addr_t pgn)
{
  printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  return 0;
}

/* Set PTE page table entry
 * @caller : caller
 * @pgn    : page number
 * @ret    : page table entry
 **/
int pte_set_entry(struct pcb_t *caller, addr_t pgn, uint32_t pte_val)
{
	struct krnl_t *krnl = caller->krnl;
	krnl->mm->pgd[pgn]=pte_val;
	
	return 0;
}

/*
 * vmap_pgd_memset - map a range of page at aligned address
 */
int vmap_pgd_memset(struct pcb_t *caller,           // process call
                    addr_t addr,                       // start address which is aligned to pagesz
                    int pgnum)                      // num of mapping page
{
  printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  return 0;
}

/*
 * vmap_page_range - map a range of page at aligned address
 */
addr_t vmap_page_range(struct pcb_t *caller,           // process call
                    addr_t addr,                       // start address which is aligned to pagesz
                    int pgnum,                      // num of mapping page
                    struct framephy_struct *frames, // list of the mapped frames
                    struct vm_rg_struct *ret_rg)    // return mapped region, the real mapped fp
{                                                   // no guarantee all given pages are mapped
  printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  return 0;
}

/*
 * alloc_pages_range - allocate req_pgnum of frame in ram
 * @caller    : caller
 * @req_pgnum : request page num
 * @frm_lst   : frame list
 */

addr_t alloc_pages_range(struct pcb_t *caller, int req_pgnum, struct framephy_struct **frm_lst)
{
  //printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  int pgit, fpn;
  struct framephy_struct *newfp_str = NULL; // pointer to a frame node
  int max_frames = caller->krnl->mram->maxsz / PAGING_PAGESZ;
  if (req_pgnum > max_frames){
    printf("[ERROR] %s: Request page number exceeds maximum frames in RAM\n", __func__);
    return -1;
  }
  for (pgit = 0; pgit < req_pgnum; pgit++)
  {
    if (MEMPHY_get_freefp(caller->krnl->mram, &fpn) != 0){
      int vicpgn;
      if (find_victim_page(caller->krnl->mm, &vicpgn) != 0){
        free_frm_lst(frm_lst, caller->krnl->mram);
        return -1;
      }
      int vicfpn = PAGING_FPN(caller->krnl->mm->pgd[vicpgn]);
      int swpfpn;
      if (MEMPHY_get_freefp(caller->krnl->active_mswp, &swpfpn) != 0){
        free_frm_lst(frm_lst, caller->krnl->mram);
        return -3000; // there is no frame left in RAM nor SWAP
      }
      // swap page from vicfpn to swpfpn
      __swap_cp_page(caller->krnl->mram, vicfpn, caller->krnl->active_mswp, swpfpn);
      pte_set_swap(caller, vicpgn, 0, swpfpn);
      fpn = vicfpn;
    }
    // make new frame node
    newfp_str = malloc(sizeof(struct framephy_struct)); // alloc a frame node
    newfp_str->fpn = fpn;
    newfp_str->owner = caller->krnl->mm;
    newfp_str->fp_next = NULL;
    // link node to frm_lst
    if (*frm_lst == NULL)
      *frm_lst = newfp_str; // if the fisrt node, so the head would be it
    else
    {
      newfp_str->fp_next = *frm_lst; // or, link the new node to the first (like the output)
    }
    *frm_lst = newfp_str; // update frm_lst head
  }
  

  return 0;
}

/*
 * vm_map_ram - do the mapping all vm are to ram storage device
 * @caller    : caller
 * @astart    : vm area start
 * @aend      : vm area end
 * @mapstart  : start mapping point
 * @incpgnum  : number of mapped page
 * @ret_rg    : returned region
 */
addr_t vm_map_ram(struct pcb_t *caller, addr_t astart, addr_t aend, addr_t mapstart, int incpgnum, struct vm_rg_struct *ret_rg)
{
  printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  return 0;
}

/* Swap copy content page from source frame to destination frame
 * @mpsrc  : source memphy
 * @srcfpn : source physical page number (FPN)
 * @mpdst  : destination memphy
 * @dstfpn : destination physical page number (FPN)
 **/
int __swap_cp_page(struct memphy_struct *mpsrc, addr_t srcfpn,
                   struct memphy_struct *mpdst, addr_t dstfpn)
{
  printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  return 0;
}

/*
 *Initialize a empty Memory Management instance
 * @mm:     self mm
 * @caller: mm owner
 */
int init_mm(struct mm_struct *mm, struct pcb_t *caller)
{
  //printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  struct vm_area_struct *vma0 = malloc(sizeof(struct vm_area_struct));
  if (vma0 == NULL) {
    // Memory allocation failed
    return -1;
  }
  vma0->vm_id = 0;
  vma0->vm_start = 0;
  vma0->vm_end = caller->bp;
  vma0->sbrk = 0;

  if (mm->mmap == NULL) {
    mm->mmap = vma0;
  } else {
    vma0->vm_next = mm->mmap;
    mm->mmap = vma0->vm_next;
  }
  mm->pgd = malloc(PAGING_MAX_PGN * sizeof(uint32_t));
  if (mm->pgd == NULL) {
    // Memory allocation failed
    free(vma0);
    return -1;
  }
}

struct vm_rg_struct *init_vm_rg(addr_t rg_start, addr_t rg_end)
{
  printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  return 0;
}

int enlist_vm_rg_node(struct vm_rg_struct **rglist, struct vm_rg_struct *rgnode)
{
  printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  return 0;
}

int enlist_pgn_node(struct pgn_t **plist, addr_t pgn)
{
  printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  return 0;
}

int print_list_fp(struct framephy_struct *ifp)
{
  printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  return 0;
}

int print_list_rg(struct vm_rg_struct *irg)
{
  printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  return 0;
}

int print_list_vma(struct vm_area_struct *ivma)
{
  printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  return 0;
}

int print_list_pgn(struct pgn_t *ip)
{
  printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  return 0;
}

int print_pgtbl(struct pcb_t *caller, uint32_t start, uint32_t end)
{
  printf("[ERROR] %s: This feature 32 bit mode is deprecated\n", __func__);
  return 0;
}

#endif //ndef MM64
