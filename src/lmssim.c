#include <stdio.h>    
#include <stdlib.h>
#include <sys/types.h>  
#include <sys/stat.h> 
#include <math.h> 
#include <time.h>         
#include <unistd.h> //for fork()  
#include <sys/wait.h> //for wait() 
#include "disksim_interface.h"
#include "lmssim.h"  
#include "lms_parameter.h"
#include "lms_msg.h" 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "lms_caching.h"   
 
#define DISPALY_EACH_REQUEST 0 //是否要顯示所有Request的資訊
#define Send_To_SSD 1
#define Send_To_HDD 1

// syssim_driver.c 所定義的   
static SysTime now = 0;         /* current time */
static SysTime previous_time  = 0;  
static SysTime tmp  = 0;
static SysTime next_event = -1; /* next event */
static int completed = 0;       /* last request was completed */
static Stat st;   

void panic(const char *s)
{
	perror(s);
	exit(1);
}

void add_statistics(Stat *s, double x)
{
	s->responseTime = x;
}

//印出處理完成的Request資訊
void print_statistics(Stat *s, long unsigned sysBlkbo, int bytes, const char *title)
{
	printf("\nStart Time = %f", s->startTime);
	printf("\tBlock Number = %lu", sysBlkbo);
	printf("\tResponse Time = %f", s->responseTime);
	printf("\tQueue Delay = %f\n", s->queueDelay);
}

void clear_statistics(Stat *s) 
{ 
        s->responseTime = 0;
}    
 
/*
 * Schedule next callback at time t.
 * Note that there is only *one* outstanding callback at any given time.
 * The callback is for the earliest event.
 */   
void syssim_schedule_callback(disksim_interface_callback_t fn, SysTime t, void *ctx) 
{
	next_event = t; 
	now = next_event;
}

/*
 * de-scehdule a callback.
 */
void syssim_deschedule_callback(double t, void *ctx)
{
	next_event = -1;
}

double total_SSD_Queue_Delay = 0;
double total_HDD_Queue_Delay = 0;

int ssd_time_status = 0;
//傳送給SSDsim的requst，完成後會透過此function計算request的response time及queuing delay
void ssdsim_report_completion(SysTime t, struct disksim_request *r, void *ctx)
{
	completed = 1;
	now = t; 
	//第一筆傳送給SSD的Request，不一定是從0秒開始執行
	//所以當request完成時會更新SSDsim的now值
	if (ssd_time_status == 0)
	{	
		ssd_time_status = 1;
		tmp = now;//將完成request的結束時間(now)存到tmp，用來計算之後request的queuing delay
		st.responseTime = now - r->start; //now值-start time即為此reuqest的access time
		st.queueDelay = 0; //第一筆requst的queuing delay為0
		/*
			
		--- request 1 ------
		   ^          ^
		   |          |
		 r->start    now = tmp	
		
		*/
	}
	else
	{
		previous_time = tmp;//previous_time為上一筆request的結束時間
		/*			
		--- request 1 ------
		              ^
		              |
		         previous_time = tmp		
		*/
		tmp = now;
		/*			
		--- request 1 -request 2 -----
		              ^          ^
		              |			 |	
		              |         now = tmp
                      |
		         previous_time 		
		*/
		st.responseTime = now - previous_time; //now值-start time即為此reuqest的access time
		st.queueDelay =  previous_time - st.startTime;//上次結束的時間點減去此筆request的開始時間(即arrival time)即為queuing delay
		//若queuing delay小於0，表示沒有產生queuing dleay
		if (st.queueDelay < 0)
		{
			st.queueDelay = 0;
		}
	}
}

