# 📝 CHANGELOG - TASK 5

**Người thực hiện:** Bình
**Ngày cập nhật:** 18/11/2025

-----

## 🚀 1. Các Tính Năng Đã Hoàn Thành

Đã hoàn thiện module **User Space Library (LIBMEM)** và logic **64-bit Paging** theo yêu cầu.

* **Giao diện Người dùng (`src/libmem.c`):**
    * [cite_start]Implement các hàm wrapper đóng vai trò cầu nối giữa tiến trình người dùng và Kernel[cite: 12].
    * [cite_start]**`liballoc` / `libfree`**: Đóng gói tham số vào struct `sc_regs` và gọi System Call `SYSCALL_MEM` với opcode `SYSMEM_INC_OP`[cite: 86, 87].
    * [cite_start]**`libread` / `libwrite`**: Gọi System Call với opcode `SYSMEM_IO_READ` và `SYSMEM_IO_WRITE` để thực hiện IO[cite: 88].

* **Quản lý Bộ nhớ 64-bit (`src/mm64.c`):**
    * [cite_start]Hiện thực logic phân trang 5 cấp (5-level paging) cho kiến trúc 64-bit[cite: 90, 91].
    * [cite_start]**`get_pd_from_pagenum`**: Hàm trích xuất index cho các bảng trang (PGD, P4D, PUD, PMD, PT) từ địa chỉ ảo 64-bit[cite: 91].
    * **`vmap_pgd_memset`**: Hàm khởi tạo và thiết lập giá trị mẫu cho Page Global Directory[cite: 92].
    * Cập nhật các macro Bitmask trong `include/mm64.h` để hỗ trợ không gian địa chỉ 57-bit.

## 🔧 2. Sửa Lỗi Quan Trọng (Fixes)

### Xử lý Xung đột Kiểu dữ liệu (Conflicting Types)
Giải quyết vấn đề xung đột chữ ký hàm giữa header 32-bit (`mm.h`) và 64-bit (`mm64.h`) gây lỗi biên dịch.

```c
/* CŨ (Gây lỗi conflict trong mm.h): */
// int get_pd_from_pagenum(addr_t pgn, ...);
// int vmap_pgd_memset(struct pcb_t *caller, addr_t addr, int pgnum);

/* MỚI (Đã đồng bộ hóa cho 64-bit trong mm64.c): */
void get_pd_from_pagenum(uint64_t pgn, uint32_t *idx_pgd, ...);
int vmap_pgd_memset(struct pcb_t *caller, addr_t addr, int pgnum, addr_t pattern);

## ⚙️ 3. Cấu Hình Hệ Thống

### Cập nhật Source Code & File Structure
Hệ thống đã được bổ sung các module mới phục vụ cho User Space và 64-bit addressing. Cấu trúc file được tổ chức như sau:

* **`src/libmem.c`**: Source code triển khai các hàm thư viện người dùng (`liballoc`, `libfree`, `libread`, `libwrite`). Đây là tầng trung gian đóng gói yêu cầu thành `syscall` gửi xuống Kernel.
* **`src/mm64.c`**: Source code chứa logic quản lý bộ nhớ 64-bit (chỉ được biên dịch khi cờ `MM64` được bật).
* **`include/libmem.h`**: Header file định nghĩa các prototype cho thư viện người dùng và các mã Opcode (`SYSMEM_...`).
* **`include/mm64.h`**: Header file chứa các định nghĩa struct và macro bitmask cho phân trang 5 cấp (5-level paging).

## 🧪 4. Kiểm Thử (Testing)

Chiến lược kiểm thử được áp dụng là **Unit Testing độc lập (Independent Testing)**. Do các module của Thành viên 5 (LIBMEM và MM64) có tính chất là giao diện và tính toán logic thuần túy, chúng được tách biệt hoàn toàn khỏi Kernel chính để kiểm tra độ chính xác trước khi tích hợp.

### ✅ Test Case 1: User Interface Wrapper (LIBMEM)

* **Mục tiêu:** Kiểm chứng tính chính xác của việc đóng gói tham số và gọi System Call từ phía người dùng (User Space). Đảm bảo các hàm `lib...` gửi đúng `Opcode` và dữ liệu xuống Kernel.
* **Phương pháp:** Sử dụng kỹ thuật **Mocking (Giả lập)**. Một hàm `syscall` giả (Stub) được viết trong `test_libmem.c` để bắt các yêu cầu từ `libmem`, in ra các giá trị thanh ghi nhận được thay vì thực thi logic kernel phức tạp.
* **File thực thi:** `src/test_libmem.c`
* **Lệnh biên dịch & Chạy:**
    ```bash
    gcc -o test_libmem -I ../include libmem.c test_libmem.c
    ./test_libmem
    ```
* **Kết quả thực tế:**
    * Hàm `liballoc(size=1024)` ➔ Gửi thành công Opcode **2** (`SYSMEM_INC_OP`) với Size **1024**.
    * Hàm `libfree(size=512)` ➔ Gửi thành công Opcode **2** (`SYSMEM_INC_OP`) với Size **512**.
    * Hàm `libread` ➔ Gửi thành công Opcode **4** (`SYSMEM_IO_READ`).
    * Hàm `libwrite` ➔ Gửi thành công Opcode **5** (`SYSMEM_IO_WRITE`).
    * **Kết luận:** ✅ **PASSED**. Logic wrapper hoạt động chính xác.

### ✅ Test Case 2: 64-bit Address Translation

* **Mục tiêu:** Kiểm tra tính chính xác toán học của thuật toán dịch địa chỉ ảo 64-bit sang 5 cấp chỉ mục bảng trang (PGD, P4D, PUD, PMD, PT).
* **Phương pháp:** Chạy kiểm tra với các địa chỉ ảo mẫu (bao gồm các trường hợp biên của kiến trúc 57-bit) qua hàm `translate_64bit_address`. So sánh kết quả tính toán (bit shifting & masking) với giá trị kỳ vọng.
* **File thực thi:** `src/test_64bit.c` (kết hợp với `translate64.c`)
* **Lệnh biên dịch & Chạy:**
    ```bash
    gcc -o test64 -I ../include translate64.c test_64bit.c
    ./test64
    ```
* **Kết quả thực tế:**
    * Output nhận được: `64BIT PAGING TEST PASSED!`
    * **Kết luận:** ✅ **PASSED**. Các macro bitmask và logic dịch bit trong `mm64.h` hoạt động đúng với chuẩn kiến trúc 64-bit giả lập.
