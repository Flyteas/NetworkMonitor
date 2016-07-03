/*
 * Client.h
 *
 *  Created on: Jul 1, 2016
 *      Author: flyshit
 *        Mail: flyshit@cqu.edu.cn | flyteas@gmail.com
 */

#ifndef CLIENT_H_
#define CLIENT_H_

int SocketConnect(char* IP,int Port); //Socket Connect
int SendSystemInfo(int fd); //Send SystemInfo

#endif /* CLIENT_H_ */
