#ifndef MM_MEMPHY_H
#define MM_MEMPHY_H

#define FRAME_SIZE 1     // đơn giản hóa: mỗi frame chứa 1 byte
                         // dễ test, dễ debug

struct freeframe_node {
    int fpn;
    struct freeframe_node *next;
};

struct freeframe_node;

struct memphy_struct {
    int maxfp;                  // số frame
    unsigned char *storage;     // bộ nhớ vật lý
    struct freeframe_node *free_fp_list; // danh sách frame rảnh
    int rdmflg;                 // cờ RAM/SWAP (không bắt buộc dùng)
};

/* Hàm quản lý MEMPHY */
int memphy_init(struct memphy_struct *mp, int maxfp, int rdmflag);

int memphy_alloc(struct memphy_struct *mp, int *retfpn);
int memphy_free(struct memphy_struct *mp, int fpn);

int MEMPHY_read(struct memphy_struct *mp, int fpn, int offset, unsigned char *value);
int MEMPHY_write(struct memphy_struct *mp, int fpn, int offset, unsigned char value);

/* Hàm tiện ích nội bộ */
void memphy_print_free_list(struct memphy_struct *mp);

#endif
