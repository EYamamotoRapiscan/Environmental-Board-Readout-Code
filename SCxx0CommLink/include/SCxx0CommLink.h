#ifndef SCXXXCOMMLINK
#define SCXXXCOMMLINK

/****************************************************************
 * Includes
 ****************************************************************/

#define MSGSZ     128

#define SCMSGLEN 34

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include<stdio.h>
#include<stdlib.h>
#include<string.h>	//strlen
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr


typedef struct msgbuf {
    long    mtype;
    char    mtext[MSGSZ];
} message_buf;


#define MSGQKEY	1337


#endif /* SCXXXCOMMLINK */

