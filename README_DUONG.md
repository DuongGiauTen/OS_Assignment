# 📌 TÓM TẮT CÔNG VIỆC - DƯƠNG (THÀNH VIÊN 4)

**Ngày phân tích:** 17/11/2025  
**Trạng thái:** ✅ Phân tích hoàn tất  
**Số file cần làm:** 7 file  
**Độ phức tạp:** ⭐⭐⭐ Trung bình  
**Ước tính thời gian:** 2-2.5 giờ

---

## 🎯 Tóm tắt công việc của bạn

**Vai trò:** Bộ nhớ Ảo (Virtual Memory) & System Calls

**Chịu trách nhiệm chính:**
1. ✅ **mm-vm.c** - Quản lý bộ nhớ ảo (hoàn thiện 3 functions)
2. ✅ **sys_meminc.c** - Syscall handler mới
3. ✅ **sys_memswp.c** - Syscall handler mới
4. ✅ **Cập nhật** sys_mem.c, syscall.tbl, include/mm.h

---

## 📂 Danh sách file cần làm

### **🔴 PHẢI LÀM (Critical)**

| # | File | Vị trí | Công việc | Ưu tiên |
|----|------|--------|----------|---------|
| 1 | **mm.h** | `include/` | Fix OVERLAP/INCLUDE macro | 🔴 PHẢI TRƯỚC |
| 2 | **mm-vm.c** | `src/` | Hoàn thiện 3 functions | 🔴 CỐT LỖI |
| 3 | **sys_meminc.c** | `src/` | TẠO MỚI | 🔴 CỐT LỖI |
| 4 | **sys_memswp.c** | `src/` | TẠO MỚI | 🔴 CỐT LỖI |
| 5 | **syscall.tbl** | `src/` | Thêm 2 syscall mới | 🟡 SAU |
| 6 | **sys_mem.c** | `src/` | Cập nhật handler | 🟡 SAU |
| 7 | **Makefile** | root | Cập nhật (nếu cần) | 🟡 SAU |

---

## 🔧 Chi tiết công việc từng file

### **1️⃣ include/mm.h - Fix Macro**

**Vị trí:** Dòng ~72-73

**Hiện tại:**
```c
#define OVERLAP(x1,x2,y1,y2) (0)       ❌ Sai
#define INCLUDE(x1,x2,y1,y2) (0)       ❌ Sai
```

**Cần sửa thành:**
```c
#define OVERLAP(x1,x2,y1,y2) ((x1 < y2) && (y1 < x2))
#define INCLUDE(x1,x2,y1,y2) ((x1 <= y1) && (y2 <= x2))
```

**Thời gian:** 5 phút

---

### **2️⃣ src/mm-vm.c - Hoàn thiện 3 functions**

#### **2.1 get_vm_area_node_at_brk() - Dòng 58-81**

**Vấn đề:**
- Cần xác định boundary của region tại break point
- Cần update break point sau khi allocate

**Cần làm:**
```c
// 1. Get current VMA by vmaid
struct vm_area_struct *cur_vma = get_vma_by_num(caller->krnl->mm, vmaid);

// 2. Allocate new region
struct vm_rg_struct *newrg = malloc(...);

// 3. Set boundaries (rg_start = sbrk, rg_end = sbrk + size)
newrg->rg_start = cur_vma->sbrk;
newrg->rg_end = newrg->rg_start + alignedsz;

// 4. Update break point
cur_vma->sbrk = newrg->rg_end;

// 5. Return newrg
return newrg;
```

**Thời gian:** 15 phút

---

#### **2.2 validate_overlap_vm_area() - Dòng 84-125**

**Vấn đề:** OVERLAP macro chưa implement

**Cần làm:** Không cần sửa nội bộ, chỉ cần fix macro ở bước 1

**Kiểm tra:** Function sẽ kiểm tra xem có overlap với VMAs khác không

**Thời gian:** 5 phút (tự động hoạt động sau fix macro)

---

#### **2.3 inc_vma_limit() - Dòng 128-157**

**Vấn đề:** 
- Toàn bộ logic bị comment out
- Cần implement từ đầu

**Cần làm:**
```c
// 1. Align size
addr_t inc_amt = PAGING_PAGE_ALIGNSZ(inc_sz);

// 2. Get current VMA
struct vm_area_struct *cur_vma = get_vma_by_num(caller->krnl->mm, vmaid);

// 3. Calculate new boundary
addr_t new_end = cur_vma->vm_end + inc_amt;

// 4. Check overlap
validate_overlap_vm_area(caller, vmaid, cur_vma->vm_end, new_end);

// 5. Map to RAM
vm_map_ram(caller, old_end, new_end, old_end, incnumpage, &newrg);

// 6. Update VMA
cur_vma->vm_end = new_end;

// 7. Return 0
return 0;
```

**Thời gian:** 30 phút

---

### **3️⃣ src/sys_meminc.c - Tạo file mới**

**Mục đích:** Syscall handler cho MEMINC syscall

**Nội dung cơ bản:**
```c
int __sys_meminc(struct krnl_t *krnl, uint32_t pid, struct sc_regs* regs)
{
    // 1. Find process by PID từ running_list
    // 2. Call inc_vma_limit(caller, regs->a1, regs->a2)
    // 3. Return result
}
```

**Thời gian:** 15 phút

---

### **4️⃣ src/sys_memswp.c - Tạo file mới**

**Mục đích:** Syscall handler cho MEMSWP syscall

**Nội dung cơ bản:**
```c
int __sys_memswp(struct krnl_t *krnl, uint32_t pid, struct sc_regs* regs)
{
    // 1. Find process by PID từ running_list
    // 2. Call __mm_swap_page(caller, regs->a1, regs->a2)
    // 3. Return result
}
```

