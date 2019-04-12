#include "../include/EmMessageController.h"
#include "MessageStruct.h"
#include <assert.h>
#include <time.h>

char hexDigit(unsigned n)
{
	if (n < 10) {
		return n + '0';
	} else {
		return (n - 10) + 'A';
	}
}

void charToHex(char c, char hex[3])
{
	hex[0] = hexDigit(c / 0x10);
	hex[1] = hexDigit(c % 0x10);
	hex[2] = '\0';
}

int getSensorMessage(struct sockaddr_in* server,
		SensorMessage* m)
{
	int socket_desc;
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}

	struct timeval tv;
	tv.tv_sec = 1;  /* 30 Secs Timeout */
	tv.tv_usec = 0;  // Not init'ing this can cause strange errors
	setsockopt(socket_desc, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));


	char *message, server_reply[44];
	char header[5] = { 255, 0, 170, 85, 14 };
	// D.E.M Command   0xFF00AA55 0E//Connect to remote server
	if (connect(socket_desc, (struct sockaddr*) &*server, sizeof(*server))
			< 0) {
		puts("connect error\n");
		close(socket_desc);
		return -1;
	}
	puts("Connected\n");
	//Send some data
	if (send(socket_desc, header, 5, 0) < 0) {
		puts("Send failed ");
		close(socket_desc);
		return -1;
	}

	int i;
	int size;
	char foo[ENVMSGLEN];
	int totalbytes = 0;

	while ((size = recv(socket_desc, server_reply, 1, 0) > 0)) {

		foo[totalbytes] = server_reply[0];
		totalbytes += size;
		if (totalbytes >= ENVMSGLEN) {
			break;
		}
	}
	puts("Reply received\n ");

	close(socket_desc);

	char bar[5];
	bar[4] = '\0';
	char hex[3];
	char hexMessage[9];
	hexMessage[8] = '\0';
	int j;
	long value[12];
	//parse 36 byte message into 9 4-byte values
	for (i = 0; i < 12; i++) {
		int k = 8;
		for (j = 0; j < 4; j++)        //reorder bytes
		{
			int index = i * 4 + j;
			charToHex(foo[index], hex);
			hexMessage[k - 2] = hex[0];
			hexMessage[k - 1] = hex[1];
			k -= 2;
		}
		value[i] = strtol(hexMessage, NULL, 16);
		printf("message %i, %s,\t%ld\n", i, hexMessage, value[i]);
	}
	fillMessage(value, m);
	close(socket_desc);
	return 0;
}

void SendQMessage(char *qmsg, int msqid) {
	message_buf sbuf;
	sbuf.mtype = ENVMESSTYPE;
	(void) strcpy(sbuf.mtext, qmsg);
	//size_t buf_length = strlen(sbuf.mtext) + 1;
	size_t buf_length = strlen(sbuf.mtext) + 1;
	/*
	 * Send a message.
	 */
	if (msgsnd(msqid, &sbuf, buf_length, IPC_NOWAIT) < 0) {
		printf("%d, %d, %s, %d\n", msqid, sbuf.mtype, sbuf.mtext, buf_length);
		perror("msgsnd");
	} else
		printf("Message Sent To Queue: %s\n", sbuf.mtext);
}

