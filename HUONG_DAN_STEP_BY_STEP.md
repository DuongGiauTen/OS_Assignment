# 🚀 Step-by-Step Implementation Guide

## Phần 1: Fix Macro (5 phút)

### **Bước 1.1: Mở file `include/mm.h`**

Tìm đến dòng ~72-73:

```c
#define OVERLAP(x1,x2,y1,y2) (0)       ← ❌ Cần sửa
#define INCLUDE(x1,x2,y1,y2) (0)       ← ❌ Cần sửa
```

### **Bước 1.2: Thay thế bằng logic đúng**

```c
#define OVERLAP(x1,x2,y1,y2) ((x1 < y2) && (y1 < x2))
#define INCLUDE(x1,x2,y1,y2) ((x1 <= y1) && (y2 <= x2))
```

**Giải thích:**
- **OVERLAP:** Hai range `[x1,x2)` và `[y1,y2)` có giao nhau
  - Điều kiện: `x1 < y2 AND y1 < x2`
  - Ví dụ: [0,10) và [5,15) → overlap ✓
  - Ví dụ: [0,10) và [10,20) → không overlap ✓

- **INCLUDE:** Range `[x1,x2)` bao hàm range `[y1,y2)`
  - Điều kiện: `x1 <= y1 AND y2 <= x2`
  - Ví dụ: [0,20) bao hàm [5,15) ✓
  - Ví dụ: [0,10) không bao hàm [5,15) ✓

---

## Phần 2: Hoàn thiện mm-vm.c (45 phút)

### **Bước 2.1: Sửa `get_vm_area_node_at_brk()`**

**File:** `src/mm-vm.c` dòng 58-81

**Thay thế từ:**
```c
struct vm_rg_struct *get_vm_area_node_at_brk(struct pcb_t *caller, int vmaid, addr_t size, addr_t alignedsz)
{
  struct vm_rg_struct * newrg;
  /* TODO retrive current vma to obtain newrg, current comment out due to compiler redundant warning*/
  //struct vm_area_struct *cur_vma = get_vma_by_num(caller->kernl->mm, vmaid);

  //newrg = malloc(sizeof(struct vm_rg_struct));

  /* TODO: update the newrg boundary
  // newrg->rg_start = ...
  // newrg->rg_end = ...
  */
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->krnl->mm, vmaid);

  newrg = malloc(sizeof(struct vm_rg_struct));
  newrg->rg_start = cur_vma->sbrk;
  newrg->rg_end = newrg->rg_start + size;
  /* END TODO */

  return newrg;
}
```

**Thành:**
```c
struct vm_rg_struct *get_vm_area_node_at_brk(struct pcb_t *caller, int vmaid, addr_t size, addr_t alignedsz)
{
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->krnl->mm, vmaid);
  
  if (cur_vma == NULL)
    return NULL;

  struct vm_rg_struct *newrg = malloc(sizeof(struct vm_rg_struct));
  if (newrg == NULL)
    return NULL;

  /* Set region boundaries */
  newrg->rg_start = cur_vma->sbrk;
  newrg->rg_end = newrg->rg_start + alignedsz;
  newrg->rg_next = NULL;

  /* Update the break point */
  cur_vma->sbrk = newrg->rg_end;

  return newrg;
}
```

---

### **Bước 2.2: Hoàn thiện `validate_overlap_vm_area()`**

**File:** `src/mm-vm.c` dòng 84-125

Sau khi fix macro OVERLAP ở bước 1.2, hàm này sẽ hoạt động đúng. Không cần sửa đổi nội bộ.

**Kiểm tra:** Function nên kiểm tra xem [vmastart, vmaend) có overlap với VMAs khác không.

---

### **Bước 2.3: Hoàn thiện `inc_vma_limit()`**

**File:** `src/mm-vm.c` dòng 128-157

