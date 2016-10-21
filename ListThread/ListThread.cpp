#include <Windows.h>
#include <process.h>
#include <time.h>
#include <stdio.h>
#include <conio.h>
#include <list>

#pragma comment(lib,"Winmm.lib")

using namespace std;

#define MAX_THREAD 6

unsigned __stdcall PrintThread(LPVOID printParam);
unsigned __stdcall DeleteThread(LPVOID deleteParam);
unsigned __stdcall WorkerThread(LPVOID workerParam);
unsigned __stdcall SaveThread(LPVOID saveParam);

list<int> g_NumList;
bool b_Shutdown = false;
HANDLE hEvent;
CRITICAL_SECTION cs;

void main()
{
	HANDLE hThread[MAX_THREAD];
	DWORD dwThreadId;
	
	InitializeCriticalSection(&cs);

	hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hEvent == NULL)
		return;


	hThread[0] = (HANDLE)_beginthreadex(
		NULL,
		0,
		SaveThread,
		0,
		0,
		(unsigned int *)&dwThreadId);

	hThread[1] = (HANDLE)_beginthreadex(
		NULL,
		0,
		PrintThread,
		0,
		0,
		(unsigned int *)&dwThreadId);

	hThread[2] = (HANDLE)_beginthreadex(
		NULL,
		0,
		DeleteThread,
		0,
		0,
		(unsigned int *)&dwThreadId);

	for (int iCnt = 3; iCnt < 6; iCnt++)
	{
		hThread[iCnt] = (HANDLE)_beginthreadex(
			NULL,
			0,
			WorkerThread,
			0,
			0,
			(unsigned int *)&dwThreadId);
	}

	while (1)
	{
		if (_kbhit() != 0)
		{
			char ch = _getch();

			if ('s' == ch || 'S' == ch)
				SetEvent(hEvent);

			else if ('q' == ch || 'Q' == ch)
			{
				SetEvent(hEvent);
				b_Shutdown = true;
				break;
			}
		}
	}

	WaitForMultipleObjects(MAX_THREAD, hThread, TRUE, INFINITE);
	DWORD error = GetLastError();
	DeleteCriticalSection(&cs);
}

unsigned __stdcall PrintThread(LPVOID printParam)
{
	DWORD dwTick = timeGetTime();
	list<int>::iterator iter;

	while (1)
	{
		if (b_Shutdown)
			return 0;

		if (timeGetTime() - dwTick >= 1000)
		{
			EnterCriticalSection(&cs);

			system("cls");
			for (iter = g_NumList.begin(); iter != g_NumList.end(); ++iter)
				printf("%d ", (*iter));

			if (!g_NumList.empty())printf("\n");

			LeaveCriticalSection(&cs);

			dwTick = timeGetTime();
		}
	}
}

unsigned __stdcall DeleteThread(LPVOID deleteParam)
{
	DWORD dwTick = timeGetTime();

	while (1)
	{
		if (b_Shutdown)
			return 0;

		if ((timeGetTime() - dwTick >= 1000) && !g_NumList.empty())
		{
			EnterCriticalSection(&cs);

			g_NumList.pop_back();

			LeaveCriticalSection(&cs);

			dwTick = timeGetTime();
		}
	}
}

unsigned __stdcall WorkerThread(LPVOID workerParam)
{
	DWORD dwTick = timeGetTime();
	list<int>::iterator iter;

	srand(time(NULL) + GetCurrentThreadId());

	while (1)
	{
		if (b_Shutdown)
			return 0;

		if (timeGetTime() - dwTick >= 1000)
		{
			EnterCriticalSection(&cs);

			g_NumList.push_back(rand());

			LeaveCriticalSection(&cs);

			dwTick = timeGetTime();
		}
	}
}

unsigned __stdcall SaveThread(LPVOID saveParam)
{
	FILE *fp = NULL;
	list<int>::iterator iter;

	while (1){
		WaitForSingleObject(hEvent, INFINITE);

		if (b_Shutdown)
			return 0;

		fopen_s(&fp, "List.txt", "w");
		if (NULL == fp)
			printf("fopen_s Fail\n");

		for (iter = g_NumList.begin(); iter != g_NumList.end(); ++iter)
		{
			fprintf_s(fp, "%d ", (*iter));
		}

		fclose(fp);
		ResetEvent(hEvent);
	}
}