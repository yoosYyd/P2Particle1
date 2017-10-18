#pragma warning(disable : 4996)
#pragma pack(push,1)
struct STUN_REQ//stun request
{
	UINT16 type;
	UINT16 length;
	UINT32 magic;
	UINT8 tsx_id[12];
};

struct STUN_RESP//response from stun server
{
	UINT16 type;
	UINT16 length;
	UINT32 magic;
	UINT8 tsx_id[12];

	unsigned char unknown[5];//it useless for me

	unsigned char  family;//0x01:IPv4 0x02:IPv6
	unsigned short port;
	unsigned int   addr;
};
#pragma pack(pop,1)

/* STUN message types*/
#define STUN_BINDREQ    0x0001
#define STUN_BINDRESP   0x0101
#define STUN_BINDERR    0x0111
#define STUN_SECREQ     0x0002
#define STUN_SECRESP    0x0102
#define STUN_SECERR     0x0112
/////////////////////////////////
#define STUN_MAGIC 0x2112A442
////////////////////////////////
class STUN
{
private:
	SOCKET s;
	WSADATA wsd;
	struct sockaddr_in stun_serv;
	void Request(char *ip, char *port)
	{
		STUN_REQ *req = new STUN_REQ;//preparing request
		req->type = htons(STUN_BINDREQ);
		req->length = 0;
		req->magic = STUN_MAGIC;
		srand((unsigned)time(NULL));
		for (int i = 0; i < 12; i++)
		{
			req->tsx_id[i] = rand() % 255;
		}

		if (sendto(s, (char*)req, sizeof(STUN_REQ), 0, (struct sockaddr *) &this->stun_serv, sizeof(this->stun_serv)) == SOCKET_ERROR)
		{
			printf("sendto error %d\n", WSAGetLastError());
		}
		int slen = sizeof(struct sockaddr_in);

		STUN_RESP *resp = new STUN_RESP;//getting server response
		if (recvfrom(s, (char*)resp, sizeof(STUN_RESP), 0, (struct sockaddr *) &this->stun_serv, &slen) == SOCKET_ERROR)
		{
			printf("recvfrom error %d\n", WSAGetLastError());
		}

		struct sockaddr_in sa = { 0 };//parsing  response
		sa.sin_addr.s_addr = resp->addr;
		wsprintfA(ip, "%s", inet_ntoa(sa.sin_addr));
		wsprintfA(port, "%d", ntohs(resp->port));

		delete resp;
		delete req;
	}
public:
	STUN(char *stun_name)
	{
		WSAStartup(MAKEWORD(2, 2), &this->wsd);
		struct hostent *remoteSTUN;
		remoteSTUN = gethostbyname(stun_name);
		int i = 0;
		while (remoteSTUN->h_addr_list[i] != 0)
		{
			this->stun_serv.sin_addr.s_addr = *(u_long *)remoteSTUN->h_addr_list[i++];
		}
		this->stun_serv.sin_family = AF_INET;
		this->stun_serv.sin_port = htons(19302);
		this->s = socket(AF_INET, SOCK_DGRAM, 0);
	}
	~STUN()
	{

	}
	SOCKET GetSocket()
	{
		return s;
	}
	void GetIpPort(char *ip, char *port)
	{
		this->Request(ip, port);
	}
};

