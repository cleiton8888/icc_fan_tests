/*****************************************************************
*
* ============== PS4 Kernel Dumper ===============
*
*	Thanks to:
*	- CelesteBlue for the rewriting (USB mode + cross FW compatibility)
*	- WildCard for his functional code
*	- Shadow for the copyout trick
*	- xvortex for the updated PS4 payload SDK
*	- CTurt for his PS4 payload SDK
*	- Specter for his Code Execution method and implementation
*	- qwertyoruiopz for his webkit and kernel exploits
*
******************************************************************/

#include "kernel_utils.h"


//#define DEBUG_SOCKET // comment to use USB method
//#define KERNEL_CHUNK_SIZE	0x1000
//#define KERNEL_CHUNK_NUMBER	0x69B8

#define printf_notification(...) \
	do { \
		char message[256]; \
		snprintf(message, sizeof(message), ##__VA_ARGS__); \
		send_system_notification_with_text(message); \
	} while (0)

#ifdef DEBUG_SOCKET
#define printfsocket(format, ...)\
	do {\
		char buffer[512];\
		int size = sprintf(buffer, format, ##__VA_ARGS__);\
		sceNetSend(sock, buffer, size, 0);\
	} while(0)

#define IP(a, b, c, d) (((a) << 0) + ((b) << 8) + ((c) << 16) + ((d) << 24))
#endif


void send_system_notification_with_text(char *message) {
	char notification_buf[512];
	sprintf(notification_buf, "%s\n\n\n\n\n\n\n", message);
	sceSysUtilSendSystemNotificationWithText(0x81, notification_buf);
}


int _main(struct thread *td) {
	// Init and resolve libraries
	initKernel();
	initLibc();
	initNetwork();
	initPthread();
	
#ifdef DEBUG_SOCKET
	// Create our TCP server
	struct sockaddr_in server;
	server.sin_len = sizeof(server);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = IP(192, 168, 0, 19);
	server.sin_port = sceNetHtons(9023);
	memset(server.sin_zero, 0, sizeof(server.sin_zero));
	int sock = sceNetSocket("debug", AF_INET, SOCK_STREAM, 0);
	sceNetConnect(sock, (struct sockaddr *)&server, sizeof(server));
	int flag = 1;
	sceNetSetsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));
	printfsocket("connected\n");
#endif
	
	uint64_t fw_version = get_fw_version();
	
	jailbreak(fw_version); // Patch some things in the kernel (sandbox, prison) to give to userland more privileges
	
	initSysUtil(); // Need the browser to have been jailbreaked first

	int fd = open("/dev/icc_fan", O_RDONLY, 0);
    if (fd <= 0) {
        send_system_notification_with_text("ICC_FAN CAN'T OPEN.");
        return 0;
    }else{
		send_system_notification_with_text("ICC_FAN OPENED.");
	}

	//needs research. props to theorywrong

    char* data[10] = {0,0,0,0,0,0,0,0,0,0x80};
    int ret = ioctl(fd, 0xC01C8F07, data);

    close(fd);
	
	return ret;
	
	//return 0;
}