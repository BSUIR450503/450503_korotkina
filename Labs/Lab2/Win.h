#include "Lib.h"
#ifdef _WIN32
class Process
{
public:
	Process();
	Process(int, char*);
	HANDLE printEvent;
	HANDLE closeEvent;
};

Process::Process()
{
	printEvent = NULL;
	closeEvent = NULL;
}

Process::Process(int number, char* name)
{
	STARTUPINFO startupInfo;
	PROCESS_INFORMATION processInfo;

	ZeroMemory(&startupInfo, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);
	ZeroMemory(&processInfo, sizeof(processInfo));

	printEvent = CreateEvent(NULL, TRUE, FALSE, printEvents[number - 1]);
	closeEvent = CreateEvent(NULL, FALSE, FALSE, closeEvents[number - 1]);

	char commandLine[10] = "-n ";
	strcat(commandLine, printEvents[number - 1]);
	strcat(commandLine, " ");
	strcat(commandLine, closeEvents[number - 1]);

	if (!CreateProcess(name, commandLine, NULL, NULL, FALSE, NULL, NULL,
		NULL, &startupInfo, &processInfo))
	{
		cout << "Can't create process. Error " << GetLastError();
	}
}

int getch_noblock()
{
	if (_kbhit())
		return _getch();
	else
		return -1;
}
#endif
