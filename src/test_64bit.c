#include <stdio.h>
#include <stdint.h>
#include <assert.h>

void translate_64bit_address(uint64_t addr,
    uint32_t *pgd, uint32_t *p4d, uint32_t *pud,
    uint32_t *pmd, uint32_t *pt, uint32_t *off);

int main()
{
    uint64_t addr = 0x39ffe1d9c9000ULL;

    uint32_t pgd, p4d, pud, pmd, pt, off;
    translate_64bit_address(addr, &pgd, &p4d, &pud, &pmd, &pt, &off);

    assert(off == 0x0);
    assert(pt  == 457);
    assert(pmd == 236);
    assert(pud == 504);
    assert(p4d == 319);
    assert(pgd == 3);

    printf("64BIT PAGING TEST PASSED!\n");
    return 0;
}
