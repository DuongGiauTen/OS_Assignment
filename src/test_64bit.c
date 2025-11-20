#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include "mm64.h"

/* Test translate 64-bit */
void test_translate()
{
    uint64_t va = 0x39ffe1d9c9000ULL; // Địa chỉ mẫu

    uint32_t pgd, p4d, pud, pmd, pt, off;
    translate_64bit_address(va, &pgd, &p4d, &pud, &pmd, &pt, &off);

    printf("PGD=%u P4D=%u PUD=%u PMD=%u PT=%u OFF=%u\n",
           pgd, p4d, pud, pmd, pt, off);

    /* CẬP NHẬT ASSERT CHO ĐÚNG VỚI ĐỊA CHỈ 0x39ffe1d9c9000 */
    assert(pgd == 3);    // 0x3
    assert(p4d == 319);  // 0x13F
    assert(pud == 504);  // 0x1F8
    assert(pmd == 236);  // 0xEC
    assert(pt  == 457);  // 0x1C9
    assert(off == 0);
}

/* Test pte_set_entry và pte_get_entry */
void test_pte()
{
    struct pcb_t fake;
    struct mm_struct mm;

    fake.krnl = malloc(sizeof(struct krnl_t));
    fake.krnl->mm = &mm;

    /* Khởi tạo NULL để test logic tự cấp phát (demand paging) */
    mm.pgd = NULL; 

    /* Map 1 entry */
    uint64_t va = 0x12345000ULL;
    uint32_t pte_val = (99 << 12) | 1;

    pte_set_entry(&fake, va, pte_val);

    uint32_t got = pte_get_entry(&fake, va >> 12);

    assert(got == pte_val);

    printf("PTE test OK\n");
}

int main()
{
    printf("=== TEST 64BIT PAGING ===\n");

    test_translate();
    test_pte();

    printf("ALL TEST PASSED!\n");
    return 0;
}