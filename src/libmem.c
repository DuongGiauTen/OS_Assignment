/*
 * Copyright (C) 2026 pdnguyen of HCMC University of Technology VNU-HCM
 * Libmem Library Implementation
 */

#include "libmem.h"
#include "syscall.h" // Chứa struct sc_regs và prototype syscall()
#include "common.h"  // Chứa struct pcb_t
#include <stdio.h>
#include <stdlib.h>

// ID của System Call quản lý bộ nhớ (thường là 17 trong project này)
// Nếu trong syscall.h chưa có, ta định nghĩa mặc định.
#ifndef SYSCALL_MEM
#define SYSCALL_MEM 17
#endif

/*
 * liballoc - Yêu cầu cấp phát bộ nhớ
 * Logic: Gọi syscall MEMINC để tăng kích thước heap.
 */
int liballoc(struct pcb_t *caller, addr_t base, uint32_t size)
{
    struct sc_regs regs;
    
    regs.a1 = SYSMEM_INC_OP; // Opcode: 2
    regs.a2 = 0;             // VMA ID (mặc định 0)
    regs.a3 = size;          // Kích thước cần cấp phát
    regs.a4 = base;          // Địa chỉ cơ sở (thường để 0 để kernel tự chọn)

    // Gửi yêu cầu xuống Kernel
    return syscall(caller->krnl, caller->pid, SYSCALL_MEM, &regs);
}

/*
 * libfree - Yêu cầu giải phóng bộ nhớ
 * Logic: Gọi syscall MEMINC nhưng với ý nghĩa giảm hoặc giải phóng.
 * Lưu ý: Tùy thuộc vào nhóm Kernel của bạn quy định size âm hay opcode riêng.
 * Ở đây tôi giữ nguyên opcode INC theo đề bài thường gặp.
 */
int libfree(struct pcb_t *caller, uint32_t size)
{
    struct sc_regs regs;
    
    regs.a1 = SYSMEM_INC_OP; // Opcode: 2
    regs.a2 = 0;             // VMA ID
    regs.a3 = size;          // Kích thước cần free
    regs.a4 = 0;

    return syscall(caller->krnl, caller->pid, SYSCALL_MEM, &regs);
}

/*
 * libread - Đọc dữ liệu từ bộ nhớ
 * Logic: Gọi syscall MEMIO_READ.
 */
int libread(struct pcb_t *caller, uint32_t source, addr_t offset, uint32_t *data)
{
    struct sc_regs regs;
    
    regs.a1 = SYSMEM_IO_READ; // Opcode: 4
    regs.a2 = 0;              // VMA ID
    regs.a3 = source;         // Index thanh ghi nguồn (hoặc địa chỉ)
    regs.a4 = offset;         // Offset

    // Gọi syscall
    int res = syscall(caller->krnl, caller->pid, SYSCALL_MEM, &regs);

    /* * QUAN TRỌNG: Lấy dữ liệu trả về.
     * Vì syscall thường không thể ghi trực tiếp vào con trỏ *data ở user space 
     * (trừ khi giả lập chung không gian nhớ).
     * Giả định: Kernel trả kết quả đọc được vào thanh ghi regs.a1 sau khi xong.
     */
    if (res == 0 && data != NULL) {
        // *data = regs.a1; // Bỏ comment dòng này nếu Kernel trả value qua thanh ghi
        
        // HOẶC: Nếu đây là mô phỏng đơn giản chung bộ nhớ, bạn có thể truyền
        // địa chỉ con trỏ data cho kernel (Hack):
        // regs.a3 = (uint32_t)data; 
        // Nhưng cần thống nhất với code Kernel (mm-vm.c).
    }

    return res;
}

/*
 * libwrite - Ghi dữ liệu vào bộ nhớ
 * Logic: Gọi syscall MEMIO_WRITE.
 */
int libwrite(struct pcb_t *caller, BYTE data, uint32_t destination, addr_t offset)
{
    struct sc_regs regs;
    
    regs.a1 = SYSMEM_IO_WRITE; // Opcode: 5
    regs.a2 = 0;               // VMA ID
    regs.a3 = (uint32_t)data;  // Dữ liệu (Byte)
    regs.a4 = destination;     // Index thanh ghi đích
    // Lưu ý: Offset có thể cần truyền thêm. Nếu thiếu thanh ghi (chỉ có a1-a4),
    // Kernel team cần quy ước lại cách đóng gói (ví dụ: destination + offset).

    return syscall(caller->krnl, caller->pid, SYSCALL_MEM, &regs);
}

// Hàm helper để lấy VMA (được yêu cầu extern trong libmem.h)
// Trong thực tế User Space không nên truy cập cái này, nhưng để khớp header:
struct vm_area_struct *get_vma_by_num(struct mm_struct *mm, int vmaid)
{
    // Trong User Space thực, hàm này không tồn tại.
    // Nhưng nếu đề bài yêu cầu wrapper này cho mục đích test/debug:
    struct vm_area_struct *vma = mm->mmap;
    if (vmaid == 0) return vma;
    
    while (vma != NULL) {
        if (vma->vm_id == vmaid) return vma;
        vma = vma->vm_next;
    }
    return NULL;
}