**Thay thế từ:**
```c
int inc_vma_limit(struct pcb_t *caller, int vmaid, addr_t inc_sz)
{
  //struct vm_rg_struct * newrg = malloc(sizeof(struct vm_rg_struct));

  /* TOTO with new address scheme, the size need tobe aligned 
   *      the raw inc_sz maybe not fit pagesize
   */ 
  //addr_t inc_amt;

//  int incnumpage =  inc_amt / PAGING_PAGESZ;

  /* TODO Validate overlap of obtained region */
  //if (validate_overlap_vm_area(caller, vmaid, area->rg_start, area->rg_end) < 0)
  //  return -1; /*Overlap and failed allocation */

  /* TODO: Obtain the new vm area based on vmaid */
  //cur_vma->vm_end... 
  // inc_limit_ret...
  /* The obtained vm area (only)
   * now will be alloc real ram region */

//  if (vm_map_ram(caller, area->rg_start, area->rg_end, 
//                   old_end, incnumpage , newrg) < 0)
//    return -1; /* Map the memory to MEMRAM */

  return 0;
}
```

**Thành:**
```c
int inc_vma_limit(struct pcb_t *caller, int vmaid, addr_t inc_sz)
{
  /* Align size with page size */
  addr_t inc_amt = PAGING_PAGE_ALIGNSZ(inc_sz);
  int incnumpage = DIV_ROUND_UP(inc_amt, PAGING_PAGESZ);

  /* Get current VMA */
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->krnl->mm, vmaid);
  if (cur_vma == NULL)
    return -1;

  /* Calculate new end address */
  addr_t old_end = cur_vma->vm_end;
  addr_t new_end = old_end + inc_amt;

  /* Validate overlap with other VMAs */
  if (validate_overlap_vm_area(caller, vmaid, old_end, new_end) < 0)
    return -1;  /* Overlap and failed allocation */

  /* Create new region structure */
  struct vm_rg_struct newrg;
  newrg.rg_start = old_end;
  newrg.rg_end = new_end;
  newrg.rg_next = NULL;

  /* Map memory to RAM */
  if (vm_map_ram(caller, newrg.rg_start, newrg.rg_end, 
                 old_end, incnumpage, &newrg) < 0)
    return -1;  /* Map the memory to MEMRAM */

  /* Update VMA end address */
  cur_vma->vm_end = new_end;

  return 0;
}
```

---

## Phần 3: Tạo Syscall Handlers Mới (30 phút)

### **Bước 3.1: Tạo `src/sys_meminc.c`**

**Tạo file mới với nội dung:**

```c
/*
 * Copyright (C) 2026 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* LamiaAtrium release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

#include "syscall.h"
#include "common.h"
#include "os-mm.h"
#include "mm.h"
#include <stdlib.h>

int __sys_meminc(struct krnl_t *krnl, uint32_t pid, struct sc_regs* regs)
{
    /* Extract syscall parameters */
    int vmaid = regs->a1;       /* Virtual Memory Area ID */
    addr_t inc_sz = regs->a2;   /* Increment size */
    
    /* TODO: Traverse proclist to find matching process by PID */
    struct pcb_t *caller = NULL;
    struct queue_t *running_list = krnl->running_list;
    
    /* Find the process with matching PID in running list */
    if (running_list != NULL) {
        struct qnode_t *qnode = running_list->head;
        while (qnode != NULL) {
            struct pcb_t *proc = (struct pcb_t *)qnode->data;
            if (proc != NULL && proc->pid == pid) {
                caller = proc;
                break;
            }
            qnode = qnode->next;
        }
    }
    
    /* Check if process found */
    if (caller == NULL)
        return -1;
    
    /* Call inc_vma_limit */
    int ret = inc_vma_limit(caller, vmaid, inc_sz);
    
    /* Store result in return register */
    regs->a3 = ret;
    
    return ret;
}
```

---

### **Bước 3.2: Tạo `src/sys_memswp.c`**

**Tạo file mới với nội dung:**

