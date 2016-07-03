/*
 * Server.h
 *
 *  Created on: Jul 2, 2016
 *      Author: flyshit
 *        Mail: flyshit@cqu.edu.cn | flyteas@gmail.com
 */

#ifndef SERVER_H_
#define SERVER_H_

int SocketListen(int Port); //Open a Socket Listen
void MsgReceive(int* SocketAcceptFd); //Msg receive thread function
void ParseRecvMsg(char* RecvMsg,char* ClientIP,char* ParsedMsg); //Parse the received message

#endif /* SERVER_H_ */
