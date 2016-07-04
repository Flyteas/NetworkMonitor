/*
 * Server.c
 *
 *  Created on: Jul 2, 2016
 *      Author: flyshit
 *        Mail: flyshit@cqu.edu.cn | flyteas@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <arpa/inet.h>
#include "Server.h"
#define MSG_REC_BUFFER_SIZE 1024

int main(int argc,char* argv[])
{
	int ListenPort;
	int SocketListenResult;

	if(argc < 2)
	{
		printf("Usage: ./ProgramName ListenPort\n");
		printf("Example: ./Server 8888\n");
		return 0;
	}
	ListenPort = atoi(argv[1]);
	SocketListenResult = SocketListen(ListenPort);
	if(SocketListenResult == 1)
	{
		printf("Create socket falid\n");
		return 0;
	}
	if(SocketListenResult == 2)
	{
		printf("Socket bind falid\n");
		return 0;
	}
	if(SocketListenResult == 3)
	{
		printf("Socket Linsten falid\n");
		return 0;
	}
	return 0;
}

int SocketListen(int Port)
{
	struct sockaddr_in Addr;
	int SocketListenFd; //Listen fd
	int SocketAcceptFd; //Accept fd
	time_t TimeNow;
	struct tm* LocalTimeNow;

	SocketListenFd = socket(AF_INET,SOCK_STREAM,0); //Create Socket
 	if(SocketListenFd < 0) //Create Socket faild;
 	{
		return 1; //Create socket error
  	}
 	bzero(&Addr,sizeof(struct sockaddr_in)); //reset socket addr
 	Addr.sin_family = AF_INET;
 	Addr.sin_port = htons(Port);
 	Addr.sin_addr.s_addr = htonl(INADDR_ANY);
 	if(bind(SocketListenFd,(struct sockaddr *)&Addr,sizeof(Addr)) < 0)
 	{
		return 2; //Socket bind error
  	}
  	if(listen(SocketListenFd,3) == -1) //Listen falid
  	{
  		return 3; //Socket listen error
  	}
  	time(&TimeNow);//get now time
  	LocalTimeNow = localtime(&TimeNow); // convert now time to local time
  	printf("[%d-%02d-%02d %02d:%02d:%02d]\n",LocalTimeNow->tm_year+1900,LocalTimeNow->tm_mon+1,LocalTimeNow->tm_mday,LocalTimeNow->tm_hour,LocalTimeNow->tm_min,LocalTimeNow->tm_sec);
  	printf("Monitor service start\nPort: %d\n\n",Port);
  	while(1)
  	{
  		SocketAcceptFd = accept(SocketListenFd,NULL,NULL); //Accept Socket connect
   		if((SocketAcceptFd < 0) && (errno == EINTR))
		{
      		continue;
		}
   		else if(SocketAcceptFd < 0)
		{
			printf("accept error");
			continue;
		}
   		pthread_t RecvThread;
   		memset(&RecvThread, 0, sizeof(RecvThread));
   		int* NewSocketAcceptFd = (int*)malloc(sizeof(int));
   		*NewSocketAcceptFd = SocketAcceptFd;
   		if(pthread_create(&RecvThread, NULL, (void*)MsgReceive, NewSocketAcceptFd) != 0)
   		{
   			printf("Create message receive thread faild");
   		}
	}
  	close(SocketListenFd);
  	return 0;
}

void MsgReceive(int* SocketAcceptFd) //Msg receive thread function
{
	time_t TimeNow;
	struct tm* LocalTimeNow;
	struct sockaddr_in ClientAddr;
	int ClientAddrLen;
	int ReadSize;
	char ClientIP[INET_ADDRSTRLEN];//Client's IP
	char MsgRecvBuff[MSG_REC_BUFFER_SIZE]; //Msg received buffer

	time(&TimeNow);
	LocalTimeNow = localtime(&TimeNow);
	ClientAddrLen = sizeof(ClientAddr);
	getpeername(*SocketAcceptFd, (struct sockaddr *)&ClientAddr, (socklen_t *)&ClientAddrLen);//Get client's address
	inet_ntop(AF_INET, &ClientAddr.sin_addr, ClientIP, sizeof(ClientIP));
	printf("[%d-%02d-%02d %02d:%02d:%02d]\n",LocalTimeNow->tm_year+1900,LocalTimeNow->tm_mon+1,LocalTimeNow->tm_mday,LocalTimeNow->tm_hour,LocalTimeNow->tm_min,LocalTimeNow->tm_sec);
	printf("Msg: Client %s connected\n\n",ClientIP);
	while(1)
	{
		ReadSize = recv(*SocketAcceptFd,MsgRecvBuff,MSG_REC_BUFFER_SIZE, 0);
		time(&TimeNow); //get now time
		LocalTimeNow = localtime(&TimeNow); //convert now time to local time
		printf("[%04d-%02d-%02d %02d:%02d:%02d]\n",LocalTimeNow->tm_year+1900,LocalTimeNow->tm_mon+1,LocalTimeNow->tm_mday,LocalTimeNow->tm_hour,LocalTimeNow->tm_min,LocalTimeNow->tm_sec);
		if(ReadSize == -1) //Recived faild
		{
			printf("Msg: Client %s recived falid\n\n",ClientIP);
			continue;
		}
		if(ReadSize == 0) //Client socket close
		{
			printf("Msg: Client %s disconnected\n\n",ClientIP);
			break;
		}
		char ParsedMsg[1024] = "";
		ParseRecvMsg(MsgRecvBuff,ClientIP,ParsedMsg);
		printf("%s\n",ParsedMsg);
	}
	close(*SocketAcceptFd);
	return;
}

void ParseRecvMsg(char* RecvMsg,char* ClientIP,char* ParsedMsg) //Parse the received message
{
	int TimeStampInt; //Client system timestamp int
	time_t ClientSysTime; //Client system timestamp time_t
	struct tm* ClientSysTimeLocal; //Client system local time
	char HostName[_UTSNAME_SYSNAME_LENGTH]; // Host name
	char SystemVersion[_UTSNAME_SYSNAME_LENGTH]; //System Version
	char ClientSysTimeStr[20]; //System time Str
	char Runtime[20]; //System run time
	char CpuLoadOneMin[20]; //CPU load(1 minute)
	char CpuLoadFiveMin[20]; //CPU load(5 minutes)
	char CpuLoadFifteenMin[20]; //CPU load(15 minutes)
	char TotalRam[20]; //Total Ram
	char FreeRam[20]; //Free Ram
	char ProcessNum[8]; //Processes number

	strcat(ParsedMsg,"Client IP: ");
	strcat(ParsedMsg,ClientIP);
	strcat(ParsedMsg,"\n");
	if(strcmp(RecvMsg,"1") == 0) //Get client's uname faild
	{
		strcat(ParsedMsg,"Get client's uname faild\n");
		return;
	}
	if(strcmp(RecvMsg,"2") == 0) //Get client's sysinfo faild
	{
		strcat(ParsedMsg,"Get client's system info faild\n");
		return;
	}
	sprintf(HostName,"%s",strtok(RecvMsg,"|"));
	sprintf(SystemVersion,"%s",strtok(NULL,"|"));
	TimeStampInt = atoi(strtok(NULL,"|"));
	ClientSysTime = (time_t)TimeStampInt; //Timestamp to time_t
	ClientSysTimeLocal = localtime(&ClientSysTime); // time_t to struct tm(Local time)
	sprintf(ClientSysTimeStr,"%04d-%02d-%02d %02d:%02d:%02d",ClientSysTimeLocal->tm_year+1900,ClientSysTimeLocal->tm_mon+1,ClientSysTimeLocal->tm_mday,ClientSysTimeLocal->tm_hour,ClientSysTimeLocal->tm_min,ClientSysTimeLocal->tm_sec); //struct tm to str
	sprintf(Runtime,"%s",strtok(NULL,"|"));
	sprintf(CpuLoadOneMin,"%s",strtok(NULL,"|"));
	sprintf(CpuLoadFiveMin,"%s",strtok(NULL,"|"));
	sprintf(CpuLoadFifteenMin,"%s",strtok(NULL,"|"));
	sprintf(TotalRam,"%s",strtok(NULL,"|"));
	sprintf(FreeRam,"%s",strtok(NULL,"|"));
	sprintf(ProcessNum,"%s",strtok(NULL,"|"));
	sprintf(ParsedMsg,"%sHost Name: %s\n",ParsedMsg,HostName);
	sprintf(ParsedMsg,"%sSystem Version: %s\n",ParsedMsg,SystemVersion);
	sprintf(ParsedMsg,"%sSystem Time: %s\n",ParsedMsg,ClientSysTimeStr);
	sprintf(ParsedMsg,"%sSystem Run Time: %s seconds\n",ParsedMsg,Runtime);
	sprintf(ParsedMsg,"%sCPU load average 1-min 5-mins 15-mins: %s %s %s\n",ParsedMsg,CpuLoadOneMin,CpuLoadFiveMin,CpuLoadFifteenMin);
	sprintf(ParsedMsg,"%sRAM of total: %s KB\n",ParsedMsg,TotalRam);
	sprintf(ParsedMsg,"%sRAM of free: %s KB\n",ParsedMsg,FreeRam);
	float TotalRamInt;
	float FreeRamInt;
	int RamUtilizationRate;
	TotalRamInt = atoi(TotalRam);
	FreeRamInt = atoi(FreeRam);
	RamUtilizationRate = ((TotalRamInt-FreeRamInt)/TotalRamInt)*100;
	sprintf(ParsedMsg,"%sRAM utilization rate: %d%%\n",ParsedMsg,RamUtilizationRate);
	sprintf(ParsedMsg,"%sTask count: %s\n",ParsedMsg,ProcessNum);
}