int hdd_time_status = 0;
//傳送給HDDsim的requst，完成後會透過此function計算request的response time及queuing delay
void hddsim_report_completion(SysTime t, struct disksim_request *r, void *ctx)
{	
	//計算方法同ssdsim_report_completion
	completed = 1;
	now = t; 
	if (hdd_time_status == 0)
	{	
		hdd_time_status = 1;
		tmp = now;
		st.responseTime = now - r->start;
		st.queueDelay = 0;
		
	}
	else
	{
		previous_time = tmp;
		tmp = now;
		st.responseTime = now - previous_time;
		st.queueDelay =  previous_time - st.startTime;
		if (st.queueDelay < 0)
		{
			st.queueDelay = 0;
		}
	}
}

/*印出request的資訊*/
unsigned requestCount = 0;
unsigned ReadMissCountSSD = 0;
unsigned WriteMissCountSSD = 0;
unsigned ReadHitCountSSD = 0;
unsigned WriteHitCountSSD = 0;
unsigned tSSD = 0, tHDD = 0;
unsigned HitCntSSD = 0;
unsigned MissCntSSD = 0;

void printReq(REQ *r) 
{	
	printf(COLOR_GREEN"Request : %u\n"COLOR_RESET, requestCount);
	printf(COLOR_BLUE"[INFO]"COLOR_RESET"arrivalTime=%lf\n", r->arrivalTime);
	printf(COLOR_BLUE"[INFO]"COLOR_RESET"devno=%u\n", r->devno);
	printf(COLOR_BLUE"[INFO]"COLOR_RESET"blkno=%u\n", r->blkno);
	printf(COLOR_BLUE"[INFO]"COLOR_RESET"reqSize=%u\n", r->reqSize);
	printf(COLOR_BLUE"[INFO]"COLOR_RESET"reqFlag=%u\n", r->reqFlag);
	printf(COLOR_BLUE"[INFO]"COLOR_RESET"queuedelay=%lf\n", r->queueDelay);
	printf(COLOR_BLUE"[INFO]"COLOR_RESET"responseTime=%lf\n", r->responseTime);
	requestCount++;
}

/*複製Request，R1複製到R2*/
void copyReq(REQ *r1, REQ *r2)
{
	r2->arrivalTime = r1->arrivalTime;
	r2->devno = r1->devno;
	r2->blkno = r1->blkno;
	r2->reqSize = r1->reqSize;
	r2->reqFlag = r1->reqFlag;
	r2->queueDelay = r1->queueDelay;
	r2->responseTime = r1->responseTime;
}

pid_t SSDsimID, HDDsimID;
char *par[5];

double total_SSD_Response_Time = 0;
double total_HDD_Response_Time = 0;

unsigned ssdRead = 0;
unsigned ssdWrite = 0;
unsigned hddRead = 0;
unsigned hddWrite = 0;

