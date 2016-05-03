#ifdef _WIN32
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>

#endif

#ifdef linux
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#endif

using namespace std;

#ifdef _WIN32
class MyProcess
{
public:
	MyProcess() {};
	MyProcess(char*);
};

MyProcess::MyProcess(char* name)
{
	STARTUPINFO startupInfo;
	PROCESS_INFORMATION processInfo;

	ZeroMemory(&startupInfo, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);
	ZeroMemory(&processInfo, sizeof(processInfo));

	char commandLine[16] = "-n 1";

	if (!CreateProcess(name,
		commandLine,
		NULL,
		NULL,
		FALSE,
		CREATE_NEW_CONSOLE,
		NULL,
		NULL,
		&startupInfo,
		&processInfo))
	{
		cout << "Can't create process. Error " << GetLastError();
	}
}
#endif

#ifdef __linux__
#endif

