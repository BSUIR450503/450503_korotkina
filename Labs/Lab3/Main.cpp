#include "Header.h"

int main(int argc, char *argv[])
{
#ifdef _WIN32
	DWORD numRead;
	DWORD numToWrite;

	if (argc != 2)
	{
		HANDLE namedPipe = CreateNamedPipe(TEXT("\\\\.\\pipe\\Pipe"),
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			1024, 1024,
			NMPWAIT_USE_DEFAULT_WAIT,
			NULL);

		if (namedPipe == INVALID_HANDLE_VALUE)
		{
			cout << "Can't create pipe. Error. Press any key to exit " << GetLastError();
			_getch();
			return 0;
		}

		HANDLE hSemaphore = CreateSemaphore(NULL, 0, 1, "SEMAPHORE_SERVER");
		HANDLE hSemaphore_client = CreateSemaphore(NULL, 0, 1, "SEMAPHORE_CLIENT");

		MyProcess *clientProcess = new MyProcess(argv[0]);

		ConnectNamedPipe(namedPipe, NULL);

		WaitForSingleObject(hSemaphore, INFINITE);

		char *buffer = NULL;
		buffer = (char *)malloc(sizeof(char)* 1024);

		printf_s("Server ...\n");

		if (!WriteFile(namedPipe, "Ready", 1024, &numToWrite, NULL))
			return 0;

		while (1)
		{
			ReleaseSemaphore(hSemaphore_client, 1, NULL);
			WaitForSingleObject(hSemaphore, INFINITE);

			if (ReadFile(namedPipe, buffer, 1024, &numRead, NULL))
				printf_s("Client: %s", buffer);

			if (!strcmp(buffer, "exit"))
			{
				CloseHandle(namedPipe);
				CloseHandle(hSemaphore);
				free(buffer);
				return 0;
			}

			printf_s("\nServer: ");
			fflush(stdin);
			gets_s(buffer, 1024);

			if (!WriteFile(namedPipe, buffer, 1024, &numToWrite, NULL))
				break;

			ReleaseSemaphore(hSemaphore_client, 1, NULL);

			if (!strcmp(buffer, "exit"))
			{
				CloseHandle(namedPipe);
				CloseHandle(hSemaphore);

				free(buffer);
				return 0;
			}
		}
		return 0;
	}
	else
	{
		HANDLE hSemaphore = OpenSemaphore(EVENT_ALL_ACCESS, FALSE, "SEMAPHORE_SERVER");
		HANDLE hSemaphore_client = OpenSemaphore(EVENT_ALL_ACCESS, FALSE, "SEMAPHORE_CLIENT");

		HANDLE namedPipe = CreateFile(TEXT("\\\\.\\pipe\\Pipe"),
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);

		if (namedPipe == INVALID_HANDLE_VALUE)
		{
			printf_s("Can't create pipe. Error. Press any key to exit", GetLastError());
			_getch();
			return 0;
		}

		ReleaseSemaphore(hSemaphore, 1, NULL);

		char *buffer = NULL;
		buffer = (char *)malloc(sizeof(char)* 1024);

		printf_s("Client...\n");

		while (1)
		{
			WaitForSingleObject(hSemaphore_client, INFINITE);

			if (ReadFile(namedPipe, buffer, 1024, &numRead, NULL))
			{
				if (!strcmp(buffer, "exit"))
				{
					CloseHandle(hSemaphore_client);
					free(buffer);
					return 0;
				}

				printf_s("Server: %s", buffer);

				char input[1024] = { '\0' };
				cout << "\nClient: ";
				fflush(stdin);
				gets_s(input, 1024);

				if (!WriteFile(namedPipe, input, 1024, &numToWrite, NULL))
					break;

				if (!strcmp(input, "exit"))
				{
					ReleaseSemaphore(hSemaphore, 1, NULL);
					CloseHandle(hSemaphore_client);
					free(buffer);
					return 0;
				}
			}
			ReleaseSemaphore(hSemaphore, 1, NULL);
		}
		return 0;
	}

#elif __linux__
	pid_t pid;
	key_t key = ftok("/home/", 0);

	struct sembuf hSemaphore;
	struct sembuf hSemaphore_client;
	int semaphoreId;

	int sharedMemoryId;
	char *segmentPtr;

	if (argc != 2)
	{
		switch (pid = fork())
		{
		case -1:
			printf("Can't fork process %d\n", pid);
			break;
		case 0:
			execlp("gnome-terminal", "gnome-terminal", "-x", argv[0], "1", NULL);
		default:
			hSemaphore.sem_num = 0;
			hSemaphore.sem_op = 0;
			hSemaphore.sem_flg = 0;

			if ((sharedMemoryId = shmget(key, 1024, IPC_CREAT | IPC_EXCL | 0660)) == -1)
			{
				if ((sharedMemoryId = shmget(key, 1024, 0)) == -1)
				{
					printf("error\n");
					exit(1);
				}
			}

			if ((segmentPtr = (char*)shmat(sharedMemoryId, NULL, 0)) == (char*)(-1))
			{
				printf("Can't attach shared memory\n");
				exit(1);
			}

			semaphoreId = semget(key, 1, 0666 | IPC_CREAT);

			if (semaphoreId < 0)
			{
				printf("Can't get semaphore\n");
				exit(EXIT_FAILURE);
			}

			if (semctl(semaphoreId, 0, SETVAL, (int)0) < 0)
			{
				printf("Can't initialize semaphore\n");
				exit(EXIT_FAILURE);
			}

			semop(semaphoreId, &hSemaphore, 1);

			printf("Server process: ");

			while (1) {
				char *message = NULL;
				message = (char*)malloc(1024 * sizeof(char));

				if (semop(semaphoreId, &hSemaphore, 1) < 0)
					printf("Can't initialize server semaphore\n");

				printf("\nInput message to client: ");
				fflush(stdin);
				cin >> message;

				strcpy(segmentPtr, message);

				hSemaphore.sem_op = 3;
				semop(semaphoreId, &hSemaphore, 1);

				hSemaphore.sem_op = 0;
				semop(semaphoreId, &hSemaphore, 1);

				if (!strcmp("exit", message))
					return 0;

				strcpy(message, segmentPtr);
				printf("Client message: %s\n", message);

				hSemaphore.sem_op = 3;
				semop(semaphoreId, &hSemaphore, 1);

				strcpy(segmentPtr, message);

				hSemaphore.sem_op = 0;
			}
			return 0;
		}
	}
	else
	{
		if ((sharedMemoryId = shmget(key, 1024, IPC_CREAT | IPC_EXCL | 0660)) == -1)
		{
			if ((sharedMemoryId = shmget(key, 1024, 0)) == -1)
			{
				printf("error\n");
				exit(1);
			}
		}

		if ((segmentPtr = (char*)shmat(sharedMemoryId, NULL, 0)) == (char*)(-1))
		{
			printf("Can't attach shared memory\n");
			exit(1);
		}


		semaphoreId = semget(key, 1, 0666 | IPC_CREAT);

		if (semaphoreId < 0)
		{
			perror("");
			exit(EXIT_FAILURE);
		}

		if (semctl(semaphoreId, 0, SETVAL, (int)0) < 0)
		{
			perror("");
			exit(EXIT_FAILURE);
		}

		hSemaphore_client.sem_num = 0;
		hSemaphore_client.sem_op = 1;
		hSemaphore_client.sem_flg = 0;

		semop(semaphoreId, &hSemaphore_client, 1);

		hSemaphore_client.sem_op = -1;
		semop(semaphoreId, &hSemaphore_client, 1);
		hSemaphore_client.sem_op = -2;

		if ((sharedMemoryId = shmget(key, 1024, 0)) == -1)
		{
			perror("");
			exit(1);
		}


		while (1) {
			char *message = NULL;
			message = (char*)malloc(1024 * sizeof(char));

			semop(semaphoreId, &hSemaphore_client, 1);

			strcpy(message, segmentPtr);

			if (!strcmp("exit", message))
			{
				hSemaphore_client.sem_op = -1;
				semop(semaphoreId, &hSemaphore_client, 1);
				return 0;
			}

			printf("\nServer message: %s", message);

			printf("\nInput message to server: ");
			fflush(stdin);
			cin >> message;

			strcpy(segmentPtr, message);

			hSemaphore_client.sem_op = -1;
			semop(semaphoreId, &hSemaphore_client, 1);

			hSemaphore_client.sem_op = -2;
			semop(semaphoreId, &hSemaphore_client, 1);

			strcpy(message, segmentPtr);
			printf("\nServer message: %s", message);

			hSemaphore_client.sem_op = -1;
			semop(semaphoreId, &hSemaphore_client, 1);

			hSemaphore_client.sem_op = -2;
		}
		return 0;
	}
#endif
}
