#include "stdafx.h"
#pragma warning(disable : 4996)

SOCKET p2pSocket;
struct sockaddr_in padr;
HANDLE this_proc;

DWORD WINAPI ChatOutThr(LPVOID lParam)
{
	char *partner_id = (char*)lParam;
	char *buf = new char[65536];
	int slen = sizeof(struct sockaddr_in);
	for (;;)
	{
		if (recvfrom(p2pSocket, buf, 65536, 0, (struct sockaddr *) &padr, &slen) == SOCKET_ERROR)
		{
			printf("!recvfrom err: %d\n", WSAGetLastError());
		}
		else
		{
			printf("%s -> %s\n> ", partner_id,buf);
			if (strcmp(buf, "bye") == 0)
			{
				closesocket(p2pSocket);
				WSACleanup();
				TerminateProcess(this_proc, 0);
			}
			memset(buf, 0, 65536);
		}
	}
	return 0;
}
int main()
{
	this_proc = GetCurrentProcess();
	STUN *stun = new STUN("stun.l.google.com");//public stun server
	char ip[100] = "", port[10] = "";
	char partner[100] = "";
	char *pport;
	stun->GetIpPort(ip, port);
	p2pSocket = stun->GetSocket();
	stun->~STUN();
	printf("My internet address: %s:%s\n",ip,port);
	scanf("%s", partner);//put partner ip:port here
	pport = strstr(partner, ":");
	pport += 1;
	strtok(partner, ":");
	puncher *up = new puncher(p2pSocket, partner, pport);//"UDP punching" begin here
	if (up->Start() == true)//true if "UDP punching" is success
	{
		Sleep(400);//wait for all datagrams
		printf("UDP punching succes\n");
		up->CopyRemoteAddr(&padr);//get struct sockaddr_in with partner adress
		int slen = sizeof(struct sockaddr_in);
		char *buf = new char[65536];
		fd_set read_check;
		FD_ZERO(&read_check);
		FD_SET(p2pSocket,&read_check);
		timeval time_out;
		time_out.tv_sec = 0; 
		time_out.tv_usec = 100000;
		for (;;)//clear socket
		{
			if (select(0, &read_check, 0, 0, &time_out) == 0)
			{
				break;
			}
			if (recvfrom(p2pSocket, buf, 4, 0, (struct sockaddr *) &padr, &slen) == SOCKET_ERROR)
			{
				printf("recvfrom error: %d\n", WSAGetLastError());
			}
		}
		memset(buf, 0, 65536);
		/*simple demo chat*/
		HANDLE thr = CreateThread(0, 0, ChatOutThr, (void*)partner, 0, 0);//starting receiving thread
		for (;;)
		{
			printf("> ");
			gets_s(buf, 65536);
			if (sendto(p2pSocket, buf, strlen(buf), 0, (struct sockaddr *) &padr, sizeof(padr)) == SOCKET_ERROR)
			{
				printf("sendto error: %d\n", WSAGetLastError());
			}
			if (strcmp(buf, "bye") == 0)
			{
				TerminateThread(thr, 0);
				break;
			}
			memset(buf, 0, 65536);
		}
	}
	else
	{
		printf("UDP punching failed :(\n");
	}
	closesocket(p2pSocket);
	WSACleanup();
	_getch();
    return 0;
}

