#include <iostream> 
#include "../../include/util/utsname.h"

#if defined(_WIN32) || defined(_WIN64)

int uname(struct utsname* name)
	{

		OSVERSIONINFO osvi;

		ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

		GetVersionEx(&osvi);

		strcpy(name->sysname, "Windows");
		itoa(osvi.dwBuildNumber, name->release, 10);
		sprintf(name->version, "%i.%i", osvi.dwMajorVersion, osvi.dwMinorVersion);
		_Out_writes_bytes_(namelen) char* name22 = new char[100];

		char buffer[MAX_COMPUTERNAME_LENGTH + 1];
		DWORD size = sizeof(buffer);
		GetComputerNameA(buffer, &size);
		strcpy(name->nodename, buffer);


		SYSTEM_INFO sysInfo;
		ZeroMemory(&sysInfo, sizeof(SYSTEM_INFO));
		GetSystemInfo(&sysInfo);
		switch (sysInfo.wProcessorArchitecture) {
		case PROCESSOR_ARCHITECTURE_AMD64:
			strcpy(name->machine, "x86_64");
			break;
		case PROCESSOR_ARCHITECTURE_IA64:
			strcpy(name->machine, "ia64");
			break;
		case PROCESSOR_ARCHITECTURE_INTEL:
			strcpy(name->machine, "x86");
			break;
		case PROCESSOR_ARCHITECTURE_UNKNOWN:
		default:
			strcpy(name->machine, "unknown");
		}

		return 0;
	}
#elif defined __linux__
#include <sys/utsname.h>
#else
#error Not implemented!
#endif