void initSSDsim(const char *parm_file, const char *output_file)
{	
	printf(COLOR_CYAN"Start SSDsim\nSSDsim ID : %d\nParameter File : %s\nOutput File : %s\n"COLOR_RESET, SSDsimID, parm_file, output_file);	

	struct disksim_interface *disksim;
	struct disksim_request ssdqueue;
	struct stat buf;

	if (stat(parm_file, &buf) < 0)
	{
		panic(parm_file);
	}

	disksim = disksim_interface_initialize(parm_file,
		output_file,
		ssdsim_report_completion,
		syssim_schedule_callback,
		syssim_deschedule_callback,
		0,
		0,
		0); 

	REQ *send2main = NULL;
	send2main = malloc(sizeof(REQ));
	send2main->reqFlag = SSD_WARM_UP;
	if (sendRequestByMSQ(MSQ_KEY_MAIN, send2main, MSG_TYPE_MAIN) == -1)
	{
		printf(COLOR_RED"A request not sent to MSQ\n"COLOR_RESET);
	}
	free(send2main);

	REQ *reqSSD = NULL;
	reqSSD = malloc(sizeof(REQ));
	for (;;)
	{	
		// getchar();
		if(recvRequestByMSQ(MSQ_KEY_SSD, reqSSD, MSG_TYPE_SSD) == -1)
		{
			printf(COLOR_RED"[SSD]A request not received from MSQ\n"COLOR_RESET);
		}
		// printReq(reqSSD);
		
		if (reqSSD->reqFlag == 1)
		{
			ssdRead++;
			ssdqueue.flags = DISKSIM_READ;
		}
		else if (reqSSD->reqFlag == 0)
		{
			ssdWrite++;
			ssdqueue.flags = DISKSIM_WRITE;
		}
		else
		{
			printf(COLOR_BLUE"[SSD]Read Count : %u\n"COLOR_RESET, ssdRead);
			printf(COLOR_BLUE"[SSD]Write Count : %u\n"COLOR_RESET, ssdWrite);
			printf(COLOR_BLUE"[SSD]Total Response Time = %f\n"COLOR_RESET, total_SSD_Response_Time);	
			printf(COLOR_BLUE"[SSD]Total Queue Delay = %f\n"COLOR_RESET, total_SSD_Queue_Delay);	
			disksim_interface_shutdown(disksim, now);
			exit(0);
		}

		ssdqueue.start = reqSSD->arrivalTime;
		ssdqueue.devno = reqSSD->devno;
		ssdqueue.blkno = reqSSD->blkno;//*SSD_BLOCK2SECTOR;
		ssdqueue.bytecount = reqSSD->reqSize*DISKSIM_SECTOR;

		if (now<ssdqueue.start)
		{
			now = ssdqueue.start;
			tmp = now;
		}

		completed = 0;
		disksim_interface_request_arrive(disksim, now, &ssdqueue);

		while(next_event >= 0)
		{
			st.startTime = reqSSD->arrivalTime; 
			now = next_event;
			next_event = -1;
			disksim_interface_internal_event(disksim, now, 0);
		}

		if (!completed)
		{
			fprintf(stderr, "[SSD]internal error. Last event not completed\n");
			exit(1);
		}
		else
		{	
			// each request
			#if DISPALY_EACH_REQUEST
				printf("[SSD]");
				print_statistics(&st, ssdqueue.blkno, ssdqueue.bytecount, "Response Time");
			#endif

			//total request 
			total_SSD_Response_Time += st.responseTime;
			total_SSD_Queue_Delay += st.queueDelay;
			clear_statistics(&st);
		}
	}
}

void initHDDsim(const char *parm_file, const char *output_file)
{
	printf(COLOR_MAGENTA"Start HDDsim\nHDDsim ID : %d\nParameter File : %s\nOutput File : %s\n"COLOR_RESET, HDDsimID, parm_file, output_file);	

	struct disksim_interface *disksim;
	struct disksim_request hddqueue;
	struct stat buf;

	if (stat(parm_file, &buf) < 0)
	{
		panic(parm_file);
	}

	disksim = disksim_interface_initialize(parm_file,
		output_file,
		hddsim_report_completion,
		syssim_schedule_callback,
		syssim_deschedule_callback,
		0,
		0,
		0); 

	REQ *send2main = NULL;
	send2main = malloc(sizeof(REQ));
	send2main->reqFlag = HDD_WARM_UP;
	if (sendRequestByMSQ(MSQ_KEY_MAIN, send2main, MSG_TYPE_MAIN) == -1)
	{
		printf(COLOR_RED"A request not sent to MSQ\n"COLOR_RESET);
	}
	free(send2main);


	REQ *reqHDD = NULL;
	reqHDD = malloc(sizeof(REQ));	
	for (;;)
	{
		if(recvRequestByMSQ(MSQ_KEY_HDD, reqHDD, MSG_TYPE_HDD) == -1)
		{
			printf(COLOR_RED"[HDD]A request not received from MSQ\n"COLOR_RESET);
		}
		// printReq(reqHDD);

		if (reqHDD->reqFlag == 1)
		{
			hddRead++;
			hddqueue.flags = DISKSIM_READ;
		}
		else if (reqHDD->reqFlag == 0)
		{
			hddWrite++;
			hddqueue.flags = DISKSIM_WRITE;
		}
		else
		{	
			printf(COLOR_GREEN"[HDD]Read Count : %u\n"COLOR_RESET, hddRead);
			printf(COLOR_GREEN"[HDD]Write Count : %u\n"COLOR_RESET, hddWrite);
			printf(COLOR_GREEN"[HDD]Total Response Time = %f\n"COLOR_RESET, total_HDD_Response_Time);
			printf(COLOR_BLUE"[HDD]Total Queue Delay = %f\n"COLOR_RESET, total_HDD_Queue_Delay);				
			disksim_interface_shutdown(disksim, now);
			exit(0);
		}

		hddqueue.start = reqHDD->arrivalTime;
		hddqueue.devno = reqHDD->devno;
		hddqueue.blkno = reqHDD->blkno;
		hddqueue.bytecount = reqHDD->reqSize*DISKSIM_SECTOR;

		if (now<hddqueue.start)
		{
			now = hddqueue.start;
			tmp = now;
		}

		completed = 0;
		disksim_interface_request_arrive(disksim, now, &hddqueue);

		while(next_event >= 0)
		{	
			st.startTime = reqHDD->arrivalTime;
			now = next_event;
			next_event = -1;
			disksim_interface_internal_event(disksim, now, 0);
		}

		if (!completed)
		{
			fprintf(stderr, "[HDD]internal error. Last event not completed\n");
			exit(1);
		}
		else
		{	 
			// each request
			#if DISPALY_EACH_REQUEST 
				printf("[HDD]");    
				print_statistics(&st, hddqueue.blkno, hddqueue.bytecount, "Response Time");
			#endif 
			// total request
			total_HDD_Response_Time +=st.responseTime;
			total_HDD_Queue_Delay += st.queueDelay;
			clear_statistics(&st);
		}
	}
}

