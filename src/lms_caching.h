#ifndef LMS_CACHING_H
#define LMS_CACHING_H

#include <stdlib.h>
#include <stdio.h>

#include "lmssim.h"
#include "lms_parameter.h"

typedef struct cache
{
	struct CACHE *pre;
	unsigned long ssdPage;
	unsigned long hddBlkno;
	int flag;
	struct CACHE *next;
}CACHE;

static CACHE *cachingTable = NULL;

static int cacheSpace[CACHE_SPACE];

CACHE *searchCache(unsigned long hddBlock);
CACHE *evictCache(unsigned long hddBlock, int scheme);
void displayFreeSpace();
void displayCacheTable();
int caching(unsigned long *ssdBlock, unsigned long *hddBlock, int flag);

#endif