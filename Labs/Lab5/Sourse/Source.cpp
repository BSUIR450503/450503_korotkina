#include "Header.h"

#ifdef _WIN32
#define FILE_PATH "D:\\Linux"

int main(int argc, char *argv[])
{
	HANDLE hEvent;
	HANDLE threads[2];
	char filePath[MAX_PATH] = FILE_PATH;
	Lib = LoadLibrary("Lib.dll");

	hEvent = CreateEvent(NULL, FALSE, TRUE, TEXT("event"));
	events[WRITE_FINISH] = CreateEvent(NULL, FALSE, TRUE, NULL);
	events[READ_FINISH] = CreateEvent(NULL, FALSE, FALSE, NULL);
	events[EXIT] = CreateEvent(NULL, TRUE, FALSE, NULL);

	operationInfo.bytesCount = sizeof(operationInfo.buffer);
	operationInfo.pos_out_file = 0;
	operationInfo.overlapped.Offset = 0;
	operationInfo.overlapped.OffsetHigh = 0;
	operationInfo.overlapped.hEvent = hEvent;

	threads[0] = CreateThread(NULL, 0, readerThread, (LPVOID)filePath, 0, NULL);
	threads[1] = CreateThread(NULL, 0, writerThread, (LPVOID)filePath, 0, NULL);
	WaitForMultipleObjects(2, threads, TRUE, INFINITE);

	CloseHandle(threads[0]);
	CloseHandle(threads[1]);
	CloseHandle(events[WRITE_FINISH]);
	CloseHandle(events[READ_FINISH]);
	CloseHandle(events[EXIT]);
	CloseHandle(hEvent);

	FreeLibrary(Lib);
	printf("All data were successfully written.\n");
	system("Pause");
	return 0;
}

DWORD WINAPI readerThread(PVOID path)
{
	string folderName((const char*)path);
	folderName.append("\\");
	string fileName = folderName + "?.txt";
	char currentFileName[MAX_PATH];
	BOOL(*readFunction)(OperationInfo*) = (BOOL(*)(OperationInfo*))GetProcAddress(Lib, "readFromFile");
	WIN32_FIND_DATA findData;
	HANDLE file, fileToRead;

	file = FindFirstFile(fileName.c_str(), &findData);
	if (file == INVALID_HANDLE_VALUE)
	{
		printf("Path error");
		return 0;
	}

	while (true)
	{
		WaitForSingleObject(events[WRITE_FINISH], INFINITE);
		strcpy(currentFileName, folderName.c_str());
		strcat(currentFileName, findData.cFileName);
		fileToRead = CreateFile(currentFileName, GENERIC_READ, 0, NULL,
			OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
		operationInfo.hFile = fileToRead;
		(readFunction)(&operationInfo);
		SetEvent(events[READ_FINISH]);

		if (FindNextFile(file, &findData))
		{
			CloseHandle(fileToRead);
			continue;
		}
		else break;
	}

	FindClose(file);
	CloseHandle(fileToRead);
	SetEvent(events[EXIT]);
	return 0;
}

DWORD WINAPI writerThread(PVOID path)
{
	string fileName((const char*)path);
	fileName.append("\\final.txt");
	HANDLE hFile = CreateFile(fileName.c_str(), GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
	HANDLE eventsForWait[2] = { events[READ_FINISH], events[EXIT] };
	BOOL(*writeFunction)(OperationInfo*) = (BOOL(*)(OperationInfo*))GetProcAddress(Lib, "writeToFile");

	while (true)
	{
		int event = WaitForMultipleObjects(2, eventsForWait, FALSE, INFINITE);
		if (event == EXIT) break;
		operationInfo.hFile = hFile;
		(writeFunction)(&operationInfo);
		SetEvent(events[WRITE_FINISH]);
	}

	CloseHandle(hFile);
	return 0;
}

#elif __linux__

int main()
{
	if (pthread_mutex_init(&mutex, NULL))
	{
		printf("Can't create mutex");
		return 0;
	}

	remove("./final.txt");
	void *library;
	library = dlopen("./Lib.so", RTLD_NOW);
	readFunction = (void(*)(struct OperationInfo *operationInfo))dlsym(library, "readFromFile");
	writeFunction = (void(*)(struct OperationInfo *operationInfo))dlsym(library, "writeToFile");

	struct dirent *dp;
	DIR *dir;

	dir = opendir("./");
	puts("Files in directory: ");
	while ((dp = readdir(dir)) != NULL)
	{
		if (strstr(dp->d_name, ".txt") != NULL)
		{
			strcpy(fileNames[numberOfFiles], dp->d_name);
			numberOfFiles++;
			puts(dp->d_name);
		}
	}

	pthread_create(&readerThread, NULL, reader, NULL);
	pthread_create(&writerThread, NULL, writer, NULL);

	pthread_join(readerThread, NULL);
	pthread_join(writerThread, NULL);

	return 0;
}
void *reader(void *arg)
{
	for (int i = 0; i < numberOfFiles; i++)
	{
		pthread_mutex_lock(&mutex);

		strcpy(operationInfo.readFileName, fileNames[i]);
		(*readFunction)(&operationInfo);

		pthread_mutex_unlock(&mutex);
		usleep(100000);
	}
	return NULL;
}

void *writer(void *arg)
{
	strcpy(operationInfo.writeFileName, "./final.txt");

	for (int i = 0; i < numberOfFiles; i++)
	{
		usleep(10000);
		pthread_mutex_lock(&mutex);
		(*writeFunction)(&operationInfo);
		pthread_mutex_unlock(&mutex);
		usleep(100000);
	}
	return NULL;
}

#endif