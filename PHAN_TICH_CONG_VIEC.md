# Phân Tích Công Việc: Virtual Memory & System Calls (Thành viên 4 - Dương)

**Ngày phân tích:** 17/11/2025  
**Vai trò:** Bộ nhớ Ảo (Virtual Memory) & System Calls

---

## 📋 MỤC LỤC
1. [Tóm tắt công việc](#tóm-tắt-công-việc)
2. [File cần hoàn thiện](#file-cần-hoàn-thiện)
3. [Phân tích chi tiết từng file](#phân-tích-chi-tiết-từng-file)
4. [Hướng dẫn implement](#hướng-dẫn-implement)
5. [Các bước tiến hành](#các-bước-tiến-hành)

---

## 🎯 Tóm tắt công việc

**Thành viên 4 (Dương) phụ trách:**
- 📌 **File chính:** `mm-vm.c` - Quản lý bộ nhớ ảo (Virtual Memory)
- 📌 **Syscall handlers mới:** Tạo các file `sys_xxxhandler.c` cho các system calls liên quan đến memory mapping

**Kiến trúc hệ thống:**
- **Loại memory:** Paging-based (256B pages)
- **CPU Bus:** 22-bit (MAX 4MB)
- **Virtual Address Management:** Multi-level paging (PGD, P4D, PUD, PMD, PT)
- **Swap Support:** Support cho memory swap (PAGING_MEMSWPSZ = 512MB)

---

## 📁 File cần hoàn thiện

### **🔴 CHÍNH (Ưu tiên cao)**

| # | File | Trạng thái | Mô tả | Ghi chú |
|---|------|-----------|-------|---------|
| 1 | `src/mm-vm.c` | ❌ Cần hoàn thiện | Virtual Memory quản lý | Có 6 TODO cần implement |
| 2 | `src/sys_memmap.c` | ❌ Chưa tạo | Syscall handler cho memory map | Cần tạo mới |
| 3 | `src/sys_meminc.c` | ❌ Chưa tạo | Syscall handler cho memory increment | Cần tạo mới |
| 4 | `src/sys_memswp.c` | ❌ Chưa tạo | Syscall handler cho memory swap | Cần tạo mới |

### **🟡 HỖ TRỢ (Ưu tiên trung bình)**

| # | File | Trạng thái | Mô tả |
|---|------|-----------|-------|
| 5 | `src/sys_mem.c` | 🟡 Cần cập nhật | Chính handler, cần thêm cases |
| 6 | `src/syscall.tbl` | 🟡 Cần cập nhật | Syscall table, thêm syscalls mới |
| 7 | `include/mm.h` | 🟡 Kiểm tra | Header definitions |

---

## 🔍 Phân tích chi tiết từng file

### **1️⃣ File: `src/mm-vm.c`** (Virtual Memory Management)

#### 📊 Trạng thái hiện tại:
- ✅ Một số function đã implement:
  - `get_vma_by_num()` - Lấy VM area theo ID
  - `__mm_swap_page()` - Swap page giữa RAM và swap space
  - `validate_overlap_vm_area()` - Kiểm tra overlap

- ❌ Một số function cần hoàn thiện (có TODO):

#### **TODO #1: `get_vm_area_node_at_brk()` - Lấy VM area node tại break point**

```c
struct vm_rg_struct *get_vm_area_node_at_brk(struct pcb_t *caller, 
                                             int vmaid, 
                                             addr_t size, 
                                             addr_t alignedsz)
```

**Vấn đề hiện tại:**
- ❓ Đã allocate memory cho `newrg` nhưng cần xác định rõ boundaries
- ❓ Cần lấy `sbrk` từ VMA hiện tại

**Cần làm:**
- ✅ Lấy current VMA dựa trên vmaid
- ✅ Set `rg_start = cur_vma->sbrk` (current break point)
- ✅ Set `rg_end = rg_start + alignedsz` (aligned size)
- ✅ Update `cur_vma->sbrk = rg_end` (update break point)
- ✅ Return newrg

**Cấu trúc liên quan:**
```c
struct vm_area_struct {
    unsigned long vm_id;      // VM area ID
    addr_t vm_start;          // Start address
    addr_t vm_end;            // End address
    addr_t sbrk;              // Current break point
    struct vm_rg_struct *vm_freerg_list;  // Free regions list
};
```

---

#### **TODO #2: `validate_overlap_vm_area()` - Kiểm tra overlap VM areas**

```c
int validate_overlap_vm_area(struct pcb_t *caller, 
                            int vmaid, 
                            addr_t vmastart, 
                            addr_t vmaend)
```

**Vấn đề hiện tại:**
- ❌ Macro `OVERLAP()` chưa implement (trả về 0 - luôn false)
- ⚠️ Logic kiểm tra cần được hoàn thiện

**Cần làm:**
1. **Kiểm tra overlap macro định nghĩa:**
   - File `include/mm.h` dòng 72-73:
     ```c
     #define OVERLAP(x1,x2,y1,y2) (0)  // ❌ Chưa implement
     ```
   
2. **Implement macro OVERLAP:**
   - Kiểm tra nếu hai range `[x1,x2)` và `[y1,y2)` có giao nhau
   - Công thức: `(x1 < y2) && (y1 < x2)`
   
3. **Fix trong `mm.h`:**
   ```c
   #define OVERLAP(x1,x2,y1,y2) ((x1 < y2) && (y1 < x2))
   ```

4. **Implement INCLUDE macro (cũng cần):**
   ```c
   #define INCLUDE(x1,x2,y1,y2) ((x1 <= y1) && (y2 <= x2))
   ```

---

#### **TODO #3: `inc_vma_limit()` - Tăng giới hạn VM area**

```c
int inc_vma_limit(struct pcb_t *caller, 
                 int vmaid, 
                 addr_t inc_sz)
```

**Vấn đề hiện tại:**
- ❌ Toàn bộ function bị comment out
- ⚠️ Cần implement logic tăng VMA limit

**Cần làm:**
1. **Align size với page size:**
   ```c
   addr_t inc_amt = PAGING_PAGE_ALIGNSZ(inc_sz);
   int incnumpage = inc_amt / PAGING_PAGESZ;
   ```

2. **Lấy current VMA:**
   ```c
   struct vm_area_struct *cur_vma = get_vma_by_num(caller->krnl->mm, vmaid);
   ```

3. **Kiểm tra overlap:**
   ```c
   addr_t new_end = cur_vma->vm_end + inc_amt;
   if (validate_overlap_vm_area(caller, vmaid, cur_vma->vm_end, new_end) < 0)
       return -1;
   ```

4. **Map memory to RAM:**
   ```c
   struct vm_rg_struct newrg;
   newrg.rg_start = cur_vma->vm_end;
   newrg.rg_end = new_end;
   
   if (vm_map_ram(caller, newrg.rg_start, newrg.rg_end, 
                  cur_vma->vm_end, incnumpage, &newrg) < 0)
       return -1;
   ```

5. **Update VMA:**
   ```c
   cur_vma->vm_end = new_end;
   ```

---

### **2️⃣ File: `src/sys_mem.c`** (Memory Syscall Handler - CẦN CẬP NHẬT)

#### 📊 Trạng thái hiện tại:
```c
int __sys_memmap(struct krnl_t *krnl, uint32_t pid, struct sc_regs* regs)
```

**Vấn đề:**
- ❌ Hàm tạm caller = malloc (DUMMY) - cần fix để lấy process từ queue
- ❌ Cần traverse process list để tìm process match với pid

**Memop codes hiện hỗ trợ:**
```c
case SYSMEM_MAP_OP:     // vmap_pgd_memset
case SYSMEM_INC_OP:     // inc_vma_limit
case SYSMEM_SWP_OP:     // __mm_swap_page
case SYSMEM_IO_READ:    // MEMPHY_read
case SYSMEM_IO_WRITE:   // MEMPHY_write
```

**Cần làm:**
1. Tìm process từ pid trong running_list hoặc ready_queue
2. Lấy caller = process tương ứng
3. Xử lý từng trường hợp memop tương ứng

---

### **3️⃣ Syscall Table: `src/syscall.tbl`**

#### 📊 Trạng thái hiện tại:
```plaintext
0       listsyscall sys_listsyscall
17      memmap      sys_memmap
```

**Cần thêm syscalls mới (theo hướng dẫn 1.6.3):**

Các syscall cần thêm vào:
```plaintext
0       listsyscall     sys_listsyscall         ✅ Có rồi
17      memmap          sys_memmap              ✅ Có rồi
18      meminc          sys_meminc              ❌ CẦN THÊM
19      memswp          sys_memswp              ❌ CẦN THÊM
20      memread         sys_memread             ❌ CẦN THÊM (optional)
21      memwrite        sys_memwrite            ❌ CẦN THÊM (optional)
```

---

## 🚀 Hướng dẫn implement

### **BƯỚC 1: Fix macro OVERLAP trong `include/mm.h`**

```c
// Thay thế dòng 72-73:
// #define OVERLAP(x1,x2,y1,y2) (0)
// #define INCLUDE(x1,x2,y1,y2) (0)

// Bằng:
#define OVERLAP(x1,x2,y1,y2) ((x1 < y2) && (y1 < x2))
#define INCLUDE(x1,x2,y1,y2) ((x1 <= y1) && (y2 <= x2))
```

---

### **BƯỚC 2: Hoàn thiện `src/mm-vm.c`**

#### **2.1 Complete `get_vm_area_node_at_brk()`**

```c
struct vm_rg_struct *get_vm_area_node_at_brk(struct pcb_t *caller, 
                                             int vmaid, 
                                             addr_t size, 
                                             addr_t alignedsz)
{
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->krnl->mm, vmaid);
  if (cur_vma == NULL)
    return NULL;

  struct vm_rg_struct *newrg = malloc(sizeof(struct vm_rg_struct));
  if (newrg == NULL)
    return NULL;

  newrg->rg_start = cur_vma->sbrk;
  newrg->rg_end = newrg->rg_start + alignedsz;
  newrg->rg_next = NULL;

  /* Update the break point */
  cur_vma->sbrk = newrg->rg_end;

  return newrg;
}
```

#### **2.2 Fix `validate_overlap_vm_area()` - Không cần thay đổi nếu macro được fix**

Khi OVERLAP macro được implement đúng, hàm sẽ hoạt động.

#### **2.3 Complete `inc_vma_limit()`**

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

  /* Calculate new end */
  addr_t new_end = cur_vma->vm_end + inc_amt;

  /* Validate overlap */
  if (validate_overlap_vm_area(caller, vmaid, cur_vma->vm_end, new_end) < 0)
    return -1;

  /* Create new region */
  struct vm_rg_struct newrg;
  newrg.rg_start = cur_vma->vm_end;
  newrg.rg_end = new_end;
  newrg.rg_next = NULL;

  /* Map memory to RAM */
  if (vm_map_ram(caller, newrg.rg_start, newrg.rg_end, 
                 cur_vma->vm_end, incnumpage, &newrg) < 0)
    return -1;

  /* Update VMA end */
  cur_vma->vm_end = new_end;

  return 0;
}
```

---

### **BƯỚC 3: Tạo Syscall Handlers mới**

#### **3.1 Tạo `src/sys_meminc.c` - Handler cho MEMINC syscall**

```c
/*
 * Copyright (C) 2026 pdnguyen of HCMC University of Technology VNU-HCM
 */

#include "syscall.h"
#include "common.h"
#include "mm.h"

int __sys_meminc(struct krnl_t *krnl, uint32_t pid, struct sc_regs* regs)
{
    /* Lấy process từ pid */
    struct pcb_t *caller = NULL;
    
    /* TODO: Traverse proclist to find matching process */
    struct queue_t *running_list = krnl->running_list;
    
    /* Gọi inc_vma_limit */
    if (caller == NULL)
        return -1;
    
    int vmaid = regs->a1;
    addr_t inc_sz = regs->a2;
    
    int ret = inc_vma_limit(caller, vmaid, inc_sz);
    regs->a3 = ret;  /* Return result */
    
    return ret;
}
```

#### **3.2 Tạo `src/sys_memswp.c` - Handler cho MEMSWP syscall**

```c
/*
 * Copyright (C) 2026 pdnguyen of HCMC University of Technology VNU-HCM
 */

#include "syscall.h"
#include "common.h"
#include "mm.h"

int __sys_memswp(struct krnl_t *krnl, uint32_t pid, struct sc_regs* regs)
{
    /* Lấy process từ pid */
    struct pcb_t *caller = NULL;
    
    /* TODO: Traverse proclist to find matching process */
    struct queue_t *running_list = krnl->running_list;
    
    if (caller == NULL)
        return -1;
    
    addr_t vicfpn = regs->a1;   /* Victim frame page number */
    addr_t swpfpn = regs->a2;   /* Swap frame page number */
    
    int ret = __mm_swap_page(caller, vicfpn, swpfpn);
    regs->a3 = ret;
    
    return ret;
}
```

---

### **BƯỚC 4: Cập nhật Syscall Table**

#### **4.1 Update `src/syscall.tbl`**

```plaintext
0       listsyscall sys_listsyscall
17      memmap      sys_memmap
18      meminc      sys_meminc
19      memswp      sys_memswp
```

---

### **BƯỚC 5: Cập nhật `src/sys_mem.c`**

```c
int __sys_memmap(struct krnl_t *krnl, uint32_t pid, struct sc_regs* regs)
{
    int memop = regs->a1;
    BYTE value;
    
    /* TODO: Traverse proclist to find matching process */
    struct pcb_t *caller = NULL;
    struct queue_t *running_list = krnl->running_list;
    
    /* TODO: Traverse and find process by PID */
    /*
    struct qnode_t *qnode = running_list->head;
    while (qnode != NULL) {
        struct pcb_t *proc = (struct pcb_t *)qnode->data;
        if (proc->pid == pid) {
            caller = proc;
            break;
        }
        qnode = qnode->next;
    }
    */
    
    if (caller == NULL)
        return -1;
    
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
        printf("Unknown memop code: %d\n", memop);
        return -1;
    }
    
    return 0;
}
```

---

## 📝 Các bước tiến hành

### **Thứ tự ưu tiên:**

```
1. FIX MACRO (PHẢI LÀM TRƯỚC)
   ✅ Fix OVERLAP() macro trong include/mm.h
   ✅ Implement INCLUDE() macro
   
2. HOÀN THIỆN MM-VM.C (CỐT LỖI)
   ✅ get_vm_area_node_at_brk()
   ✅ validate_overlap_vm_area() (sẽ hoạt động sau khi fix macro)
   ✅ inc_vma_limit()
   
3. TẠO SYSCALL HANDLERS MỚI
   ✅ src/sys_meminc.c
   ✅ src/sys_memswp.c
   
4. CẬP NHẬT SYSCALL TABLE
   ✅ src/syscall.tbl (thêm syscalls mới)
   ✅ src/sys_mem.c (fix process finding logic)
   
5. COMPILE & TEST
   ✅ make clean
   ✅ make
   ✅ Test với input files
```

---

## 📊 Tóm tắt công việc

| STT | Task | Độ phức tạp | Ưu tiên | Estimated time |
|-----|------|-----------|---------|----------------|
| 1 | Fix OVERLAP macro | ⭐ Rất dễ | 🔴 Cao | 5 min |
| 2 | Implement get_vm_area_node_at_brk() | ⭐⭐ Dễ | 🔴 Cao | 15 min |
| 3 | Fix validate_overlap_vm_area() | ⭐ Rất dễ | 🔴 Cao | 5 min |
| 4 | Complete inc_vma_limit() | ⭐⭐⭐ Trung bình | 🔴 Cao | 30 min |
| 5 | Create sys_meminc.c | ⭐⭐ Dễ | 🟡 Trung | 15 min |
| 6 | Create sys_memswp.c | ⭐⭐ Dễ | 🟡 Trung | 15 min |
| 7 | Update syscall.tbl | ⭐ Rất dễ | 🟡 Trung | 5 min |
| 8 | Update sys_mem.c | ⭐⭐ Dễ | 🟡 Trung | 20 min |
| 9 | Compile & Test | ⭐⭐⭐ Trung bình | 🔴 Cao | 30 min |

**Total estimated time: ~2-2.5 hours**

---

## 🔗 Tài liệu tham khảo

### Các file quan trọng:
- `include/mm.h` - Định nghĩa macro và structures
- `include/os-mm.h` - OS memory structures
- `include/syscall.h` - Syscall definitions
- `src/mm-vm.c` - Virtual memory implementation
- `src/sys_mem.c` - Memory syscall handlers
- `src/syscall.tbl` - Syscall table

### Các macro/constant quan trọng:
```c
#define PAGING_PAGESZ 256           /* Page size: 256B */
#define PAGING_PAGE_ALIGNSZ(sz)     /* Align to page size */
#define PAGING_MEMRAMSZ BIT(21)     /* RAM: 2MB */
#define PAGING_MEMSWPSZ BIT(29)     /* Swap: 512MB */
#define PAGING_MAX_PGN              /* Max page number */
```

### Các function quan trọng:
```c
int vm_map_ram()                    /* Map VM to RAM */
int vmap_pgd_memset()               /* Map range of pages */
int __swap_cp_page()                /* Swap page */
int MEMPHY_read/write()             /* Physical memory I/O */
```

---

## ✅ Checklist hoàn thiện

- [ ] Fix OVERLAP macro
- [ ] Implement INCLUDE macro
- [ ] Complete get_vm_area_node_at_brk()
- [ ] Verify validate_overlap_vm_area()
- [ ] Complete inc_vma_limit()
- [ ] Create sys_meminc.c
- [ ] Create sys_memswp.c
- [ ] Update syscall.tbl
- [ ] Update sys_mem.c
- [ ] Compile successfully
- [ ] Test with input files
- [ ] Submit

---

**Được tạo bởi:** AI Assistant  
**Version:** 1.0  
**Last Updated:** 17/11/2025