```c
/*
 * Copyright (C) 2026 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* LamiaAtrium release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

#include "syscall.h"
#include "common.h"
#include "os-mm.h"
#include "mm.h"
#include <stdlib.h>

int __sys_memswp(struct krnl_t *krnl, uint32_t pid, struct sc_regs* regs)
{
    /* Extract syscall parameters */
    addr_t vicfpn = regs->a1;   /* Victim Frame Page Number */
    addr_t swpfpn = regs->a2;   /* Swap Frame Page Number */
    
    /* TODO: Traverse proclist to find matching process by PID */
    struct pcb_t *caller = NULL;
    struct queue_t *running_list = krnl->running_list;
    
    /* Find the process with matching PID in running list */
    if (running_list != NULL) {
        struct qnode_t *qnode = running_list->head;
        while (qnode != NULL) {
            struct pcb_t *proc = (struct pcb_t *)qnode->data;
            if (proc != NULL && proc->pid == pid) {
                caller = proc;
                break;
            }
            qnode = qnode->next;
        }
    }
    
    /* Check if process found */
    if (caller == NULL)
        return -1;
    
    /* Call memory swap function */
    int ret = __mm_swap_page(caller, vicfpn, swpfpn);
    
    /* Store result in return register */
    regs->a3 = ret;
    
    return ret;
}
```

---

### **Bước 3.3: Tạo `src/sys_memmap.c` (nếu cần)**

**Note:** File này đã tồn tại, nhưng có thể cần sửa đổi. Tạo version hoàn thiện:

```c
/*
 * Copyright (C) 2026 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* LamiaAtrium release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

#include "os-mm.h"
#include "syscall.h"
#include "libmem.h"
#include "queue.h"
#include "mm.h"
#include <stdlib.h>
#include <stdio.h>

int __sys_memmap(struct krnl_t *krnl, uint32_t pid, struct sc_regs* regs)
{
    int memop = regs->a1;
    BYTE value;
    
    /* Traverse proclist to find matching process */
    struct pcb_t *caller = NULL;
    struct queue_t *running_list = krnl->running_list;
    
    /* Find the process with matching PID */
    if (running_list != NULL) {
        struct qnode_t *qnode = running_list->head;
        while (qnode != NULL) {
            struct pcb_t *proc = (struct pcb_t *)qnode->data;
            if (proc != NULL && proc->pid == pid) {
                caller = proc;
                break;
            }
            qnode = qnode->next;
        }
    }
    
    /* Check if process found */
    if (caller == NULL)
        return -1;
    
    /* Process memory operation */
    switch (memop) {
    case SYSMEM_MAP_OP:
        vmap_pgd_memset(caller, regs->a2, regs->a3);
        break;
    case SYSMEM_INC_OP:
        inc_vma_limit(caller, regs->a2, regs->a3);
        break;
    case SYSMEM_SWP_OP:
        __mm_swap_page(caller, regs->a2, regs->a3);
        break;
    case SYSMEM_IO_READ:
        MEMPHY_read(caller->krnl->mram, regs->a2, &value);
        regs->a3 = value;
        break;
    case SYSMEM_IO_WRITE:
        MEMPHY_write(caller->krnl->mram, regs->a2, regs->a3);
        break;
    default:
        printf("[ERROR] Unknown memop code: %d\n", memop);
        return -1;
    }
    
    return 0;
}
```

---

## Phần 4: Cập nhật Syscall Table (10 phút)

### **Bước 4.1: Cập nhật `src/syscall.tbl`**

**File:** `src/syscall.tbl`

**Thay thế từ:**
```plaintext
0       listsyscall sys_listsyscall
17      memmap      sys_memmap
```

**Thành:**
```plaintext
0       listsyscall sys_listsyscall
17      memmap      sys_memmap
18      meminc      sys_meminc
19      memswp      sys_memswp
```

---

### **Bước 4.2: Cập nhật Makefile (nếu cần)**

**File:** `Makefile`

Kiểm tra xem `SYSCALL_OBJ` có bao gồm syscall files mới không:

```makefile
SYSCALL_OBJ = $(addprefix $(OBJ)/, syscall.o sys_mem.o sys_meminc.o sys_memswp.o sys_listsyscall.o)
```

Nếu chưa có, thêm `sys_meminc.o sys_memswp.o` vào danh sách.

---

## Phần 5: Compile & Test (20 phút)

