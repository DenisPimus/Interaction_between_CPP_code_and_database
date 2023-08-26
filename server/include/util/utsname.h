#include <iostream> 

#ifdef __linux__
	#include <sys/utsname.h> // Для uname()
#elif defined(_WIN32) || defined(_WIN64)
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma warning(disable : 4996)
#pragma comment(lib, "wsock32.lib")
#define UTSNAME_MAXLENGTH 256
	struct utsname {
		char sysname[UTSNAME_MAXLENGTH]; // name of this implementation of the operating system
		char nodename[UTSNAME_MAXLENGTH]; // name of this node within an implementation - dependent communications network
		char release[UTSNAME_MAXLENGTH]; //  current release level of this implementation
		char version[UTSNAME_MAXLENGTH]; //  current version level of this release
		char machine[UTSNAME_MAXLENGTH]; //  name of the hardware type on which the system is running
	};
	int uname(struct utsname* name);
#elif defined __linux__
#include <sys/utsname.h>
#else
#error Not implemented!
#endif
