#ifndef MM_MEMPHY_H
#define MM_MEMPHY_H

#include "../include/os-mm.h"

int init_memphy(struct memphy_struct *mp, addr_t max_size, int randomflg);
int MEMPHY_read(struct memphy_struct *mp, addr_t addr, BYTE *value);
int MEMPHY_write(struct memphy_struct *mp, addr_t addr, BYTE data);
int MEMPHY_get_freefp(struct memphy_struct *mp, addr_t *retfpn);
int MEMPHY_put_freefp(struct memphy_struct *mp, addr_t fpn);

#endif