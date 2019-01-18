#ifndef LMSSIM_H
#define LMSSIM_H 

// syssim_driver.h 所定義的
typedef double SysTime;         /* system time in seconds.usec */

typedef struct
{
	// int n;
	// double sum;
	// double sqr;
	double startTime;
	double responseTime;
	double queueDelay;
}Stat;

typedef struct req
{
	double arrivalTime;
	unsigned devno;
	unsigned blkno;
	unsigned reqSize;
	unsigned reqFlag; //0:Write 1:Read
	double queueDelay;
	double responseTime;
}REQ;

void printReq(REQ *r);
void copyReq(REQ *r1, REQ *r2);

#endif