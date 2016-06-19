#pragma once
#ifdef _WIN32
#include <windows.h>
#include <process.h>
#include <stdio.h>
#include <iostream>

using namespace std;

struct OperationInfo
{
	HANDLE hFile;
	DWORD bytesCount;
	CHAR  buffer[100];
	DWORD  pos_out_file;
	OVERLAPPED overlapped;
} operationInfo;

HINSTANCE Lib;
HANDLE events[3];

DWORD WINAPI readerThread(PVOID path);
DWORD WINAPI writerThread(PVOID path);
#endif

#ifdef __linux__
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <dirent.h>
#include <aio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <errno.h> 

struct OperationInfo
{
	char readFileName[500];
	char writeFileName[500];
	char buffer[300];
	struct aiocb readCb;
	struct aiocb writeCb;
	int bytesRead;
	int bytesWritten;
} operationInfo;

char fileNames[20][100];
int numberOfFiles = 0;
pthread_t readerThread;
pthread_t writerThread;
pthread_mutex_t mutex;

void(*writeFunction)(struct OperationInfo *operationInfo);
void(*readFunction)(struct OperationInfo *operationInfo);
#endif

#define READ_FINISH 0
#define EXIT 1
#define WRITE_FINISH 2