void initDisksim()
{
	pid_t processID;

	processID = fork();
	if (processID == 0) 
	{
	 	SSDsimID = getpid();
	 	initSSDsim(par[1], par[2]);
		exit(0);
 	}
	else if (processID < 0)
	{ 
		printf(COLOR_RED"SSDsim process fork error\n"COLOR_RESET);
	}

	processID = fork();
	if (processID == 0) 
	{
	 	HDDsimID = getpid();	 	
	 	initHDDsim(par[3], par[4]);
		exit(0);
 	}
	else if (processID < 0)
	{
		printf(COLOR_RED"HDDsim process fork error\n"COLOR_RESET);
	}

	initMSQ();
}

void uninitDisksim()
{	
	REQ *finishSimulator;
	finishSimulator = malloc(sizeof(REQ));
	finishSimulator->reqFlag = 2;	

	if (sendRequestByMSQ(MSQ_KEY_SSD, finishSimulator, MSG_TYPE_SSD) == -1)
	{
		printf(COLOR_RED"A end signal not sent to SSD\n"COLOR_RESET);
	}
	if (sendRequestByMSQ(MSQ_KEY_HDD, finishSimulator, MSG_TYPE_HDD) == -1)
	{  
		printf(COLOR_RED"A end signal not sent to MSQ\n"COLOR_RESET);
	}	
} 

FILE *trace;
int startSSD = 0;
int startHDD = 0;  
int SSD_Evtion=0; 
//4KB = 1024*4Byte

int max(int x, int y)
{
	return (x>=y?x:y);
}
int min(int x, int y)
{
	return (x<=y?x:y);
}

