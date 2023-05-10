#include "operator.h"

void printBoard(ControlData* cd) {
	_tprintf(TEXT("oi3\n"));
	for (int i = 0; i < cd->g->number_of_lanes; i++) {
		_tprintf(TEXT("\n"));
		for (int j = 0; j < MAX_BOARD_COL; j++) {
			_tprintf(TEXT("%c"), cd->g->board[i][j]);
		}
	}
	_tprintf(TEXT("\n"));
}

DWORD WINAPI threadFunc(LPVOID params) {
	ControlData *cd = (ControlData*) params;
	while (1) {
		WaitForSingleObject(cd->eventHandle, INFINITE);
		Sleep(1000);
		_tprintf(TEXT("\t[*] 3...\n"));
		Sleep(1000);
		_tprintf(TEXT("\t[*] 2...\n"));
		Sleep(1000);
		_tprintf(TEXT("\t[*] 1...\n"));
		Sleep(1000);
		_tprintf(TEXT("\t[*] Signal received!\n"));
		// Zum zum, faz cenas
		_tprintf(TEXT("Olá, o Tomás ajudou aqui, ou melhor? Em todo o trabalho LOOOOL"));
		_tprintf(TEXT("oi1\n"));
		printBoard(&cd);
		_tprintf(TEXT("oi2\n"));
		ResetEvent(cd->eventHandle);
	}
}

int _tmain(int argc, TCHAR* argv[]) {
	#ifdef UNICODE
		_setmode(_fileno(stdin), _O_WTEXT);
		_setmode(_fileno(stdout), _O_WTEXT);
		_setmode(_fileno(stderr), _O_WTEXT);
	#endif // UNICODE

	ControlData cd;
	cd.hSemRead = CreateSemaphore(NULL, 0, 1, TEXT("SemRead"));
	cd.hSemWrite = CreateSemaphore(NULL, 1, 1, TEXT("SemWrite"));
	cd.hMutex = CreateSemaphore(NULL, 1, 1, TEXT("SemMutex"));
	cd.eventHandle = CreateEvent(NULL, TRUE, FALSE, TEXT("BoardEvent"));

	HANDLE hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,
		FALSE,
		KEY_SHARED_MEMORY
	);
	if (hMapFile == NULL || hMapFile == INVALID_HANDLE_VALUE) {
		_tprintf(TEXT("[Operator.c/_tmain] Error opening shared memory\n"));
		return 1;
	}
	cd.g = (Game*)MapViewOfFile(
		hMapFile,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		0
	);
	if (cd.g == NULL || cd.g == INVALID_HANDLE_VALUE) {
		_tprintf(TEXT("[Operator.c/_tmain] Error creating shared memory pointer\n"));
		return 1;
	}

	// Create function to display the board
	printBoard(&cd);
	_tprintf(TEXT("number_of_lanes: %d\tinitial_speed: %d"), cd.g->number_of_lanes, cd.g->initial_speed);

	// Create threads. One for receive commands, another to send commands and another to update the board

	HANDLE hThread = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)threadFunc,
		&cd,
		0,
		NULL
	);

	if (hThread == NULL)
	{
		_tprintf(TEXT("[Server.c/init_server] Error creating server manager thread\n"));
		return;
	}

	WaitForSingleObject(hThread, INFINITE);

	//Wait for the threads to finish with WaitForSingleObject

	UnmapViewOfFile(cd.g);
	CloseHandle(hMapFile);
	CloseHandle(cd.hSemRead);
	CloseHandle(cd.hSemWrite);
	CloseHandle(cd.hMutex);
	return 0;
}