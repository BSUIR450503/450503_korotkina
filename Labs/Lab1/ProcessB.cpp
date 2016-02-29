#include <fstream>
#include <iostream>
#ifdef _WIN32
#include <string>
#include <math.h>
#include <conio.h>
#endif
#ifdef _linux
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#endif
using namespace std;

int main(int argc, char* argv[])
{
	int out;
	int in;
	fstream inFile;
		inFile.open("mFile.bin", ios::app | ios::binary | ios::in );
		if (!inFile){
			cout << "error";
			return 0;
		}
	inFile>>in;
	inFile.close();

	out = in*in;
	
    inFile.open("mFile.bin", ios::binary | ios_base::trunc |ios::out );
	if(!inFile){
		cout<<"error";
		return 0;
	}
	inFile<<out;
	
	inFile.close();
	
	return 0;

}
