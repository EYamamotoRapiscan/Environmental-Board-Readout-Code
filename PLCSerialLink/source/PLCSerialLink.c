#include "../include/PLCSerialLink.h"
#include "../../SCxx0CommLink/include/SCxx0CommLink.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>

int
set_interface_attribs (int fd, int speed, int parity)
{
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	if (tcgetattr (fd, &tty) != 0)
	{
		printf("Error from tcgetattr: %s\n", strerror(errno));
		return -1;
	}

	cfsetospeed (&tty, speed);
	cfsetispeed (&tty, speed);

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
	// disable IGNBRK for mismatched speed tests; otherwise receive break
	// as \000 chars
	tty.c_iflag &= ~IGNBRK;         // disable break processing
	tty.c_lflag = 0;                // no signaling chars, no echo,
	// no canonical processing
	tty.c_oflag = 0;                // no remapping, no delays
	tty.c_cc[VMIN]  = 0;            // read doesn't block
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

	tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
	// enable reading
	tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
	tty.c_cflag |= parity;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;

	if (tcsetattr (fd, TCSANOW, &tty) != 0)
	{
		printf("Error from tcsetattr: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

void
set_blocking (int fd, int should_block)
{
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	if (tcgetattr (fd, &tty) != 0)
	{
		printf("Error tcgetattr: %s\n", strerror(errno));
		return;
	}

	tty.c_cc[VMIN]  = should_block ? 1 : 0;
	tty.c_cc[VTIME] = 10;            // 0.5 seconds read timeout

	if (tcsetattr (fd, TCSANOW, &tty) != 0)
		printf("Error tcsetattr: %s\n", strerror(errno));
}

void set_mincount(int fd, int mcount)
{
	struct termios tty;

	if (tcgetattr(fd, &tty) < 0) {
		printf("Error tcgetattr: %s\n", strerror(errno));
		return;
	}

	tty.c_cc[VMIN] = mcount ? 1 : 0;
	tty.c_cc[VTIME] = 5;        /* half second timer */

	if (tcsetattr(fd, TCSANOW, &tty) < 0)
		printf("Error tcsetattr: %s\n", strerror(errno));
}



void send_popen(char *command)
{
	FILE *pf;
	//char command[20];
	//char data[512];



	// Setup our pipe for reading and execute our command.
	pf = popen(command,"w");

	if (pf == NULL)
		printf("\nFAILED!!!\n");
	// Error handling

	if (pclose(pf) != 0)
		fprintf(stderr," Error: Failed to close command stream \n");

	return;
}

void SendPLCQMessage(char *qmsg, int msqid) {
	message_buf sbuf;
	sbuf.mtype = PLCMSGTYPE;
	(void) strcpy(sbuf.mtext, qmsg);
	//size_t buf_length = strlen(sbuf.mtext) + 1;
	size_t buf_length = strlen(sbuf.mtext) + 1;
	printf("Buf length %i",buf_length);
	/*
	 * Send a message.
	 */
	if (msgsnd(msqid, &sbuf, buf_length, IPC_NOWAIT) < 0) {
		printf("%d, %d, %s, %d\n", msqid, sbuf.mtype, sbuf.mtext, buf_length);
		perror("msgsnd");
	} else
		printf("Message Sent To Queue: %s\n", sbuf.mtext);
}

main(int argc, char * argv[])
{
	//setup for message queue
	int msqid;
	int msgflg = IPC_CREAT | 0666;
	key_t key;
	int i;

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	FILE *fl = fopen("/home/debian/Rapiscan.log", "a");
	if(fl != NULL)
	{
	fprintf(fl, "Starting %s, %d-%d-%d %d:%d:%d \n", argv[0], tm.tm_mon + 1, tm.tm_mday, tm.tm_year, tm.tm_hour, tm.tm_min, tm.tm_sec);
	fclose(fl);;
	}
	key = MSGQKEY;

	(void) fprintf(stderr, "\nmsgget: Calling msgget(%#lx,\
%#o)\n",
key, msgflg);

	if ((msqid = msgget(key, msgflg )) < 0) {
		perror("msgget");
		exit(1);
	}
	else
		(void) fprintf(stderr,"msgget: msgget succeeded: msqid = %d\n", msqid);





	char device[32] = "/dev/ttyO4";
	char configuration[256];

	//printf("%s\n",configuration);
	char message[256];

	message_buf  rbuf;
	//clear message queue of any existing messages from this application
	while(msgrcv(msqid, &rbuf, MSGSZ, PLCMSGTYPE, IPC_NOWAIT) > 0) {
		printf("clearing buffer: %s\n",rbuf.mtext);
		usleep(100000);
	}
	printf("%s\n", rbuf.mtext);

	int fd;
	int wlen;

	fd = open(device, O_RDONLY | O_NOCTTY | O_SYNC);
	if (fd < 0) {
		printf("Error opening %s: %s\n", device, strerror(errno));
		return -1;
	}
	printf("setting interface...\n");
	set_interface_attribs (fd, B19200, PARENB|PARODD);
	set_blocking (fd, 1);                // set blocking


	char mArray[128]; //array to hold plc message
	i = 0;
	do {
		unsigned char buf[80];
		int rdlen;

		rdlen = read(fd, buf, SPDMSGLEN);
		if (rdlen > 0) {

			buf[rdlen] = 0;
			printf("Read %d: \"%s\"\n", rdlen, buf);

			unsigned char   *p;
			printf("Read %d:", rdlen);
			for (p = buf; rdlen-- > 0; p++)
			{
				mArray[i] = *p; //copy message to array
				i++;
				printf(" %x", *p);
			}
			printf("\n");

			//check for end of line characters. should be 0xd 0xa but getting 0xa 0xa
			//or check length of array;

			if(i >= 6)
			{
				if(
						(mArray[i - 2] == 0xa && mArray[i - 1] == 0xa)
						|| (mArray[i - 2] == 0xd && mArray[i - 1] == 0xa)

				)
				{

					//clear message queue of any existing messages from this application
					while(msgrcv(msqid, &rbuf, MSGSZ, PLCMSGTYPE, IPC_NOWAIT) > 0) {
						printf("clearing buffer: %s\n",rbuf.mtext);
						usleep(100000);
					}

					mArray[i] = '\0';
					//send message to message queue
					SendPLCQMessage(mArray,msqid);
					i = 0;



				}
			}

		} else if (rdlen < 0) {
			printf("Error from read: %d: %s\n", rdlen, strerror(errno));
		}
		/* repeat read to get full message */
	} while (1);


	//		if(continuous)
	//		{
	//		usleep(200000);
	//		}
	//		else break;
	close(fd);
	exit(0);
}
