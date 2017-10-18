class puncher
{
private:
	static DWORD WINAPI NetThr(LPVOID lParam)//receiving thread 
	{
		WSADATA wsd = { 0 };
		WSAStartup(MAKEWORD(2, 2), &wsd);
		puncher *punch = (puncher *)lParam;
		struct sockaddr_in remote;
		int slen = sizeof(struct sockaddr_in);
		punch->CopyRemoteAddr(&remote);
		SOCKET s = punch->GetSocket();
		DWORD magic = 0xDEADBEEF/*data marker*/, buf = 0;
		for (;;)
		{
			if (recvfrom(s, (char*)&buf, 4, 0, (struct sockaddr *) &remote, &slen) == SOCKET_ERROR)//wait data from NAT
			{
				printf("!recvfrom err: %d\n", WSAGetLastError());
			}
			else
			{
				if (buf == magic)
				{
					return 0;
				}
			}
		}
		return 0;
	}
	const DWORD magic = 0xDEADBEEF;/*data marker*/
	SOCKET s;
	struct sockaddr_in remote;
public:
	puncher(SOCKET localSocket, char *destIp, char *destPort)
	{
		this->s = localSocket;
		this->remote.sin_family = AF_INET;
		this->remote.sin_port = htons(atoi(destPort));
		InetPtonA(AF_INET, destIp, &(this->remote.sin_addr));
	}
	~puncher()
	{

	}
	SOCKET GetSocket()
	{
		return this->s;
	}
	void CopyRemoteAddr(struct sockaddr_in *remote)
	{
		memcpy(remote, &this->remote, sizeof(struct sockaddr_in));
	}
	bool Start()
	{
		HANDLE thr = CreateThread(0, 0, this->NetThr, this, 0, 0);
		int stop = 0;
		for (;;)
		{
			/*this sendto said to our NAT, what we wait data from host which described in "this->remote" class member*/
			if (sendto(s, (char*)&this->magic, 4, 0, (struct sockaddr *) &this->remote, sizeof(this->remote)) == SOCKET_ERROR)
			{
				printf("sendto error: %d\n", WSAGetLastError());
			}
			else
			{
				if (WaitForSingleObject(thr, 1000) == WAIT_OBJECT_0)//receiving thread is run?
				{
					for (int i = 0; i < 15; i++)//if NO,that means success 
					{/*then send data to partner NAT*/
						if (sendto(s, (char*)&this->magic, 4, 0, (struct sockaddr *) &this->remote, sizeof(this->remote)) == SOCKET_ERROR)
						{
							printf("sendto error: %d\n", WSAGetLastError());
						}
					}
					return true;
				}
				else
				{
					printf("timeout\n");
					stop++;
				}
			}
			if (stop == 10) { return false; }
		}
		return false;
	}
};
