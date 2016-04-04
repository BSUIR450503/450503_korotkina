#ifdef _WIN32
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
char* printEvents[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10" };
char* closeEvents[] = { "1c", "2c", "3c", "4c", "5c", "6c", "7c", "8c", "9c", "10c" };
#endif

#ifdef linux
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

char* nameProc[] = { "1.First\n\r", "2.Second\n\r", "3.Third\n\r",
"4.Fourth\n\r", "5.Fifth\n\r", "6.Sixth\n\r", "7.Seventh\n\r", "8.Eighth\n\r",
"9.Ninth\n\r", "10.Tenth\n\r" };

using namespace std;

void print(int number);


#ifdef __linux__
int printFlag = 0;
int endFlag = 1;

struct sigaction printSignal;
struct sigaction endSignal;

void setPrint(int sig)
{
	printFlag = 1;
}

void setEnd(int sig)
{
	endFlag = 1;
}
#endif

void print(int number)
{
	for (int i = 0; i < strlen(nameProc[number - 1]); i++)
	{
		cout << nameProc[number - 1][i];
#ifdef _WIN32
		Sleep(50);
#elif __linux__
		usleep(1000);
		fflush(stdout);
		refresh();
#endif
	}
	return;
}