void FormatAndSendMessages(SensorMessage *SMA, int msqid)
{
	int i;
	//format messages to send over wire
	//first 4 are detectors
	char tempMessage[40];
	char pressMessage[40];
	char humidityMessage[40];
	char dewPointMessage[40];
	char masterMessage[40];
	char slaveMessage[40];
	char battMessage[40];
	char masterBattMess[40];
	char slaveBattMess[40];

	char prepend = '!';
	char append[4] = "\r\n#";

	sprintf(tempMessage,"%cTPDT,%05.1f,%05.1f,%05.1f,%05.1f,0%s"
			,prepend
			,SMA[0].temp1
			,SMA[1].temp1
			,SMA[2].temp1
			,SMA[3].temp1
			,append
	);

	sprintf(humidityMessage,"%cRHDT,%05.1f,%05.1f,%05.1f,%05.1f,0%s"
			,prepend
			,SMA[0].humidity
			,SMA[1].humidity
			,SMA[2].humidity
			,SMA[3].humidity
			,append
	);

	sprintf(pressMessage,"%cPRES,%05i,%05i,%05i,%05i,0%s"
			,prepend
			,SMA[0].pressure
			,SMA[1].pressure
			,SMA[2].pressure
			,SMA[3].pressure
			,append
	);

	sprintf(dewPointMessage,"%cDEWP,%05.1f,%05.1f,%05.1f,%05.1f,0%s"
			,prepend
			,SMA[0].dewPoint
			,SMA[1].dewPoint
			,SMA[2].dewPoint
			,SMA[3].dewPoint
			,append
	);

	sprintf(masterMessage,"%cMTRH,%05.1f,%05.1f,%05.1f,%05.1f,0%s"
			,prepend
			,SMA[4].temp1
			,SMA[4].humidity
			,SMA[4].pillarThermocouple
			,SMA[4].pillarExtHumidity
			,append
	);

	sprintf(slaveMessage,"%cSTRH,%05.1f,%05.1f,%05.1f,%05.1f,0%s"
			,prepend
			,SMA[5].temp1
			,SMA[5].humidity
			,SMA[5].pillarThermocouple
			,SMA[5].pillarExtHumidity
			,append
	);
	sprintf(masterBattMess,"%cMBAT,%05.1f,%05.1f,00000,%05.1f,0%s"
			,prepend
			,SMA[4].vBatt
			,SMA[4].dcCurrent
			,SMA[4].acCurrent
			,append
	);
	sprintf(slaveBattMess,"%cSBAT,%05.1f,%05.1f,00000,%05.1f,0%s"
			,prepend
			,SMA[5].vBatt
			,SMA[5].dcCurrent
			,SMA[5].acCurrent
			,append
	);

	if(SMA[0].vm || SMA[1].vm || SMA[2].vm ||  SMA[3].vm)
	{
		SendQMessage(tempMessage, msqid);
		SendQMessage(humidityMessage, msqid);
		SendQMessage(pressMessage, msqid);
		SendQMessage(dewPointMessage,msqid);
	}
	if(SMA[4].vm)
		{
		SendQMessage(masterBattMess, msqid);
		SendQMessage(masterMessage,msqid);
		}

	if(SMA[5].vm)
		{
		SendQMessage(slaveBattMess, msqid);
		SendQMessage(slaveMessage, msqid);
		}


	char detMsg[128];
	for(i =6; i < MAXIPS; i++)
	{ //loop through and send out any additional detector information.
		if(SMA[i].vm == 1)
		{
			printf("Detector %i Message vm = %i\n",i, SMA[i].vm);
			sprintf(detMsg,"%cD%i,%05.1f,%05.1f,%05i,%05.1f,%05.1f,%05.1f,%05.1f%s"
					,prepend
					,i
					,SMA[i].temp1
					,SMA[i].humidity
					,SMA[i].pressure
					,SMA[i].temp2
					,SMA[i].vBatt
					,SMA[i].dcCurrent
					,SMA[i].acCurrent
					,append
			);
			SendQMessage(detMsg,msqid);
		}
	}
	/*	char testmessage[9] = "0000fffe";
long testOut = strtol(testmessage, NULL, 16);
printf("test negative value: %f\n", signedTemp(testOut));*/

	printf("%s\n",tempMessage);
	printf("%s\n",humidityMessage);
	printf("%s\n",pressMessage);
	printf("%s\n",masterMessage);
	printf("%s\n",slaveMessage);
	printf("%s\n",masterBattMess);
	printf("%s\n",slaveBattMess);
}

