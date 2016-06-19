#ifdef _WIN32
#include <windows.h>
#include <process.h>
#include <stdio.h>
#include <iostream>
#endif

#ifdef __linux__
#include <stdio.h>
#include <stdlib.h>
#include <aio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#endif

#ifdef _WIN32
using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

	struct OperationInfo
	{
		HANDLE hFile;
		DWORD bytesCount;
		CHAR  buffer[100];
		DWORD  fileOffset;
		OVERLAPPED overlapped;
	} operationInfo;

	__declspec(dllexport) BOOL readFromFile(OperationInfo *operationInfo)
	{
		BOOL readResult;
		DWORD bytesTransferred;
		operationInfo->overlapped.Offset = 0;

		ReadFile(operationInfo->hFile, operationInfo->buffer,
			operationInfo->bytesCount, NULL, &operationInfo->overlapped);

		WaitForSingleObject(operationInfo->overlapped.hEvent, 1000);

		readResult = GetOverlappedResult(operationInfo->hFile,
			&operationInfo->overlapped, &bytesTransferred, FALSE);

		return readResult;
	}

	__declspec(dllexport) BOOL writeToFile(OperationInfo *operationInfo) {
		BOOL writeResult;
		DWORD bytesTransferred;
		operationInfo->overlapped.Offset = operationInfo->fileOffset;

		WriteFile(operationInfo->hFile, operationInfo->buffer,
			operationInfo->overlapped.InternalHigh, NULL, &operationInfo->overlapped);

		WaitForSingleObject(operationInfo->overlapped.hEvent, 1000);

		writeResult = GetOverlappedResult(operationInfo->hFile,
			&operationInfo->overlapped, &bytesTransferred, FALSE);

		if (writeResult)
		{
			operationInfo->fileOffset = operationInfo->fileOffset + bytesTransferred;
		}

		return writeResult;
	}

#ifdef __cplusplus
}
#endif

#endif

#ifdef __linux__

struct OperationInfo
{
	char readFileName[500];
	char writeFileName[500];
	char buffer[300];
	struct aiocb readCb;
	struct aiocb writeCb;
	int bytesRead;
	int bytesWritten;
};


void readFromFile(struct OperationInfo *operationInfo)
{
	printf("\nReading from %s\n", operationInfo->readFileName);

	int hFile = open(operationInfo->readFileName, O_RDONLY, 0);

	if (hFile == -1)
	{
		printf("Can't to open file for read: %s\n", operationInfo->readFileName);
		return;
	}

	operationInfo->readCb.aio_fildes = hFile;
	operationInfo->readCb.aio_buf = operationInfo->buffer;
	operationInfo->readCb.aio_nbytes = sizeof(operationInfo->buffer);
	operationInfo->readCb.aio_offset = 0;

	aio_read(&operationInfo->readCb);
	while (aio_error(&operationInfo->readCb) == EINPROGRESS);

	operationInfo->bytesRead = aio_return(&operationInfo->readCb);
	close(hFile);
}

void writeToFile(struct OperationInfo *operationInfo) {

	int hFile = open(operationInfo->writeFileName, O_CREAT | O_RDWR | O_APPEND, 0666);

	if (hFile == -1)
	{
		printf("Can't to open file for write: %s\n", operationInfo->writeFileName);
		return;
	}

	operationInfo->writeCb.aio_fildes = hFile;
	operationInfo->writeCb.aio_buf = operationInfo->buffer;
	operationInfo->writeCb.aio_nbytes = operationInfo->bytesRead;

	printf("Writing to %s\n", operationInfo->writeFileName);
	aio_write(&operationInfo->writeCb);
	while (aio_error(&operationInfo->writeCb) == EINPROGRESS);

	operationInfo->bytesWritten = aio_return(&operationInfo->writeCb);
	close(hFile);
}

#endif