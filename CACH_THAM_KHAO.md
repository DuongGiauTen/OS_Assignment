# Hướng dẫn Chi Tiết: Cách Triển khai Virtual Memory & System Calls

## 📚 Bảng tham khảo Structures

### **Cấu trúc VM Area (Virtual Memory Area)**

```c
struct vm_area_struct {
    unsigned long vm_id;              // ID của VMA (0, 1, 2, 3...)
    addr_t vm_start;                  // Địa chỉ ảo bắt đầu của vùng
    addr_t vm_end;                    // Địa chỉ ảo kết thúc của vùng
    addr_t sbrk;                      // Current break point (tương tự malloc brk)
    struct mm_struct *vm_mm;          // Pointer tới mm_struct của process
    struct vm_rg_struct *vm_freerg_list;  // Danh sách các vùng free trong VMA này
    struct vm_area_struct *vm_next;   // Pointer tới VMA tiếp theo
};
```

**Ví dụ:**
```
Process có 3 VMA:
VMA[0]: vm_start=0x0,    vm_end=0x1000, sbrk=0x800    (Code - 4KB)
VMA[1]: vm_start=0x1000, vm_end=0x2000, sbrk=0x1500   (Heap - 4KB)
VMA[2]: vm_start=0x2000, vm_end=0x4000, sbrk=0x2000   (Stack - 8KB)
```

---

### **Cấu trúc Memory Region (VM Region)**

```c
struct vm_rg_struct {
    addr_t rg_start;                  // Địa chỉ bắt đầu của region
    addr_t rg_end;                    // Địa chỉ kết thúc của region
    struct vm_rg_struct *rg_next;     // Pointer tới region tiếp theo
};
```

**Ví dụ:** Region represent một đoạn memory được allocate
```
Region 1: rg_start=0x1000, rg_end=0x1100 (256 bytes)
Region 2: rg_start=0x1200, rg_end=0x1400 (512 bytes)
```

---

### **Cấu trúc Memory Management (MM)**

```c
struct mm_struct {
    uint32_t *pgd;                    // Page Global Directory
    struct vm_area_struct *mmap;      // Linked list các VMA
    struct vm_rg_struct symrgtbl[PAGING_MAX_SYMTBL_SZ];  // Symbol regions table
    struct pgn_t *fifo_pgn;           // FIFO page list
};
```

---

### **Cấu trúc Physical Memory (Frame)**

```c
struct framephy_struct {
    addr_t fpn;                       // Frame Page Number (FPN)
    struct framephy_struct *fp_next;  // Next frame
    struct mm_struct* owner;          // Owner process mm_struct
};

struct memphy_struct {
    BYTE *storage;                    // Pointer tới actual memory
    int maxsz;                        // Max size
    int rdmflg;                       // Random flag
    int cursor;                       // Cursor position
    struct framephy_struct *free_fp_list;  // List các free frames
    struct framephy_struct *used_fp_list;  // List các used frames
};
```

---

## 🔢 Các Macro quan trọng

### **Page Size & Memory Layout**

```c
#define PAGING_PAGESZ       256         // 1 page = 256 bytes
#define PAGING_MEMRAMSZ     BIT(21)     // RAM size = 2MB (2^21)
#define PAGING_MEMSWPSZ     BIT(29)     // Swap size = 512MB (2^29)
#define PAGING_CPU_BUS_WIDTH 22         // 22-bit addressing = 4MB max
```

### **Address Components**

```c
#define PAGING_ADDR_OFFST_LOBIT 0
#define PAGING_ADDR_OFFST_HIBIT 7       // Offset: bits 0-7 (8 bits = 256 values)

#define PAGING_ADDR_PGN_LOBIT   8
#define PAGING_ADDR_PGN_HIBIT   21      // Page Number: bits 8-21 (14 bits)

#define PAGING_ADDR_FPN_LOBIT   8
#define PAGING_ADDR_FPN_HIBIT   20      // Frame Page Number: bits 8-20

/* Masks */
#define PAGING_OFFST_MASK   GENMASK(7, 0)      // 0xFF
#define PAGING_PGN_MASK     GENMASK(21, 8)     // 0x3FFF00
#define PAGING_FPN_MASK     GENMASK(20, 8)     // 0x1FFF00
```

### **Address Extraction**