int main(int argc , char *argv[])
{

	//setup for message queue
	int msqid;
	int msgflg = IPC_CREAT | 0666;
	key_t key;
	int i;

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	/*
	FILE *fl = fopen("/home/debian/Rapiscan.log", "a");
	fprintf(fl, "Starting %s, %d-%d-%d %d:%d:%d \n", argv[0], tm.tm_mon + 1, tm.tm_mday, tm.tm_year, tm.tm_hour, tm.tm_min, tm.tm_sec);
	fclose(fl);
	*/
	/*
	 * Get the message queue id for the
	 * "name" 1234, which was created by
	 * the server.
	 */
	key = MSGQKEY;

	(void) fprintf(stderr, "\nmsgget: Calling msgget(%#lx,\%#o)\n",key, msgflg);

	if ((msqid = msgget(key, msgflg )) < 0) {
		perror("msgget");
		exit(1);
	}
	else
		(void) fprintf(stderr,"msgget: msgget succeeded: msqid = %d\n", msqid);



	printf("polling devices...\n");
	//default ip range
	int ipaddresses[MAXIPS];
	int nip = 1;
	int continuousRun = 0; //flag for continuous scan;

	//ip address scheme for rpm:
	// 1 to 4 are detectors 1 to 4
	// 5 is master
	// 6 is slave
	for(i = 0; i <MAXIPS; i++)
	{
		ipaddresses[i] = 0;
	}


	if(argc > 1)
	{
		//user is specifying IP addresses
		nip = argc - 1;

		for(i = 1; i < argc; i++)
		{
			printf("%s\n",argv[i]);
			int tip = atoi(argv[i]);
			assert(tip != 100);//reserved for beaglebone
			assert(tip < 255);
			if(tip <=0)
			{
				// flag to allow for infinite loop
				tip = -tip < 1 ? 1: -tip;
				printf("Configuring to run continuously, poll every %i seconds\n", tip);
				continuousRun = tip;

			}
			else ipaddresses[tip-1] = tip; //no checks no for valid argc value.
		}
	}
	//	else 	ipaddresses[5] = 6; //default device
	else
	{
		printf("usage: specify at least one ip address space delimited.\nIf you want to run continuously, specify the number of seconds as a negative number.\n");
		exit(0);
	}
	SensorMessage *SMA = malloc(nip*sizeof(*SMA));



	struct sockaddr_in server;
	//Create socket
	//socket_desc = socket(AF_INET , SOCK_DGRAM , 0);

	char ipadd[16];
	int connectionvalid;

	int iteration = 0; //track the number of iterations. It seems this application hangs after a while. so restart it after a while.

	while(1)
	{
		iteration++;
		if(iteration >= 10)
		{
			//restart
			//FILE *fl = fopen("/home/debian/Rapiscan.log", "a");
			//fprintf(fl, "Re-starting %s, %d-%d-%d %d:%d:%d \n", argv[0], tm.tm_mon + 1, tm.tm_mday, tm.tm_year, tm.tm_hour, tm.tm_min, tm.tm_sec);
			//fclose(fl);
			iteration = 0; //not necessary since new process starts
			if(execv(argv[0], argv))
			{
				//failed to open.
				//FILE *fl = fopen("/home/debian/Rapiscan.log", "a");
				//fprintf(fl, "Failed re-starting %s, %d-%d-%d %d:%d:%d \n", argv[0], tm.tm_mon + 1, tm.tm_mday, tm.tm_year, tm.tm_hour, tm.tm_min, tm.tm_sec);
				//fclose(fl);
				exit(-1);
			}
		}
		message_buf  rbuf;
		//clear message queue of any existing messages from this application
		while(msgrcv(msqid, &rbuf, MSGSZ, ENVMESSTYPE, IPC_NOWAIT) > 0) {
			printf("clearing buffer: %s\n",rbuf.mtext);
			usleep(100000);
		}


		for(i = 0; i < MAXIPS; i++) //get messages from each device
		{
			initializeSensorMessage(&SMA[i]); //set values to default
			connectionvalid = -1;
			if(ipaddresses[i] >0)
			{
				//initializeSensorMessage(&SMA[i]); //set values to default
				sprintf(ipadd,"192.168.10.%i",ipaddresses[i]);
				server.sin_addr.s_addr = inet_addr(ipadd);
				server.sin_family = AF_INET;
				server.sin_port = htons( 7 );


				connectionvalid = getSensorMessage(&server, &SMA[i]);
				if(connectionvalid == 0)
				{
					printf("Data from Sensor at %s\n",ipadd);
					//SMA[i].vm = 1;
					printMessage(&SMA[i]);
				}
			}

		}

			FormatAndSendMessages(SMA, msqid);

		if(!continuousRun) break; //run once
		sleep(continuousRun);
	}
	//free(SMA);
	exit(0);



}
