#include "syscall.h"
#include "common.h"
#include "os-mm.h"
#include "mm.h"
#include <stdlib.h>
#include "queue.h" // Vẫn cần include file này

int __sys_meminc(struct krnl_t *krnl, uint32_t pid, struct sc_regs* regs)
{
    int vmaid = regs->a1;
    addr_t inc_sz = regs->a2;
    
    struct pcb_t *caller = NULL;
    struct queue_t *running_list = krnl->running_list;
    
    // Tìm kiếm PCB của tiến trình gọi dựa trên PID
    if (running_list != NULL) {
        // Duyệt qua mảng proc[] dựa trên kích thước size hiện tại
        for (int i = 0; i < running_list->size; i++) {
            struct pcb_t *proc = running_list->proc[i];
            
            // Kiểm tra nếu tìm thấy PID trùng khớp
            if (proc != NULL && proc->pid == pid) {
                caller = proc;
                break; // Tìm thấy rồi thì thoát vòng lặp
            }
        }
    }
    // --------------------------------------
    
    if (caller == NULL) return -1;
    
    int ret = inc_vma_limit(caller, vmaid, inc_sz);
    regs->a3 = ret;
    return ret;
}