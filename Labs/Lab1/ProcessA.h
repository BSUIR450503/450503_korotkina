#include <fstream>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#endif
#ifdef __linux__
#include <sys/wait.h> 
#include <sys/types.h> 
#include <unistd.h> 
#endif
using namespace std;
class  ProcessA
{
public:
	ProcessA();
	~ProcessA() {};
	int readFromFile();
	void proc(int, char**);
};