**Thời gian:** 15 phút

---

### **5️⃣ src/syscall.tbl - Cập nhật Syscall Table**

**Hiện tại:**
```plaintext
0       listsyscall sys_listsyscall
17      memmap      sys_memmap
```

**Cần thêm:**
```plaintext
18      meminc      sys_meminc
19      memswp      sys_memswp
```

**Thời gian:** 5 phút

---

### **6️⃣ src/sys_mem.c - Cập nhật Handler**

**Vấn đề:**
- Hiện đang tạo dummy caller = malloc
- Cần traverse running_list để tìm process

**Cần làm:**
```c
// Traverse running_list để tìm process có pid match
struct pcb_t *caller = NULL;
struct queue_t *running_list = krnl->running_list;
if (running_list != NULL) {
    struct qnode_t *qnode = running_list->head;
    while (qnode != NULL) {
        struct pcb_t *proc = (struct pcb_t *)qnode->data;
        if (proc->pid == pid) {
            caller = proc;
            break;
        }
        qnode = qnode->next;
    }
}
```

**Thời gian:** 20 phút

---

### **7️⃣ Makefile - Cập nhật (nếu cần)**

**Kiểm tra:** 
- Xem `SYSCALL_OBJ` có bao gồm `sys_meminc.o sys_memswp.o` chưa

**Cần làm (nếu chưa có):**
```makefile
SYSCALL_OBJ = $(addprefix $(OBJ)/, syscall.o sys_mem.o sys_meminc.o sys_memswp.o sys_listsyscall.o)
```

**Thời gian:** 5 phút

---

## 📊 Timeline hoàn thành

```
Phút 0-5:     Fix macro trong mm.h ✓
Phút 5-20:    get_vm_area_node_at_brk() ✓
Phút 20-25:   validate_overlap_vm_area() ✓ (tự động)
Phút 25-55:   inc_vma_limit() ✓
Phút 55-70:   sys_meminc.c ✓
Phút 70-85:   sys_memswp.c ✓
Phút 85-90:   syscall.tbl ✓
Phút 90-110:  sys_mem.c ✓
Phút 110-140: Compile & Test ✓
────────────────────────────
Total: ~130 phút = 2h 10 min
```

---

## 🎓 Các khái niệm cần hiểu

### **Virtual Memory Area (VMA)**
- Là một vùng bộ nhớ ảo của process
- Có start, end, và break point (sbrk)
- Một process có thể có nhiều VMA

### **Memory Region (VM Region)**
- Là một đoạn nhỏ được allocate từ VMA
- Có start và end address
- Nhiều region có thể tồn tại trong một VMA

### **Syscall Handler**
- Là hàm kernel được gọi khi user program thực hiện syscall
- Có tên format: `__sys_xxxx()`
- Nhận tham số từ `struct sc_regs`

### **Page Alignment**
- Kích thước page = 256 bytes
- Bất kỳ allocation nào cũng phải align tới múltiple của 256 bytes
- Ví dụ: 0x150 bytes → align thành 0x200 bytes

---

## ✅ Checklist trước khi submit

- [ ] Fix OVERLAP & INCLUDE macro
- [ ] get_vm_area_node_at_brk() implement đúng
- [ ] validate_overlap_vm_area() hoạt động (sau fix macro)
- [ ] inc_vma_limit() implement hoàn chỉnh
- [ ] sys_meminc.c tạo mới
- [ ] sys_memswp.c tạo mới
- [ ] syscall.tbl cập nhật
- [ ] sys_mem.c cập nhật
- [ ] Makefile cập nhật (nếu cần)
- [ ] `make clean && make` chạy thành công
- [ ] Không có compile errors
- [ ] Test với input files không bị segfault
- [ ] Output files có content hợp lý

---

## 🆘 Các vấn đề thường gặp

### **Lỗi 1: undefined reference to functions**
→ Kiểm tra xem functions đã được declare trong .h file không

### **Lỗi 2: OVERLAP macro không hoạt động đúng**
→ Kiểm tra file include/mm.h, đảm bảo đã fix macro

### **Lỗi 3: Process không được tìm thấy**
→ Thêm NULL checks, debug prints để xem queue structure

### **Lỗi 4: Segmentation fault**
→ Kiểm tra NULL pointers trước mỗi dereference

---

## 📚 Tài liệu tham khảo trong workspace

| File | Nội dung |
|------|----------|
| PHAN_TICH_CONG_VIEC.md | Phân tích chi tiết từng file |
| CACH_THAM_KHAO.md | Giải thích structures và macros |
| HUONG_DAN_STEP_BY_STEP.md | Hướng dẫn chi tiết từng bước |
| include/mm.h | Macro definitions |
| include/os-mm.h | Struct definitions |
| src/mm-vm.c | Virtual memory implementation |
| src/sys_mem.c | Memory syscall handler |

---

## 🚀 Bắt đầu làm việc

**Bước 1:** Mở file `HUONG_DAN_STEP_BY_STEP.md` để follow chi tiết

**Bước 2:** Làm theo thứ tự:
1. Fix macro (5 min)
2. Hoàn thiện mm-vm.c (45 min)
3. Tạo syscall handlers (30 min)
4. Cập nhật files (20 min)
5. Compile & Test (20 min)

**Bước 3:** Khi gặp khó khăn:
- Xem `CACH_THAM_KHAO.md` để hiểu structures
- Xem `PHAN_TICH_CONG_VIEC.md` để hiểu chi tiết

**Good luck! 🍀 Chúc bạn thành công!**

---

**Version:** 1.0  
**Last Updated:** 17/11/2025  
**Created by:** AI Assistant for Dương (Team Member 4)