```c
/* Lấy các thành phần từ virtual address */
#define PAGING_OFFST(x)  GETVAL(x, PAGING_OFFST_MASK, 0)      // Offset
#define PAGING_PGN(x)    GETVAL(x, PAGING_PGN_MASK, 8)        // Page Number
#define PAGING_FPN(x)    GETVAL(x, PAGING_FPN_MASK, 8)        // Frame Page Number

/* Ví dụ:
   Virtual Address: 0x12A5 = 0001 0010 1010 0101
   Offset: 0x12A5 & 0xFF = 0xA5 = 165
   PGN:    (0x12A5 >> 8) & 0x3FFF = 0x12 = 18
*/
```

---

## 🎯 Syscall Codes

### **Memory Operation Codes** (được sử dụng trong sys_mem.c)

```c
#define SYSMEM_MAP_OP     0     // Map memory: vmap_pgd_memset
#define SYSMEM_INC_OP     1     // Increase VMA limit: inc_vma_limit
#define SYSMEM_SWP_OP     2     // Swap page: __mm_swap_page
#define SYSMEM_IO_READ    3     // Read from physical memory
#define SYSMEM_IO_WRITE   4     // Write to physical memory
```

### **Syscall Registers** (struct sc_regs)

```c
struct sc_regs {
    arg_t a1;           // Argument 1 (first parameter)
    arg_t a2;           // Argument 2 (second parameter)
    arg_t a3;           // Argument 3 (third parameter/return value)
    arg_t a4;           // Argument 4
    arg_t a5;           // Argument 5
    arg_t a6;           // Argument 6
    uint32_t orig_ax;   // Original syscall number
    int32_t flags;      // Flags
};
```

### **Gọi Syscall từ User Program**

```
Syscall: memmap (số 17)
Payload trong regs:
  a1 = memop code (0-4)
  a2 = parameter 1 (tùy memop)
  a3 = parameter 2 (tùy memop)
  
Ví dụ:
  // Increase VMA 1 by 0x100 bytes
  regs.a1 = SYSMEM_INC_OP      (memop)
  regs.a2 = 1                  (vmaid)
  regs.a3 = 0x100              (inc_sz)
```

---

## 💾 Function Prototypes

### **Virtual Memory Management Functions**

```c
/* Allocate pages */
addr_t alloc_pages_range(struct pcb_t *caller, 
                        int req_pgnum,              // Số pages cần allocate
                        struct framephy_struct **frm_lst);

/* Map VM area to RAM */
addr_t vm_map_ram(struct pcb_t *caller, 
                 addr_t astart,                    // Start address
                 addr_t aend,                      // End address
                 addr_t mapstart,                  // Map start point
                 int incpgnum,                     // Số pages
                 struct vm_rg_struct *ret_rg);    // Return region

/* Map pages with memset */
int vmap_pgd_memset(struct pcb_t *caller,
                   addr_t addr,                   // Aligned address
                   int pgnum);                     // Number of pages

/* Map range of pages */
addr_t vmap_page_range(struct pcb_t *caller,
                      addr_t addr,                 // Start address
                      int pgnum,                   // Number of pages
                      struct framephy_struct *frames,
                      struct vm_rg_struct *ret_rg);

/* Get VMA by ID */
struct vm_area_struct *get_vma_by_num(struct mm_struct *mm, int vmaid);

/* Find victim page for swap */
int find_victim_page(struct mm_struct* mm, addr_t *pgn);

/* Get symbol region by ID */
struct vm_rg_struct * get_symrg_byid(struct mm_struct* mm, int rgid);
```

---

## 🔄 Flow: Syscall Memmap

### **User Program gọi SYSMEM_INC_OP:**

```
1. User Process: call SYSMEM_INC_OP (vmaid=1, inc_sz=0x100)
                 └─> Load thành syscall registers (a1, a2, a3)
                 └─> SYSCALL instruction

2. Kernel: syscall() dispatcher
           └─> Number 17 (memmap) → __sys_memmap()

3. __sys_memmap(): handler
           a1 = SYSMEM_INC_OP
           a2 = vmaid (1)
           a3 = inc_sz (0x100)
           └─> switch(a1):
               case SYSMEM_INC_OP:
                 inc_vma_limit(caller, a2, a3)
                 └─> Tăng VMA 1 lên 0x100 bytes

4. Return: regs->a3 = result (0 = success, -1 = error)
           └─> User process continues
```

---

## 🧠 Chi tiết hàm: `inc_vma_limit()`

