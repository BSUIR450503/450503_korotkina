#include "Win.h"

int main(int argc, char *argv[])
{
#ifdef _WIN32
	if (argc == 3)
	{
		HANDLE eventToPrint = OpenEvent(EVENT_ALL_ACCESS, FALSE, argv[1]);
		HANDLE eventToClose = OpenEvent(EVENT_ALL_ACCESS, FALSE, argv[2]);

		while (1)
		{
			if (WaitForSingleObject(eventToPrint, 1) == WAIT_OBJECT_0)
			{
				print(atoi(argv[1]));
				ResetEvent(eventToPrint);
			}
			if (WaitForSingleObject(eventToClose, 0) == WAIT_OBJECT_0)
			{
				CloseHandle(eventToClose);
				CloseHandle(eventToPrint);
				return 0;
			}
		}
	}
	else
	{
		Process* process[10];
		int count = 0;
		int curPrint = 1;
		while (1)
		{
			char key = getch_noblock();
			Sleep(1);
			switch (key)
			{
			case '+':
				if (count < 10)
				{
					count++;
					process[count - 1] = new Process(count, argv[0]);
				}
				break;
			case '-':
				if (count > 0)
				{
					count--;
					SetEvent(process[count]->closeEvent);
				}
				break;
			case 'q':
				if (count > 0)
				{
					for (int i = 0; i < count; i++)
						SetEvent(process[i]->closeEvent);
				}
				return 0;
			}

			if (count && WaitForSingleObject(process[curPrint - 1]->printEvent, 0) == WAIT_TIMEOUT)
			{
				if (curPrint >= count)
					curPrint = 1;
				else
					curPrint++;
				SetEvent(process[curPrint - 1]->printEvent);
			}
		}
	}

#elif __linux__
	pid_t process[10];
	char key;
	int count = 0;
	int currentPrinting = 1;

	initscr();
	clear();
	noecho();
	refresh();

	printSignal.sa_handler = setPrint;
	sigaction(SIGUSR1, &printSignal, NULL);

	endSignal.sa_handler = setEnd;
	sigaction(SIGUSR2, &endSignal, NULL);

	while (1)
	{
		key = getchar();
		usleep(1000);
		fflush(stdout);
		refresh();

		switch (key)
		{
		case '+':
			if (count < 10)
			{
				process[count] = fork();
				count++;

				switch (process[count - 1])
				{
				case 0:
				{
						  endFlag = 0;
						  while (!endFlag)
						  {
							  usleep(1000);
							  fflush(stdout);
							  refresh();
							  if (printFlag)
							  {
								  print(count);
								  refresh();
								  printFlag = 0;
								  kill(getppid(), SIGUSR2);
							  }
						  }
						  return 0;
						  break;
				}
				default:
					break;
				}
			}
			break;

		case '-':
			if (count > 0)
			{
				count--;
				kill(process[count], SIGUSR2);
				waitpid(process[count], NULL, 0);
				kill(process[count], SIGKILL);
				waitpid(process[count], NULL, 0);
			}
			break;

		case 'q':
			if (count > 0)
			{
				for (; count > 0; count--)
				{
					kill(process[count - 1], SIGUSR2);
					waitpid(process[count - 1], NULL, 0);
					kill(process[count - 1], SIGKILL);
					waitpid(process[count - 1], NULL, 0);
				}
			}
			clear();
			endwin();
			return 0;
		}
		if (count && endFlag)
		{
			endFlag = 0;
			if (currentPrinting >= count)
				currentPrinting = 1;
			else
				currentPrinting++;
			kill(process[currentPrinting - 1], SIGUSR1);
		}
		refresh();
	}
#endif
}