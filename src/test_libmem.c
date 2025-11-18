/*
 * test_libmem.c - Driver kiểm thử cho thư viện bộ nhớ
 */
#include <stdio.h>
#include <stdint.h>
#include "libmem.h"
#include "common.h" // Cần chứa struct pcb_t

// --- 1. Định nghĩa Mock Structures (Giả lập) ---
// Nếu common.h của bạn chưa đầy đủ struct sc_regs, ta định nghĩa lại ở đây để test
struct sc_regs {
    uint32_t a1; // Opcode
    uint32_t a2;
    uint32_t a3;
    uint32_t a4;
};

// Định nghĩa struct pcb_t tối giản cho test nếu cần
// (Nếu common.h đã có thì trình biên dịch sẽ báo duplicate, lúc đó hãy bỏ đoạn này)
// struct pcb_t {
//     int pid;
//     void *krnl; // Giữ void* để tránh phụ thuộc struct kernel
// };

// --- 2. Hàm Stub Syscall (Quan trọng nhất) ---
// Hàm này phải khớp chữ ký với hàm được gọi trong libmem.c
int syscall(void *krnl, int pid, int sys_id, struct sc_regs *regs) {
    printf("[STUB KERNEL] PID: %d call Syscall ID: %d\n", pid, sys_id);
    
    // Kiểm tra Opcode nằm trong thanh ghi a1
    printf("              -> Opcode (regs.a1): %d ", regs->a1);

    switch (regs->a1) {
        case SYSMEM_INC_OP: // 2
            printf("(ALLOC/FREE - MEMINC)\n");
            printf("              -> Size: %d\n", regs->a3);
            break;
        case SYSMEM_IO_READ: // 4
            printf("(READ - MEMIO)\n");
            printf("              -> Source: %d, Offset: %08x\n", regs->a3, regs->a4);
            break;
        case SYSMEM_IO_WRITE: // 5
            printf("(WRITE - MEMIO)\n");
            printf("              -> Data: %d, Dest: %d\n", regs->a3, regs->a4);
            break;
        default:
            printf("(Unknown Opcode)\n");
    }
    return 0;
}

// --- 3. Main Test ---
int main() {
    // Tạo một process giả
    struct pcb_t proc;
    proc.pid = 100;
    proc.krnl = NULL; // Không cần kernel thật để test logic đóng gói

    printf("=== TEST LIBMEM ===\n\n");

    // TEST 1: Cấp phát (Alloc)
    printf("--- Test 1: liballoc(size=1024) ---\n");
    liballoc(&proc, 0, 1024);
    // Kỳ vọng: Opcode 2, Size 1024

    printf("\n--- Test 2: libfree(size=512) ---\n");
    libfree(&proc, 512);
    // Kỳ vọng: Opcode 2, Size 512

    printf("\n--- Test 3: libwrite(data=99, dest=1, offset=0x20) ---\n");
    libwrite(&proc, 99, 1, 0x20);
    // Kỳ vọng: Opcode 5, Data 99

    printf("\n--- Test 4: libread(src=1, offset=0x10) ---\n");
    uint32_t data_out;
    libread(&proc, 1, 0x10, &data_out);
    // Kỳ vọng: Opcode 4

    printf("\n=== TEST DONE ===\n");
    return 0;
}