### **Mục đích:** Tăng giới hạn của một VMA

### **Input:**
- `caller`: PCB của process gọi syscall
- `vmaid`: ID của VMA cần tăng (0-3)
- `inc_sz`: Size cần tăng (byte)

### **Output:**
- `0`: Success
- `-1`: Error (overlap, insufficient memory, etc.)

### **Logic chi tiết:**

```
1. Align size:
   - inc_sz có thể không align page size
   - Phải align thành multiple của PAGING_PAGESZ (256 bytes)
   - Ví dụ: inc_sz=0x150 → align thành 0x200 (2 pages)

2. Get current VMA:
   - Lấy VMA bằng vmaid
   - Nếu không tìm thấy → return -1

3. Calculate new boundary:
   - old_end = cur_vma->vm_end
   - new_end = old_end + inc_amt
   - Example: vm_end=0x2000 + 0x200 → new_end=0x2200

4. Check overlap:
   - Kiểm tra [old_end, new_end) không overlap với VMAs khác
   - Sử dụng validate_overlap_vm_area()

5. Map memory:
   - Allocate frames từ physical memory
   - Map virtual [old_end, new_end) → physical frames
   - Sử dụng vm_map_ram()

6. Update VMA:
   - cur_vma->vm_end = new_end
   - cur_vma->sbrk = new_end
   - (nếu cần)

7. Return 0 (success)
```

### **Ví dụ thực tế:**

```c
Process p1 có:
  VMA[0]: vm_start=0x0, vm_end=0x1000
  VMA[1]: vm_start=0x1000, vm_end=0x2000 ← want to increase
  VMA[2]: vm_start=0x3000, vm_end=0x4000

Call: inc_vma_limit(p1, vmaid=1, inc_sz=0x200)

Step 1: inc_amt = align(0x200) = 0x200 (already aligned)
Step 2: cur_vma = VMA[1] at 0x1000-0x2000
Step 3: new_end = 0x2000 + 0x200 = 0x2200
Step 4: Check [0x2000, 0x2200) vs VMA[2] at 0x3000-0x4000 → No overlap ✓
Step 5: Map 0x2000-0x2200 → 2 frames
Step 6: Update: VMA[1].vm_end = 0x2200

Result: VMA[1] now 0x1000-0x2200 ✓
```

---

## 🛡️ Error Handling

### **Trường hợp lỗi có thể gặp:**

```c
1. Process không tìm thấy
   └─ Check: caller == NULL → return -1

2. VMA không tồn tại (invalid vmaid)
   └─ Check: get_vma_by_num() == NULL → return -1

3. Overlap với VMA khác
   └─ Check: validate_overlap_vm_area() < 0 → return -1

4. Insufficient physical memory
   └─ Check: vm_map_ram() < 0 → return -1

5. Invalid size
   └─ Check: inc_sz == 0 → return -1
```

---

## 📋 Testing Checklist

### **Tester nhân viên:**

```
[ ] Test 1: Fix OVERLAP macro
    ├─ OVERLAP(0, 10, 5, 15) should be 1 (true)
    ├─ OVERLAP(0, 10, 10, 20) should be 0 (false)
    └─ OVERLAP(0, 10, 11, 20) should be 0 (false)

[ ] Test 2: get_vm_area_node_at_brk()
    ├─ Can retrieve node at break point
    ├─ Boundaries are correct
    └─ Break point is updated

[ ] Test 3: validate_overlap_vm_area()
    ├─ Detects overlap correctly
    ├─ Allows non-overlapping allocation
    └─ Handles edge cases

[ ] Test 4: inc_vma_limit()
    ├─ VMA size increases correctly
    ├─ Size is properly aligned
    ├─ Detects overlaps
    └─ Memory is mapped properly

[ ] Test 5: Compilation
    ├─ make clean
    ├─ make
    └─ No errors/warnings

[ ] Test 6: Integration Test
    ├─ Run with input files
    ├─ Check output correctness
    └─ No segfaults
```

---

## 📖 Tài liệu tham khảo

- **Paging-based Memory Management**: Modern OS concepts
- **Virtual Memory**: Address translation, TLB, page tables
- **System Calls**: Mechanism để user program gọi kernel functions
- **Memory Swap**: Page replacement algorithms (FIFO, LRU, etc.)

---

**Version:** 1.0  
**Last Updated:** 17/11/2025
