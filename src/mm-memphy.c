/*
 * Copyright (C) 2026 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* LamiaAtrium release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

// #ifdef MM_PAGING
/*
 * PAGING based Memory Management
 * Memory physical module mm/mm-memphy.c
 */
#include "../include/mm-memphy.h"
#include "../include/mm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int MEMPHY_mv_csr(struct memphy_struct *mp, addr_t offset)
{
    int numstep = 0;

    mp->cursor = 0;
    while (numstep < offset && numstep < mp->maxsz)
    {
        mp->cursor = (mp->cursor + 1) % mp->maxsz;
        numstep++;
    }

    return 0;
}

/* Sequential read */
int MEMPHY_seq_read(struct memphy_struct *mp, addr_t addr, BYTE *value)
{
    if (mp == NULL) return -1;
    /* Sửa lại: Nếu là Random access (RAM) thì mới return lỗi, Sequential thì cho qua */
    if (mp->rdmflg) return -1; 

    MEMPHY_mv_csr(mp, addr);
    *value = mp->storage[addr];
    return 0;
}

/* Random or sequential read */
int MEMPHY_read(struct memphy_struct *mp, addr_t addr, BYTE *value)
{
    if (mp == NULL) return -1;

    if (mp->rdmflg)      // RAM
        *value = mp->storage[addr];
    else                 // sequential device
        return MEMPHY_seq_read(mp, addr, value);

    return 0;
}

/* Sequential write */
int MEMPHY_seq_write(struct memphy_struct *mp, addr_t addr, BYTE data)
{
    if (mp == NULL) return -1;
    /* Sửa lại tương tự */
    if (mp->rdmflg) return -1;

    MEMPHY_mv_csr(mp, addr);
    mp->storage[addr] = data;
    return 0;
}

/* Random or sequential write */
int MEMPHY_write(struct memphy_struct *mp, addr_t addr, BYTE data)
{
    if (mp == NULL) return -1;

    if (mp->rdmflg)
        mp->storage[addr] = data;
    else
        return MEMPHY_seq_write(mp, addr, data);

    return 0;
}

/* Format free frame list */
int MEMPHY_format(struct memphy_struct *mp, int pagesz)
{
    int numfp = mp->maxsz / pagesz;
    if (numfp <= 0) return -1;

    struct framephy_struct *fst = malloc(sizeof(*fst));
    fst->fpn = 0;
    mp->free_fp_list = fst;

    struct framephy_struct *cur = fst;

    for (int i = 1; i < numfp; ++i)
    {
        struct framephy_struct *node = malloc(sizeof(*node));
        node->fpn = i;
        node->fp_next = NULL;
        cur->fp_next = node;
        cur = node;
    }

    return 0;
}

/* Pop a free frame */
int MEMPHY_get_freefp(struct memphy_struct *mp, addr_t *retfpn)
{
    struct framephy_struct *fp = mp->free_fp_list;

    if (fp == NULL)
        return -1;

    *retfpn = fp->fpn;
    mp->free_fp_list = fp->fp_next;
    free(fp);
    return 0;
}

/* Push a frame back to free list */
int MEMPHY_put_freefp(struct memphy_struct *mp, addr_t fpn)
{
    struct framephy_struct *node = malloc(sizeof(*node));
    node->fpn = fpn;
    node->fp_next = mp->free_fp_list;
    mp->free_fp_list = node;
    return 0;
}

/* Init memphy */
int init_memphy(struct memphy_struct *mp, addr_t max_size, int randomflg)
{
    mp->storage = malloc(max_size);
    mp->maxsz = max_size;
    memset(mp->storage, 0, max_size);

    MEMPHY_format(mp, PAGING_PAGESZ);

    mp->rdmflg = randomflg ? 1 : 0;
    mp->cursor = 0;

    return 0;
}

int MEMPHY_dump(struct memphy_struct *mp)
{
   return 0;
}

// #endif