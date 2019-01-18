#include "lms_caching.h"   

CACHE *searchCache(unsigned long hddBlock)
{   
	CACHE *search; 
	search = cachingTable;    
	while(search != NULL)     
	{         
		if(search->hddBlkno == hddBlock)  
		{ 
			return search;
		} 
		else
		{
			search = search->next;
		}
	}
	return NULL;
}

CACHE *evictCache(unsigned long hdd_blk, int scheme)
{	
	CACHE *search =NULL;
	if (scheme == 0)
	{
		if (hdd_blk == -1)
		{	
			for (search = cachingTable; search->next != NULL; search = search->next)
			{
				;	
			}
			cacheSpace[search->ssdPage] = FREE_PAGE;
			CACHE *tmp;
			tmp = search->pre;
			tmp->next = NULL;
			if (search != NULL)
			{
				return search;
			}
			else
			{
				return NULL;
			}
		}
		else
		{	
			search = cachingTable;
			if (search->hddBlkno == hdd_blk)
			{
				cacheSpace[search->ssdPage] = FREE_PAGE;
				CACHE *tmp;
				tmp = search->next;
				tmp->pre = NULL;
				cachingTable = cachingTable->next;
				return search;
			}
			else
			{
				CACHE *tmp = NULL;
				for (search = cachingTable; search->next != NULL; search=search->next)
				{	
					tmp = search->next;
					if (tmp->hddBlkno == hdd_blk)
					{				
						cacheSpace[tmp->ssdPage] = FREE_PAGE;
						search->next = tmp->next;       
						if (tmp->next != NULL) 
						{	
							CACHE *tmp2;
							tmp2 = tmp->next;
							tmp2->pre = search;
						}
						return search;
					}
				}
			}
			return NULL;
		}
	}
	else if (scheme == 1)
	{	
		for (search = cachingTable; search->next != NULL; search = search->next)
		{
			;	
		}

		
		for (; search != NULL; search = search->pre)
		{
			if (hdd_blk == search->hddBlkno/(SSD_PAGE2SECTOR*SSD_PAGES_PER_BLOCK) )
			{	
				// printf("search->hddblk = %d\n", search->hddBlkno);
				// printf("hddblk = %d\n", hdd_blk);
				cacheSpace[search->ssdPage] = FREE_PAGE;
				if (search->next != NULL)
				{
					if (search->pre != NULL)
					{
						CACHE *tmp;
						CACHE *tmp2;
						tmp = search->pre;
						tmp->next = search->next;
						tmp2 = search->next;
						tmp2->pre = search->pre; 
					} 
					else 
					{   
						cachingTable = search->next;
					}
				}
				else
				{
					if (search->pre != NULL)
					{	
						CACHE *tmp;
						tmp = search->pre;
						tmp->next =NULL;
					}
				}
				return search;	
			}	
		}
		// displayCacheTable();
		return NULL;	
	}
}

void displayFreeSpace()
{
	int i;
	for (i = 0; i < CACHE_SPACE; i++)
	{
		if (cacheSpace[i] == FREE_PAGE)
		{
			printf("%d\n", i);				
		}	
	}
}

void displayCacheTable()
{	
	if (cachingTable != NULL)
	{
		CACHE *tmp;
		printf(COLOR_BLUE"<Cache Table>\n"COLOR_RESET);
		printf(COLOR_BLUE"Flag 1 = Clean Page, Flag 2 = Dirty Page \n");
		printf(COLOR_BLUE"<MRU>=======================================================<MRU>\n"COLOR_RESET);
		for(tmp = cachingTable; tmp->next != NULL; tmp = tmp->next)
		{
			printf(COLOR_BLUE"SSD Page Number = %4lu <=> HDD Block Number = %4lu\tFlag = %d\n", tmp->ssdPage, tmp->hddBlkno, tmp->flag);
		}
		printf(COLOR_BLUE"SSD Page Number = %4lu <=> HDD Block Number = %4lu\tFlag = %d\n", tmp->ssdPage, tmp->hddBlkno, tmp->flag);
		printf(COLOR_BLUE"<LRU>=======================================================<LRU>\n"COLOR_RESET);
	}
	else
	{
		printf(COLOR_RED"No data in the cache\n"COLOR_RESET);
	}
}
// int cd=0;
int caching(unsigned long *ssdPage, unsigned long *hddBlock, int flag)
{	
	if (cachingTable == NULL)
	{	
		int i;
		for (i = 0; i < CACHE_SPACE; i++)
		{
			cacheSpace[i] == FREE_PAGE;
		}
		cachingTable = malloc(sizeof(CACHE));
		cachingTable->ssdPage = 0;
		*ssdPage = 0;
		cacheSpace[0] = flag;
		cachingTable->hddBlkno = *hddBlock;
		cachingTable->flag = flag;
		cachingTable->pre = NULL;
		cachingTable->next = NULL;
	}
	else
	{
		CACHE *search;
		search = malloc(sizeof(CACHE));
		search = searchCache(*hddBlock);

		if (search == NULL)
		{
			int i;
			for (i = 0; i < CACHE_SPACE; i++)
			{
				if (cacheSpace[i] == FREE_PAGE)
				{
					// cd++;
					// printf("page:%d\n", cd);
					break;					
				}	
			}

			if (i == CACHE_SPACE)
			{	
				return -1;
			}

			CACHE *tmp;
			tmp = malloc(sizeof(CACHE));
			tmp->ssdPage = i;
			*ssdPage = i;
			cacheSpace[i] = flag;
			tmp ->hddBlkno = *hddBlock;
			tmp ->flag = flag;
			cachingTable->pre = tmp;
			tmp->next = cachingTable;
			tmp->pre = NULL;
			cachingTable = cachingTable->pre;
		}
		else
		{	
			switch(flag)
			{
				case CLEAN_PAGE:
					break;
				case DIRTY_PAGE:
					cacheSpace[search->ssdPage] = flag;
					break;
				default:
					printf(COLOR_RED"Caching Error\n"COLOR_RESET);
			}

			if (search->pre != NULL)
			{	
				CACHE *tmp;
				tmp = malloc(sizeof(CACHE));
				CACHE *tmp2;
				tmp2 = malloc(sizeof(CACHE));
				if (search->next ==NULL)
				{
					tmp = search->pre;
					tmp->next = NULL;
					cachingTable->pre = search;
					search->next = cachingTable;
					search->pre = NULL;
					cachingTable = cachingTable->pre;
				}
				else
				{	
					tmp = search->pre;
					tmp2 =search->next;
					tmp->next = search->next;
					tmp2->pre = search->pre;
					cachingTable->pre = search;
					search->next = cachingTable;
					search->pre = NULL;
					cachingTable = cachingTable->pre;
				}
			}
		}
	}
	return 0;
}