int main(int argc, char *argv[])
{	
	if (argc != 5)
	{
		fprintf(stderr, COLOR_RED"usage: %s <Trace file> <param file for SSD> <output file for SSD> <param file for HDD> <output file for HDD>\n"COLOR_RESET, argv[0]);
		exit(1);
	}
	int i, j;
	printf(COLOR_YELLOW"Start Simulation\n"COLOR_RESET);
	par[1] = argv[1];
	par[2] = argv[2];
	par[3] = argv[3];
	par[4] = argv[4];
	//初始化Disksim
	initDisksim();
	printf("initDisksim\n");
	//等待SSDsim和HDDsim啟動完成，避免尚未啟動就傳送request
	REQ *status = NULL;
	status = malloc(sizeof(REQ));
	while(startSSD == 0 || startHDD == 0)
	{		
		if(recvRequestByMSQ(MSQ_KEY_MAIN, status, MSG_TYPE_MAIN) == -1)
		{
			printf(COLOR_RED"[MAIN]A request not received from MSQ\n"COLOR_RESET);
		}
		if(status->reqFlag == SSD_WARM_UP)
		{	
			// printf("SSD warm up finish\n");
			startSSD = 1;
		}
		else if(status->reqFlag == HDD_WARM_UP)
		{
			// printf("HDD warm up Finish\n");
			startHDD = 1;
		}
	}
	free(status);
	REQ *tmp;
	tmp = malloc(sizeof(REQ));
	
	char buffer[] = "0 0 8 16 0";
	sscanf(buffer, "%lf%u%u%u%u",
					&tmp->arrivalTime,
					&tmp->devno,
					&tmp->blkno,
					&tmp->reqSize, 
					&tmp->reqFlag );
	//printReq(tmp);			
	int blockCount;
	blockCount = tmp->reqSize/SSD_PAGE2SECTOR;	//8
	for(i=0; i<blockCount; i++)
	{
		if (i != 0)//設定request的lpn
		{
			tmp->blkno += SSD_PAGE2SECTOR;
		}
		tmp->reqSize = SSD_PAGE2SECTOR;//設定request的size(即為SSD page size) 

		//Send_To_HDD
		#if Send_To_HDD
			// TEST_HDDSIM, 傳送給HDDsim的處理流程
			REQ *send2hdd;
			send2hdd = malloc(sizeof(REQ));
			copyReq(tmp, send2hdd);	
			// printf("->HDD: no-%u size-%u f-%u", send2hdd->blkno, send2hdd->reqSize, send2hdd->reqFlag);
			if (sendRequestByMSQ(MSQ_KEY_HDD, send2hdd, MSG_TYPE_HDD) == -1)
			{
				printf(COLOR_RED"A request not sent to HDD"COLOR_RESET);
			}
		#endif
		
		//Send_To_HDD
		#if Send_To_SSD
			REQ *send2ssd;
			send2ssd = malloc(sizeof(REQ));
			copyReq(tmp, send2ssd);	
			unsigned long ssdBlkno;

			// TEST_SSDSIM, 傳送給SSDsim的處理流程
			//嘗試放進Cache，若Cache未滿，便透過message queue傳送到SSDsim處理
			if(caching(&ssdBlkno, &send2ssd->blkno, 2) != -1)
			{ 	
				send2ssd->blkno = ssdBlkno*SSD_BLOCK2SECTOR;
				if (sendRequestByMSQ(MSQ_KEY_SSD, send2ssd, MSG_TYPE_SSD) == -1)
				{
					printf(COLOR_RED"A request not sent to SSD\n"COLOR_RESET);
				}
			}  
			//若Cache已滿
			else
			{
				SSD_Evtion=1;
				evictCache(-1,0);//先剔除一個Block，-1為依照LRU規則剔除，其他數字為剔除特定的Block Number 					
				//再次嘗試放進Cache，透過message queue傳送到SSDsim處理
				if(caching(&ssdBlkno, &send2ssd->blkno, 2) != -1)
				{	
					send2ssd->blkno = ssdBlkno*SSD_BLOCK2SECTOR;
					// printReq(send2ssd);
					if (sendRequestByMSQ(MSQ_KEY_SSD, send2ssd, MSG_TYPE_SSD) == -1)
					{
						printf(COLOR_RED"A request not sent to SSD\n"COLOR_RESET);
					}  
				}
			}
		#endif
	}  
	uninitDisksim();
	wait(NULL);
	wait(NULL);
	uninitMSQ();
	// displayCacheTable();	
	printf(COLOR_YELLOW"End Simulation\n"COLOR_RESET);
	return 0;
}
