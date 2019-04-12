#include "../include/SCxx0CommLink.h"


#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>

#include <sys/ioctl.h>
#include <linux/usbdevice_fs.h>


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
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

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

main(int argc, char * argv[])
{
	int msqid;
	key_t key;
	message_buf  rbuf;

	int continuous = 0;
	if(argc > 1)
	{
		continuous = 1;
	}

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	FILE *fl = fopen("/home/debian/Rapiscan.log", "a");
	if(fl != NULL)
	{
		fprintf(fl, "Starting %s, %d-%d-%d %d:%d:%d \n", argv[0], tm.tm_mon + 1, tm.tm_mday, tm.tm_year, tm.tm_hour, tm.tm_min, tm.tm_sec);
		fclose(fl);;
	}
	/*
	 * Get the message queue id for the
	 * "name" 1234, which was created by
	 * the server.
	 */
	key = MSGQKEY;

	if ((msqid = msgget(key, 0666)) < 0) {
		perror("msgget");
		exit(1);
	}


	/*
	 * Receive an answer of message type 1.
	 */
	//    if (msgrcv(msqid, &rbuf, MSGSZ, -10000, IPC_NOWAIT) < 0) {
	//        perror("msgrcv");
	//        exit(1);
	//    }

	char device[32] = "/dev/ttyUSB0";
	printf("%s\n",device);

	int fd;
//reset usb link every time we start this...
	printf("Resetting USB link\n");
	fd = open(device, O_WRONLY);
	ioctl(fd, USBDEVFS_RESET, 0);
	close(fd);

	fd = open(device, O_WRONLY | O_NOCTTY | O_SYNC);
	if (fd < 0) {
		printf("Error opening %s: %s\n", device, strerror(errno));
		return -1;
	}
	printf("setting interface...\n");
	set_interface_attribs (fd, B19200, PARENB|PARODD);
	set_blocking (fd, 0);                // set no blocking


	while(1)
	{
		while (msgrcv(msqid, &rbuf, MSGSZ, -10000, IPC_NOWAIT) > 0) {



			//printf(rbuf.mtext);
			/*
			 * Print the answer.
			 */

			//char configuration[256];


			//char message[256];


			printf("%s\n", rbuf.mtext);

			//int wlen;


			printf("Writing %s to serial\n",rbuf.mtext);

			int nbytes = 0;

			ssize_t bytes = write (fd, rbuf.mtext, strlen(rbuf.mtext));

		}
		if(continuous)
		{
			usleep(200000);
		}
		else break;
	}
	close(fd);
	perror("msgrcv");
	exit(0);
}
