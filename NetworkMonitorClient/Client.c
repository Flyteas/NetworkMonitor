/*
 * Client.c
 *
 *  Created on: Jul 1, 2016
 *      Author: flyshit
 *        Mail: flyshit@cqu.edu.cn | flyteas@gmail.com
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/utsname.h>
#include <sys/syscall.h>
#include <sys/sysinfo.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Client.h"
#define MSG_SEND_BUFFER_SIZE 1024
#define SEND_DELAY_SECONDS 5 //send delay(seconds)

int main(int argc,char* argv[])
{
	int SocketFd;
	char* IP;
	int Port;

	if(argc<3)
	{
		printf("Usage: ./ProgramName ServerIP ServerPort\n");
		printf("Example: ./Client 127.0.0.1 8888\n");
		return 0;
	}
	IP = argv[1];
	Port = atoi(argv[2]);
	SocketFd = SocketConnect(IP,Port);
	while(1)
	{
		int SendResult;
		sleep(SEND_DELAY_SECONDS);
		SendResult = SendSystemInfo(SocketFd);
		if(SendResult == 0)
		{
			printf("Send successful\n");
		}
		if(SendResult == 1)
		{
			printf("Get system name faild\n");
		}
		if(SendResult == 2)
		{
			printf("Get system info faild\n");
		}
		if(SendResult == 3)
		{
			printf("Connection lost\n");
			break;
		}
	}
	close(SocketFd);
	return 0;
}

int SocketConnect(char* IP,int Port) //Socket Connect
{
	struct sockaddr_in addr; //服务器端地址
	int fd; //文件描述符fd 用于唯一标识socket

 	if((fd=socket(AF_INET,SOCK_STREAM,0))<0) ////创建socket连接
	{
		printf("Create socket error\n");
		exit(1);
  	}
 	bzero(&addr,sizeof(struct sockaddr_in)); //重置
 	addr.sin_family=AF_INET; //ARPA因特网协议
 	addr.sin_port=htons(Port); //端口号
 	addr.sin_addr.s_addr=inet_addr(IP); //地址
  	if(connect(fd, (struct sockaddr *)&addr,sizeof(struct sockaddr)) == -1) //连接服务器
	{
        printf("Connect error\n");
        exit(0);
    }
  	return fd;
}

int SendSystemInfo(int fd) //Send system info
{
		time_t TimeLocal;
		char HostName[_UTSNAME_SYSNAME_LENGTH]; // Host name
		char SystemVersion[_UTSNAME_SYSNAME_LENGTH]; //System Version
		char TimeStamp[12]; //system time
		char Runtime[20]; //System run time
		char CpuLoadOneMin[20]; //CPU load(1 minute)
		char CpuLoadFiveMin[20]; //CPU load(5 minutes)
		char CpuLoadFifteenMin[20]; //CPU load(15 minutes)
		char TotalRam[20]; //Total Ram
		char FreeRam[20]; //Free Ram
		char ProcessNum[8]; //Processes number
		char MsgSend[MSG_SEND_BUFFER_SIZE]="";
		int GetUnameInfoResult;
		int GetInfoResult;
		struct utsname UnameInfo;
		struct sysinfo Info;

		GetUnameInfoResult = syscall(__NR_uname,&UnameInfo);
		GetInfoResult = syscall(__NR_sysinfo,&Info); //system call
		if(GetUnameInfoResult == 0)
		{
			sprintf(HostName,"%s|",UnameInfo.nodename);
			sprintf(SystemVersion,"%s|",UnameInfo.version);
		}
		else
		{
			strcat(MsgSend,"1"); //Get uname faild
			if(send(fd, MsgSend, strlen(MsgSend)+1, 0) != -1) //send successful
			{
				return 1;
			}
			else //send falid
			{
				return 3;
			}
		}
		if(GetInfoResult == 0) //get info successful
		{
			sprintf(TimeStamp,"%d|",(int)time(&TimeLocal));
			sprintf(Runtime,"%ld|",Info.uptime);
			sprintf(CpuLoadOneMin,"%ld|",Info.loads[0]);
			sprintf(CpuLoadFiveMin,"%ld|",Info.loads[1]);
			sprintf(CpuLoadFifteenMin,"%ld|",Info.loads[2]);
			sprintf(TotalRam,"%ld|",Info.totalram);
			sprintf(FreeRam,"%ld|",Info.freeram);
			sprintf(ProcessNum,"%hd|",Info.procs);
			strcat(MsgSend,HostName);
			strcat(MsgSend,SystemVersion);
			strcat(MsgSend,TimeStamp);
			strcat(MsgSend,Runtime);
			strcat(MsgSend,CpuLoadOneMin);
			strcat(MsgSend,CpuLoadFiveMin);
			strcat(MsgSend,CpuLoadFifteenMin);
			strcat(MsgSend,TotalRam);
			strcat(MsgSend,FreeRam);
			strcat(MsgSend,ProcessNum);
		}
		else //get info faild
		{
			strcat(MsgSend,"2"); //Get sysinfo faild;
			if(send(fd, MsgSend, strlen(MsgSend)+1, 0) != -1) //send successful
			{
				return 2;
			}
			else //send falid
			{
				return 3;
			}
		}
		if(send(fd, MsgSend, strlen(MsgSend)+1, 0) != -1) //send successful
		{
			return 0;
		}
		else
		{
			return 3;
		}
}
