#include "ProcessA.h"

using namespace std;

ProcessA::ProcessA()
{
	int n;
        cout << "Input n" << endl;
	cin >> n;
	fstream inFile;
	inFile.open("mFile.bin",  ios::binary | ios::out | ios_base::trunc);
	if (!inFile){
		cout << "error";
		return;
	}
	inFile << n;
	inFile.close();
}



int ProcessA::readFromFile()
{
	int inf;
	fstream inFile;
	inFile.open("mFile.bin", ios::in | ios::binary);
	if (!inFile){
		cout << "error";
		return 0;
	}
	inFile >> inf;
	inFile.close();
	return inf;
}
#ifdef _WIN32
void ProcessA::proc(int argc, char* argv[])
{
	STARTUPINFO cif;
	ZeroMemory(&cif, sizeof(STARTUPINFO));
	PROCESS_INFORMATION pi;
	TCHAR czCommandLine[] = "ProcessB";
	
	CreateProcess(NULL, czCommandLine, NULL, NULL, FALSE, NULL, NULL, NULL, &cif, &pi);
	WaitForSingleObject(pi.hProcess, INFINITE);
	
	    	int inf = readFromFile();

		cout << "Result  "<< inf;
		
	 
}
#endif

#ifdef __linux__
void ProcessA::proc(int argc, char* argv[]){
	pid_t num;
	num = fork();

	if (num == 0)
	{
		execl("ProcessB", " ", NULL);
		exit(0);
	}
	else
	if (num > 0)
	{
		//printf("rod process\n\n");
 		wait(NULL);
		int inf = readFromFile();

		cout << "Result  " << inf;
		
	}
	
      
}
#endif
