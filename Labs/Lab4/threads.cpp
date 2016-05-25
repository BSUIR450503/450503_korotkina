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

static const char* strings[] = { "1.First\n\r", "2.Second\n\r", "3.Third\n\r",
"4.Fourth\n\r", "5.Fifth\n\r", "6.Sixth\n\r", "7.Seventh\n\r", "8.Eighth\n\r",
"9.Ninth\n\r", "10.Tenth\n\r" };


#ifdef _WIN32

DWORD WINAPI ActionFunc(LPVOID param)
{
	while (true)
	{
		WaitForSingleObject(hMutex, INFINITE);
		int number = threadIds.size();
		for (int counter = 1; counter - 1 < number; counter++)
		{
			for (int i = 0; i < strlen(strings[counter - 1]); i++)
			{
				cout << strings[counter - 1][i];
				Sleep(50);
			}
	}
		ReleaseMutex(hMutex);
	}
}

void CreateThread()
{
	DWORD threadId;
	int msg = 0;
	HANDLE newThread = CreateThread(NULL, 0, ActionFunc, (int*)msg, 0, &threadId);
	threadIds.push(newThread);
}

void AbortLastThread()
{
	if (threadIds.empty())
	{
		cout << "There's no threads to terminate" << endl;
		return;
	}
	HANDLE lastId = threadIds.top();
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

void* outputMethod(void* args)
{

	for (;;) {
		pthread_mutex_lock(&mutex);
		if (terminateLast){
			terminateLast = false;
			pthread_mutex_unlock(&mutex);
			pthread_exit(NULL);
		}

		int number = threadIds.size();
		for (int counter = 1; counter - 1 < number; counter++)
		{
			for (int i = 0; i < strlen(strings[counter - 1]); i++)
			{
				cout << strings[counter - 1][i];
				Sleep(1);
			}
		}
		pthread_mutex_unlock(&mutex);
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