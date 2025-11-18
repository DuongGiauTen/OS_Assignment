
# 📝 CHANGELOG 

**Người thực hiện:** Dương
**Ngày cập nhật:** 18/11/2025

-----

## 🚀 1. Các Tính Năng Đã Hoàn Thành
- Đã Merge Feature/scheduler vào Assignment

  * **Core Logic (`src/mm-vm.c`):** Hoàn thiện các hàm quản lý vùng nhớ ảo (`inc_vma_limit`, `get_vm_area_node_at_brk`, `validate_overlap_vm_area`).
  * **System Call Handlers:**
      * Tạo mới `src/sys_meminc.c` (Xử lý tăng bộ nhớ).
      * Tạo mới `src/sys_memswp.c` (Xử lý hoán đổi trang).
      * Cập nhật `src/sys_mem.c`: Sửa logic tìm kiếm Process thực tế (thay vì tạo Dummy) và trả về giá trị đọc được cho `SYSMEM_IO_READ`.
  * **Merge Feature:** Đã tích hợp thành công module **Scheduler** vào hệ thống.

## 🔧 2. Sửa Lỗi Quan Trọng (Fixes)

### Cập nhật Macro Logic (`include/mm.h`)

Sửa lại công thức tính toán chồng lấn vùng nhớ (Overlap) và bao hàm (Include) để hệ thống phát hiện lỗi cấp phát chính xác.

```c
/* CŨ (Sai logic): */
// #define INCLUDE(x1,x2,y1,y2) (0)
// #define OVERLAP(x1,x2,y1,y2) (0)

/* MỚI (Đã sửa): */
#define OVERLAP(x1,x2,y1,y2) ((x1 < y2) && (y1 < x2))
#define INCLUDE(x1,x2,y1,y2) ((x1 <= y1) && (y2 <= x2))
```

## ⚙️ 3. Cấu Hình Hệ Thống

### Cập nhật `Makefile`

Thêm các object file mới vào quá trình biên dịch để hệ thống nhận diện các syscall mới.

```makefile
# Đã thêm sys_meminc.o và sys_memswp.o vào SYSCALL_OBJ
SYSCALL_OBJ = $(addprefix $(OBJ)/, syscall.o sys_mem.o sys_meminc.o sys_memswp.o sys_listsyscall.o)
```

## 🧪 4. Kiểm Thử (Testing)

Đã thêm các file Unit Test chuyên biệt để kiểm tra logic độc lập:

  * **`src/test_mm.c`**: Kiểm tra logic tính toán vùng nhớ, làm tròn trang và cập nhật `sbrk`.
  * **`src/test_sys_meminc.c`**: Kiểm tra luồng gọi System Call và truyền tham số.
  * **Kết quả:** Các test case đều đã PASSED (bao gồm cả các trường hợp biên như Overlap, Invalid ID, Out of Memory).

-----

- Cách test mm-vm.c

 ```c++
 gcc -I ../include -o test_runner mm-vm.c test_mm.c
 
 ./test_runner
 
 ```
- Cách test sys_meminc.c: 
```c++
gcc -I ../include -o test_syscall sys_meminc.c test_sys_meminc.c

./test_syscall
```


**Trạng thái hiện tại:** ✅ Ready to Merge.
