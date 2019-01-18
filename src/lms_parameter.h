#ifndef LMS_PARAMETER_H
#define LMS_PARAMETER_H

/*Color*/
#define COLOR_RED "\x1b[;31;1m"
#define COLOR_GREEN "\x1b[;32;1m"
#define COLOR_YELLOW "\x1b[;33;1m"
#define COLOR_BLUE "\x1b[;34;1m"
#define COLOR_MAGENTA "\x1b[;35;1;1m"
#define COLOR_CYAN "\x1b[;36;1;1m"
#define COLOR_RESET "\x1b[0;m"

#define DISKSIM_SECTOR 512 //byte
// #define HDD_BLOCK_SIZE 4096 
// #define HDD_BLOCK2SECTOR (HDD_BLOCK_SIZE/DISKSIM_SECTOR)
#define SSD_PAGE_SIZE 4096 //byte
#define SSD_PAGE2SECTOR (SSD_PAGE_SIZE/DISKSIM_SECTOR) //8(byte)
#define SSD_PAGES_PER_BLOCK 64
#define SSD_BLOCK_SIZE (SSD_PAGE_SIZE*SSD_PAGES_PER_BLOCK) //262144(byte)
#define SSD_BLOCK2SECTOR (SSD_BLOCK_SIZE/DISKSIM_SECTOR) //512(byte)

/*Messag Queue*/
#define MSQ_KEY_SSD 0x1000
#define MSQ_KEY_HDD 0x1001
#define MSQ_KEY_MAIN 0x1002
#define MSG_TYPE_SSD 100
#define MSG_TYPE_HDD 200
#define MSG_TYPE_MAIN 300

// #define TIME_PERIOD 3
//CACHE_SPACE:blks=8*sectors=4096bytes
//1G=262144
#define CACHE_SPACE 1024
#define FREE_PAGE 0
#define CLEAN_PAGE 1
#define DIRTY_PAGE 2

/*Prize Caching*/
#define ALPHA 0.5
#define MIN_PRIZE 0.0

/*LBPC*/
#define BETA 0.5
#define LBPC_TIME_PERIOD 5
// #define LBPC_COUNT_PERIOD 100

/*QD*/
// #define RETURN_QD1 99
#define HDDQD1 98
#define SSDQD1 97
#define HDDQD2 96
#define SSDQD2 95
#define DELTA 1
 
/*Other*/
#define SSD_WARM_UP 500
#define HDD_WARM_UP 501

#define SearchSSDTable -1
#endif