#ifndef LMS_MSG_H
#define LMS_MSG_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <errno.h>

#include "lmssim.h"
#include "lms_parameter.h"

#define MSG_SIZE sizeof(MSGBUF)-sizeof(long int)

void initMSQ();
void uninitMSQ();
int createMessageQueue(key_t key); 
int removeMessageQueue(key_t key, struct msqid_ds *msq_id);
int sendRequestByMSQ(key_t key, REQ *r, long int msgtype);
int recvRequestByMSQ(key_t key, REQ *r, long int msgtype);

#endif