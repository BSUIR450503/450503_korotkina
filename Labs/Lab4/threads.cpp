#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#include <tchar.h>
#include <stdio.h>
#include <Windows.h>
#include <iostream>
#include <conio.h>
#include <stack>
using namespace std;
stack<HANDLE> threadIds;
HANDLE hMutex;
#define MAX_THREADS 20


#endif
#ifdef __linux__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <stack>
#include <pthread.h>
#include <curses.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
using namespace std;
#endif

#ifdef _WIN32
void GetMutex()
{
	WaitForSingleObject(hMutex, INFINITE);
}

void ReleaseMutexM()
{
	ReleaseMutex(hMutex);
}

DWORD WINAPI ActionFunc(LPVOID param)
{
	while (true)
	{
		GetMutex();
		int counter = 1;
		int number = threadIds.size();
		for (int param = 0; param < number; param++)
		{
			cout << counter << endl;
			counter++;
			Sleep(200);
		}
		ReleaseMutexM();
	}
}

void CreateThread()
{
	DWORD threadId;
	int msg = 0;
	auto newThread = CreateThread(NULL, 0, ActionFunc, (int*)msg, 0, &threadId);
	threadIds.push(newThread);
}

void AbortLastThread()
{
	if (threadIds.empty())
	{
		cout << "There's no threads to terminate" << endl;
		return;
	}
	auto lastId = threadIds.top();
	TerminateThread(lastId, NULL);
	threadIds.pop();

}

void AbortAllThreads()
{
	while (!threadIds.empty())
		AbortLastThread();
	cout << "All threads terminated" << endl;
}
int main()
{
	hMutex = CreateMutex(NULL, FALSE, NULL);
	if (hMutex == NULL){
		cout << "Error creating mutex. Program will exit" << endl;
		return EXIT_FAILURE;
	}
	while (true)
	{
		char inp;
		inp = _getch();
		switch (inp){
		case '+':
			CreateThread();
			break;
		case '-':
			AbortLastThread();
			break;
		case '*':
			cout << "Stack size: " << threadIds.size() << endl;
			break;
		case 'q':
			AbortAllThreads();
			return EXIT_SUCCESS;
		default:
			continue;
		}
	}
	return EXIT_SUCCESS;
}
#endif
#ifdef __linux__

stack<pthread_t> threadIds;
bool terminateLast = false;
pthread_mutex_t mutex;

void initMutex()
{
	if (pthread_mutex_init(&mutex, NULL) != 0){
		printf("\n mutex init failed\nProgram will exit\n");
		exit(0);
	}
}

void getMutex()
{
	pthread_mutex_lock(&mutex);
}

void releaseMutex()
{
	pthread_mutex_unlock(&mutex);
}


void* outputMethod(void* args)
{

	for (;;) {
		getMutex();
		if (terminateLast){
			terminateLast = false;
			releaseMutex();
			pthread_exit(NULL);
		}

		for (int i = 1; i < threadIds.size(); i++)
		{
			printf("Thread %i is running\n\r ", i);
			sleep(1);
		}
		releaseMutex();
	}
	return NULL;
}

void menuMessage()
{

	cout << "+ Create thread\r " << endl;
	cout << "- Abort last created thread\r " << endl;
	cout << "q Exit program\r " << endl;

}
void createThread()
{
	pthread_t *threadId = (pthread_t*)calloc(1, sizeof(pthread_t));
	pthread_create(&threadId[0], NULL, &outputMethod, NULL);
	threadIds.push(threadId[0]);
}

void abortLastThread()
{
	if (threadIds.empty()){
		cout << "No thread to kill" << endl;
		return;
	}
	pthread_t t_id = threadIds.top();
	threadIds.pop();
	terminateLast = true;
}


void abortAllThreads()
{
	while (!threadIds.empty())
		abortLastThread();
	puts("All threads aborted");
}

int main(int argc, char const *argv[])
{

	initscr();
	clear();
	noecho();
	refresh();
	nodelay(stdscr, TRUE);

	initMutex();
	menuMessage();

	while (1)
	{
		char key;
		key = getch();
		usleep(1000);
		fflush(stdout);
		refresh();

		switch (key)
		{
		case '+':
			createThread();
			break;
		case '-':
			abortLastThread();
			break;
		case 'q':
			abortAllThreads();
			exit(0);
			break;

		}
		refresh();
	}
	return 0;
}
#endif