### **Bước 5.1: Clean Build**

```bash
cd c:\Users\my\ pc\OneDrive\Máy\ tính\Study\HK251_BK\Hệ\ điều\ hành\Assignment\Test\ossim_lamiaatrium
make clean
make
```

**Kỳ vọng:**
- ✅ Không có compile errors
- ✅ Có thể có warnings nhưng không critical

---

### **Bước 5.2: Kiểm tra Output Files**

Kiểm tra xem binary `os` có được tạo:

```bash
ls -la os
```

Nên thấy một executable file `os` được tạo vào lúc gần đây.

---

### **Bước 5.3: Test với Input Files**

```bash
# Test với scheduler
./os < input/sched

# Test với memory paging
./os < input/os_1_mlq_paging

# Test với syscalls
./os < input/os_syscall
```

**Kiểm tra:**
- ✅ Program chạy không bị segfault
- ✅ Output có content hợp lý
- ✅ Không có lỗi runtime

---

## Phần 6: Verification Checklist

### **Kiểm tra macro đã fix:**

```c
// Test OVERLAP
assert(OVERLAP(0, 10, 5, 15) == 1);      // Overlap: [0,10) vs [5,15)
assert(OVERLAP(0, 10, 10, 20) == 0);     // No overlap: [0,10) vs [10,20)
assert(OVERLAP(5, 15, 0, 10) == 1);      // Overlap: [5,15) vs [0,10)

// Test INCLUDE
assert(INCLUDE(0, 20, 5, 15) == 1);      // [0,20) includes [5,15)
assert(INCLUDE(0, 10, 5, 15) == 0);      // [0,10) doesn't include [5,15)
```

### **Kiểm tra file tạo:**

```bash
ls -la src/sys_*.c
# Should include:
# - sys_mem.c         ✓ (updated)
# - sys_meminc.c      ✓ (new)
# - sys_memswp.c      ✓ (new)
# - sys_listsyscall.c ✓ (existing)
```

### **Kiểm tra Makefile:**

```bash
grep "sys_mem" Makefile
# Phải chứa: sys_mem.o sys_meminc.o sys_memswp.o
```

---

## 🎯 Troubleshooting

### **Lỗi 1: "undefined reference to `inc_vma_limit`"**

**Nguyên nhân:** Function chưa được implement hoặc declared

**Giải pháp:**
- Kiểm tra `src/mm-vm.c` có implement `inc_vma_limit()` không
- Kiểm tra `include/mm.h` có declare không

### **Lỗi 2: "OVERLAP macro không hoạt động"**

**Nguyên nhân:** Macro chưa được fix

**Giải pháp:**
- Mở `include/mm.h`
- Kiểm tra OVERLAP macro định nghĩa
- Nếu vẫn là `(0)` → fix theo Bước 1.2

### **Lỗi 3: "Process không được tìm thấy"**

**Nguyên nhân:** PID lookup logic có vấn đề

**Giải pháp:**
- Kiểm tra queue structure
- Thêm debug prints để theo dõi traversal
- Xác nhận process tồn tại trong running_list

### **Lỗi 4: "Segmentation fault"**

**Nguyên nhân:** Null pointer dereference

**Giải pháp:**
- Thêm NULL checks trước mỗi pointer dereference
- Kiểm tra `caller != NULL` trước sử dụng
- Kiểm tra `cur_vma != NULL` trước sử dụng

---

## 📝 Summary

| Step | Task | Time | Status |
|------|------|------|--------|
| 1 | Fix OVERLAP macro | 5 min | ⬜ |
| 2.1 | get_vm_area_node_at_brk() | 15 min | ⬜ |
| 2.3 | inc_vma_limit() | 15 min | ⬜ |
| 3.1 | Create sys_meminc.c | 10 min | ⬜ |
| 3.2 | Create sys_memswp.c | 10 min | ⬜ |
| 4 | Update syscall.tbl | 5 min | ⬜ |
| 5 | Compile & Test | 20 min | ⬜ |

**Total: ~2 hours**

---

**Good luck! 🍀**
