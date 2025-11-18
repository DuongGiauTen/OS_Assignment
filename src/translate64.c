/*
 * translate_vmap64.c
 * Provide 64-bit address translation + PGD memset helper
 *
 * This file is standalone. Include it in your build together with:
 *   - mm64.h
 *   - os-mm.h
 *   - mm64.c
 */

#include <stdint.h>
#include "mm64.h"
#include "os-mm.h"

/*
 * translate_64bit_address()
 * Split 64-bit VA into:
 *   PGD, P4D, PUD, PMD, PT, OFFSET
 *
 * Follows 5-level x86-style paging: 9-9-9-9-9 + 12 offset = 57 bits.
 */
void translate_64bit_address(uint64_t addr,
                             uint32_t *idx_pgd,
                             uint32_t *idx_p4d,
                             uint32_t *idx_pud,
                             uint32_t *idx_pmd,
                             uint32_t *idx_pt,
                             uint32_t *offset)
{
    if (offset) *offset = (uint32_t)(addr & 0xFFFULL);    // 12-bit offset
    if (idx_pt)  *idx_pt  = (uint32_t)((addr >> 12) & 0x1FFULL); // 9 bits
    if (idx_pmd) *idx_pmd = (uint32_t)((addr >> 21) & 0x1FFULL);
    if (idx_pud) *idx_pud = (uint32_t)((addr >> 30) & 0x1FFULL);
    if (idx_p4d) *idx_p4d = (uint32_t)((addr >> 39) & 0x1FFULL);
    if (idx_pgd) *idx_pgd = (uint32_t)((addr >> 48) & 0x1FFULL);
}

/*
 * get_pd_from_pagenum()
 * pgn: page number (address >> 12)
 * Convert it back to aligned VA then reuse translate_64bit_address
 */
void get_pd_from_pagenum(uint64_t pgn,
                         uint32_t *idx_pgd,
                         uint32_t *idx_p4d,
                         uint32_t *idx_pud,
                         uint32_t *idx_pmd,
                         uint32_t *idx_pt)
{
    uint64_t addr = pgn << 12;
    translate_64bit_address(addr, idx_pgd, idx_p4d, idx_pud, idx_pmd, idx_pt, NULL);
}

/*
 * vmap_pgd_memset()
 * Write 'pattern' into PGD entries for a range of pages.
 *
 * This is the simplified version required in your assignment:
 * - treat mm->pgd as a large linear PGD array
 * - assign each entry corresponding to (addr>>12) + i
 */
int vmap_pgd_memset(struct pcb_t *caller, addr_t addr, int pgnum, addr_t pattern)
{
    if (!caller)
        return -1;

    /* Locate mm_struct.
       If your pcb_t uses different field names, adjust here. */
    struct mm_struct *mm = NULL;

    if (caller->krnl && caller->krnl->mm)
        mm = caller->krnl->mm;
    else
        return -1;

#ifndef MM64
    return -1; /* Only for 64-bit mode */
#else
    if (!mm->pgd)
        return -1;

    uint64_t *pgd = mm->pgd;

    uint64_t start_pgn = ((uint64_t)addr) >> 12;

    for (int i = 0; i < pgnum; ++i) {
        uint64_t idx = start_pgn + (uint64_t)i;
        pgd[idx] = (uint64_t)pattern;
    }

    return 0;
#endif
}
