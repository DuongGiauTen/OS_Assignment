#include <stdio.h>
#include <stdlib.h>
#include "mm-memphy.h"

/* ---------------------------
   Khởi tạo MEMPHY
---------------------------- */
int memphy_init(struct memphy_struct *mp, int maxfp, int rdmflag)
{
    mp->maxfp = maxfp;
    mp->rdmflg = rdmflag;

    mp->storage = (unsigned char *) malloc(maxfp * FRAME_SIZE);
    if (!mp->storage) return -1;

    mp->free_fp_list = NULL;

    // Đổ toàn bộ frame vào free list
    for (int i = maxfp - 1; i >= 0; i--) {
        struct freeframe_node *node =
            (struct freeframe_node*) malloc(sizeof(struct freeframe_node));
        node->fpn = i;
        node->next = mp->free_fp_list;
        mp->free_fp_list = node;
    }

    return 0;
}

/* ---------------------------
   Cấp phát frame
---------------------------- */
int memphy_alloc(struct memphy_struct *mp, int *retfpn)
{
    if (!mp->free_fp_list)
        return -1; // hết frame

    struct freeframe_node *node = mp->free_fp_list;
    mp->free_fp_list = node->next;

    *retfpn = node->fpn;
    free(node);
    return 0;
}

/* ---------------------------
   Trả frame về free list
---------------------------- */
int memphy_free(struct memphy_struct *mp, int fpn)
{
    if (fpn < 0 || fpn >= mp->maxfp)
        return -1;

    struct freeframe_node *node =
        (struct freeframe_node*) malloc(sizeof(struct freeframe_node));

    node->fpn = fpn;
    node->next = mp->free_fp_list;
    mp->free_fp_list = node;

    return 0;
}

/* ---------------------------
   Read từ MEMPHY
---------------------------- */
int MEMPHY_read(struct memphy_struct *mp, int fpn, int offset, unsigned char *value)
{
    if (fpn < 0 || fpn >= mp->maxfp) return -1;
    if (offset < 0 || offset >= FRAME_SIZE) return -1;

    int addr = fpn * FRAME_SIZE + offset;
    *value = mp->storage[addr];

    return 0;
}

/* ---------------------------
   Write vào MEMPHY
---------------------------- */
int MEMPHY_write(struct memphy_struct *mp, int fpn, int offset, unsigned char value)
{
    if (fpn < 0 || fpn >= mp->maxfp) return -1;
    if (offset < 0 || offset >= FRAME_SIZE) return -1;

    int addr = fpn * FRAME_SIZE + offset;
    mp->storage[addr] = value;

    return 0;
}

/* ---------------------------
   Debug: In free list
---------------------------- */
void memphy_print_free_list(struct memphy_struct *mp)
{
    struct freeframe_node *cur = mp->free_fp_list;
    printf("Free frames: ");
    while (cur) {
        printf("%d ", cur->fpn);
        cur = cur->next;
    }
    printf("\n");
}
