#include "Header.h"
#ifdef _WIN32
#define LENGTH 10
#define NUMBER_OF_FILES 4
char name[NUMBER_OF_FILES];
char *buffer;

DWORD WINAPI readFile(LPVOID param)
{
	HANDLE file = CreateFile(name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (file == INVALID_HANDLE_VALUE)
	{
		perror("CreateFileA");
		exit(EXIT_FAILURE);
	}
	OVERLAPPED ov = { 0 };
	DWORD numRead;
	char *read_data = (char*)malloc(10);
	memset(read_data, 0, 10);
	BOOL result = ReadFile(file, read_data, LENGTH, NULL, &ov);
	int file_length = strlen(read_data);
	buffer = (char*)malloc(file_length);
	memset(buffer, 0, file_length);
	strcpy(buffer, read_data);
	while (HasOverlappedIoCompleted(&ov) == FALSE)
	{
		Sleep(10);
	}
	result = GetOverlappedResult(file, &ov, &numRead, TRUE);
	CloseHandle(file);
}

DWORD WINAPI writeToFinalFile(LPVOID param)
{
	char namefinal[] = "final.txt";
	HANDLE file = CreateFile(namefinal, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (file == INVALID_HANDLE_VALUE)
	{
		perror("CreateFileA");
		exit(EXIT_FAILURE);
	}
	DWORD numToWrite;
	DWORD file_pointer = SetFilePointer(file, 0, NULL, FILE_END);
	OVERLAPPED ov = { 0 };
	ov.Offset = file_pointer;
		
	int file_length = strlen(buffer);
	char *write_data = (char*)malloc(file_length);
	memset(write_data, 0, file_length);
	strcpy(write_data, buffer);
	BOOL result = WriteFile(file, write_data, strlen(write_data), NULL, &ov);
	while (FALSE == HasOverlappedIoCompleted(&ov))
	{
		printf("Working");
	}
	result = GetOverlappedResult(file, &ov, &numToWrite, TRUE);
	CloseHandle(file);
}

void main()
{
	LPVOID param;
	for (int i = 1; i < NUMBER_OF_FILES+1; i++)
	{
		_itoa(i, name, 10);
		int msg = 0;
		HANDLE reader = CreateThread(NULL, 0, &readFile, 0, 0, NULL);
		WaitForSingleObject(reader, INFINITE);
		HANDLE writer = CreateThread(NULL, 0, &writeToFinalFile, 0, 0, NULL);
		WaitForSingleObject(writer, INFINITE);
	}
	return;
}
#endif