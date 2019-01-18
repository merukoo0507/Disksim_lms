#include "lms_msg.h"

typedef struct msgbuf 
{
	long int msgType;
	REQ req;
}MSGBUF;

/*
 * 初始化Messaage Queue
 * 分別建立傳送到Main,SSDsim和HDDsim的message queue
 */
void initMSQ()
{
	if(createMessageQueue(MSQ_KEY_SSD) == -1)
	{
		printf(COLOR_RED"create message queue for SSD error\n"COLOR_RESET);		
	}

	if(createMessageQueue(MSQ_KEY_HDD) == -1)
	{
		printf(COLOR_RED"create message queue for HDD error\n"COLOR_RESET);
	}

	if(createMessageQueue(MSQ_KEY_MAIN) == -1)
	{
		printf(COLOR_RED"create message queue for MAIN error\n"COLOR_RESET);
	}
}
/*
 * 程式結束時，刪除先前建立的Message Queue
 * 避免Message Queue產生錯誤，影響下一次實驗 
 */
void uninitMSQ()
{
	struct msqid_ds ds;

	if(removeMessageQueue(MSQ_KEY_SSD, &ds) == -1)
	{
		printf(COLOR_RED"Not remove message queue for SSD\n"COLOR_RESET);
	}

	if(removeMessageQueue(MSQ_KEY_HDD, &ds) == -1)
	{
		printf(COLOR_RED"Not remove message queue for HDD\n"COLOR_RESET);
	}

	if(removeMessageQueue(MSQ_KEY_MAIN, &ds) == -1)
	{
		printf(COLOR_RED"Not remove message queue for MAIN\n"COLOR_RESET);
	}
}

/*建立Message Queue*/
int createMessageQueue(key_t key) 
{
	int msqid;
	msqid = msgget(key, IPC_CREAT | 0666);
	// printf("msgid = %d\n", msqid);
	if(msqid >= 0) 
	{               
		return msqid;
	}
	else
	{
		fprintf(stderr,"Creat Message Error：%s\a\n",strerror(errno));
		return -1;
	}
}

/*刪除Message Queue*/
int removeMessageQueue(key_t key, struct msqid_ds *msq_id)
{
	int msqid;
	msqid = msgget((key_t)key, IPC_CREAT);
	if(msgctl(msqid, IPC_RMID, msq_id) == 0)
	{
		return msqid;
	}
	else
	{
		return -1;            
	}
}

/*透過Message Queue傳送Request*/
int sendRequestByMSQ(key_t key, REQ *r, long int msgtype)
{
	int msqid;
	msqid = msgget((key_t)key, IPC_CREAT);

	MSGBUF msg;

	msg.msgType = msgtype;
	msg.req.arrivalTime = r->arrivalTime;
	msg.req.devno = r->devno;
	msg.req.blkno = r->blkno;
	msg.req.reqSize = r->reqSize;
	msg.req.reqFlag = r->reqFlag;
	msg.req.queueDelay = r->queueDelay;
	msg.req.responseTime = r->responseTime;

	if(msgsnd(msqid, (void *)&msg, MSG_SIZE, 0) == 0)
	{
		return 0;
	 }
	 else
	 {   	
	 	fprintf(stderr,"Sent Message Error：%s\a\n",strerror(errno));
	 	return -1;		
	 }
}

/*透過Message Queue接收Request*/
int recvRequestByMSQ(key_t key, REQ *r, long int msgtype)
{
	int msqid;
	msqid = msgget((key_t)key, IPC_CREAT);

	MSGBUF buf;

	if(msgrcv(msqid, (void *)&buf, MSG_SIZE, msgtype, 0) >= 0)
	{
		r->arrivalTime = buf.req.arrivalTime;
		r->devno = buf.req.devno;
		r->blkno = buf.req.blkno;
		r->reqSize = buf.req.reqSize;
		r->reqFlag = buf.req.reqFlag;
		r->queueDelay = buf.req.queueDelay;
		r->responseTime = buf.req.responseTime;
		return 0;
	}
	else
	{
		return -1;
	}
}