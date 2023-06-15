#include "operator.h"

void ascii_printer() {
	_tprintf(TEXT("\t   ____                          \n"));
	_tprintf(TEXT("\t  / __/______  ___ ____ ____ ____\n"));
	_tprintf(TEXT("\t / _// __/ _ \\/ _ `/ _ `/ -_) __/\n"));
	_tprintf(TEXT("\t/_/ /_/  \\___/\\_, /\\_, /\\__/_/   \n"));
	_tprintf(TEXT("\t             /___//___/          \n"));
	_tprintf(TEXT("\t[*] OPERATOR [*]\n"));
}

void clear_screen() {
	system("cls");
}

COORD get_cursor_pos() {
	CONSOLE_SCREEN_BUFFER_INFO cbsi;
	if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cbsi)) {
		return cbsi.dwCursorPosition;
	}
	else {
		COORD invalid = { 0, 0 };
		return invalid;
	}
}

void set_cursor_pos(COORD pos) {
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void print_board(ControlData* params) {
	ControlData* cd = (ControlData*)params;
	CRITICAL_SECTION* cs = &cd->cs;
	// Convert the game time to print in minutes and seconds
	int minutes = cd->g->game_time / 60;
	int seconds = cd->g->game_time % 60;
	EnterCriticalSection(cs);
	COORD current_pos = get_cursor_pos();
	COORD print_cord;
	print_cord.X = 0;
	print_cord.Y = 8;

	set_cursor_pos(print_cord);

	_tprintf(TEXT("Game time: %d:%d\n"), minutes, seconds);	
	_tprintf(TEXT("Score: \n"));
	_tprintf(TEXT("\tFrog 1: %d\n"), cd->g->f[0].score);
	_tprintf(TEXT("\tFrog 2: %d\n"), cd->g->f[1].score);
	for (int i = 0; i < cd->g->number_of_lanes; i++) {
		_tprintf(TEXT("\n"));
		for (int j = 0; j < MAX_BOARD_COL; j++) {
			_tprintf(TEXT("%c"), cd->g->board[i][j]);
		}
	}
	_tprintf(TEXT("\n\n"));

	set_cursor_pos(current_pos);
	LeaveCriticalSection(cs);
}

DWORD WINAPI update_board(LPVOID params) {
	ControlData* cd = (ControlData*)params;

	fflush(stdin);
	while (!cd->threadStop) {
		WaitForSingleObject(cd->eventHandle, INFINITE);
		if (cd->threadStop) break;
		//clear_screen();
		//ascii_printer();
		print_board(cd);
		ResetEvent(cd->eventHandle);
		//_tprintf(TEXT("[Operator.c/operator_manager] Enter a command: "));
	}
}

void clear_line(COORD coord) {
	COORD current_pos = coord;
	COORD print_cord;
	print_cord.X = 0;
	print_cord.Y = current_pos.Y;
	set_cursor_pos(print_cord);
	for (int i = 0; i < 1000; i++) {
		_tprintf(TEXT(" "));
	}
	set_cursor_pos(print_cord);
}

DWORD WINAPI operator_manager(LPVOID params) {
	ControlData* cd = (ControlData*)params;
	CRITICAL_SECTION* cs = &cd->cs;
	BufferItem buffer_item;
	TCHAR command[100];
	TCHAR command_buffer[100];
	TCHAR** args;

	while (!cd->threadStop) {
		EnterCriticalSection(cs);
		COORD current_pos = get_cursor_pos();
		COORD print_cord;
		print_cord.X = 0;
		print_cord.Y = 10 + cd->g->number_of_lanes + 3;
		set_cursor_pos(print_cord);
		fflush(stdin);
		//clearLine();
		_tprintf(TEXT("[Operator.c/operator_manager] Enter a command: "));
		LeaveCriticalSection(cs);
		if (WaitForSingleObject(cd->closingEvent, 0) == WAIT_OBJECT_0) {
			cd->threadStop = 1;
			break;
		}
		if (cd->threadStop) break;
		_fgetts(command, 100, stdin);
		command[_tcslen(command) - 1] = '\0';
		_tcscpy_s(command_buffer, 100, command);

		EnterCriticalSection(cs);

		// Check if the command received is an enter
		if (_tcscmp(command, TEXT("")) == 0) {
			continue;
		}

		args = (TCHAR**)malloc(sizeof(TCHAR*) * 10);
		int i = 0;
		TCHAR* next_token = NULL;
		TCHAR* token = _tcstok_s(command, TEXT(" "), &next_token);
		while (token != NULL) {
			args[i] = token;
			token = _tcstok_s(NULL, TEXT(" "), &next_token);
			i++;
		}
		args[i] = NULL;

		ZeroMemory(buffer_item.command, 100 * sizeof(TCHAR));

		//clearLine();

		if (!_tcscmp(args[0], TEXT("help")) && i == 1) {
			clear_line(print_cord);
			_tprintf(TEXT("\n\t\t[Operator.c/operator_manager] Available commands:\n"));
			_tprintf(TEXT("\t[Operator.c/operator_manager] Type 'stop' for the car movement to stop\n"));
			_tprintf(TEXT("\t[Operator.c/operator_manager] Type 'obstacle' to insert an obstacle\n"));
			_tprintf(TEXT("\t[Operator.c/operator_manager] Type 'invert' to invert the direction of travel\n"));
			_tprintf(TEXT("\t[Operator.c/operator_manager] Type 'clear' to clear the screen\n"));
			_tprintf(TEXT("\t[Operator.c/operator_manager] Type 'exit' to stop the operator\n"));
			_tprintf(TEXT("\n"));
		}
		else if (!_tcscmp(args[0], TEXT("exit")) && i == 1) {
			_tcscpy_s(buffer_item.command, 100, command_buffer);
			_tprintf(TEXT("\t[Operator.c/operator_manager] Stopping operator\n"));
			SetEvent(cd->closingEvent);
			Sleep(1000);
			break;
		}
		else if (!_tcscmp(args[0], TEXT("clear")) && i == 1) {
			clear_line(print_cord);
		}
		else if (!_tcscmp(args[0], TEXT("stop")) && i == 2) {
			_tcscpy_s(buffer_item.command, 100, command_buffer);
			buffer_item.pid = GetCurrentProcessId();
			_tprintf(TEXT("\t[Operator.c/operator_manager] Stopping car movement for '%d' seconds\n"), _ttoi(args[1]));
			WaitForSingleObject(cd->hSemWrite, INFINITE);
			WaitForSingleObject(cd->hMutex, INFINITE);

			ZeroMemory(&cd->g->buffer[cd->g->in], sizeof(BufferItem));
			CopyMemory(&cd->g->buffer[cd->g->in], &buffer_item, sizeof(BufferItem));
			cd->g->in++;

			if (cd->g->in == MAX_BUFFER_SIZE) cd->g->in = 0;

			ReleaseMutex(cd->hMutex);
			ReleaseSemaphore(cd->hSemRead, 1, NULL);
			Sleep(_ttoi(args[1]) * 1000);
			clear_line(print_cord);
		}
		else if (!_tcscmp(args[0], TEXT("invert")) && i == 2) {
			_tcscpy_s(buffer_item.command, 100, command_buffer);
			buffer_item.pid = GetCurrentProcessId();
			_tprintf(TEXT("\t[Operator.c/operator_manager] Inverting direction of travel for lane '%d'\n"), _ttoi(args[1]));
			WaitForSingleObject(cd->hSemWrite, INFINITE);
			WaitForSingleObject(cd->hMutex, INFINITE);

			ZeroMemory(&cd->g->buffer[cd->g->in], sizeof(BufferItem));
			CopyMemory(&cd->g->buffer[cd->g->in], &buffer_item, sizeof(BufferItem));
			cd->g->in++;

			if (cd->g->in == MAX_BUFFER_SIZE) cd->g->in = 0;

			ReleaseMutex(cd->hMutex);
			ReleaseSemaphore(cd->hSemRead, 1, NULL);
			Sleep(1000);
			clear_line(print_cord);
		}
		else if (!_tcscmp(args[0], TEXT("obstacle")) && i == 3) {
			if (_ttoi(args[1]) > 0 && _ttoi(args[1]) <= cd->g->number_of_lanes && _ttoi(args[2]) > 0 && _ttoi(args[2]) <= MAX_BOARD_COL) {
				_tcscpy_s(buffer_item.command, 100, command_buffer);
				buffer_item.pid = GetCurrentProcessId();
				_tprintf(TEXT("\t[Operator.c/operator_manager] Inserting obstacle at lane '%d', column '%d'\n"), _ttoi(args[1]), _ttoi(args[2]));
				SetEvent(cd->eventHandle);
				WaitForSingleObject(cd->hSemWrite, INFINITE);
				WaitForSingleObject(cd->hMutex, INFINITE);

				ZeroMemory(&cd->g->buffer[cd->g->in], sizeof(BufferItem));
				CopyMemory(&cd->g->buffer[cd->g->in], &buffer_item, sizeof(BufferItem));
				cd->g->in++;

				if (cd->g->in == MAX_BUFFER_SIZE) cd->g->in = 0;

				ReleaseMutex(cd->hMutex);
				ReleaseSemaphore(cd->hSemRead, 1, NULL);
				Sleep(1000);
				clear_line(print_cord);
			}
			else {
				_tprintf(TEXT("\t[Operator.c/operator_manager] Invalid lane or column\n"));
				Sleep(1000);
				clear_line(print_cord);
			}
		}
		else {
			_tprintf(TEXT("\t[Operator.c/operator_manager] Unknown command.\n"));
			Sleep(1000);
			clear_line(print_cord);
		}

		set_cursor_pos(current_pos);

		LeaveCriticalSection(cs);
	}

}

DWORD WINAPI server_command_receiver(LPVOID params) {
	ControlData* cd = (ControlData*)params;
	BufferItem buffer_item;

	while (!cd->threadStop) {
		WaitForSingleObject(cd->closingEvent, INFINITE);
		cd->threadStop = 1;
		fflush(stdin);
		Sleep(1000);
		_tprintf(TEXT("\n\t[Operator.c/receiveCommand] Shutting down...\n"));
		Sleep(1000);
		_tprintf(TEXT("\n\t[Operator.c/receiveCommand] Press any key to exit...\n"));
		_close(_fileno(stdin));
		ResetEvent(cd->closingEvent);
	}
}

int _tmain(int argc, TCHAR* argv[]) {
	#ifdef UNICODE
		_setmode(_fileno(stdin), _O_WTEXT);
		_setmode(_fileno(stdout), _O_WTEXT);
		_setmode(_fileno(stderr), _O_WTEXT);
	#endif // UNICODE

	ControlData cd;

	// Check if the server is running
	ascii_printer();
	HANDLE hSemServer = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, SEMAPHORE_SERVER);
	if (hSemServer == NULL) {
		_tprintf(TEXT("[Operator.c/_tmain] Server is not running. Please start the server and try again.\n"));
		return -1;
	}

	cd.hSemRead = CreateSemaphore(NULL, 0, 1, TEXT("SemRead"));
	cd.hSemWrite = CreateSemaphore(NULL, 1, 1, TEXT("SemWrite"));
	cd.hMutex = CreateMutex(NULL, FALSE, TEXT("SemMutex"));
	cd.eventHandle = CreateEvent(NULL, TRUE, FALSE, TEXT("BoardEvent"));
	cd.closingEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, TEXT("ClosingEvent"));
	cd.hTypingEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	cd.h_console = GetStdHandle(STD_OUTPUT_HANDLE);
	InitializeCriticalSectionAndSpinCount(&cd.cs, 200);

	if (cd.hSemRead == NULL || cd.hSemWrite == NULL || cd.hMutex == NULL || cd.eventHandle == NULL || cd.closingEvent == NULL || cd.hTypingEvent == NULL) {
		_tprintf(TEXT("[Operator.c/_tmain] Error creating semaphores\n"));
		return 1;
	}

	cd.threadStop = 0;

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

	print_board(&cd);

	// Create threads. One for receive commands, another to send commands and another to update the board

	HANDLE operator_manager_thread = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)operator_manager,
		&cd,
		0,
		NULL
	);

	if (operator_manager_thread == NULL)
	{
		_tprintf(TEXT("[Server.c/init_server] Error creating server manager thread\n"));
		UnmapViewOfFile(cd.g);
		CloseHandle(hMapFile);
		return -1;
	}

	HANDLE server_command_receiver_thread = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)server_command_receiver,
		&cd,
		0,
		NULL
	);

	if (server_command_receiver_thread == NULL)
	{
		_tprintf(TEXT("[Server.c/init_server] Error creating server manager thread\n"));
		UnmapViewOfFile(cd.g);
		CloseHandle(hMapFile);
		return -1;
	}

	HANDLE update_board_thread = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)update_board,
		&cd,
		0,
		NULL
	);

	if (update_board_thread == NULL)
	{
		_tprintf(TEXT("[Server.c/init_server] Error creating server manager thread\n"));
		UnmapViewOfFile(cd.g);
		CloseHandle(hMapFile);
		return -1;
	}

	//Wait for the threads to finish with WaitForSingleObject
	WaitForSingleObject(operator_manager_thread, INFINITE);
	WaitForSingleObject(server_command_receiver_thread, INFINITE);
	WaitForSingleObject(update_board_thread, INFINITE);

	UnmapViewOfFile(cd.g);
	CloseHandle(hMapFile);
	CloseHandle(cd.hSemRead);
	CloseHandle(cd.hSemWrite);
	CloseHandle(cd.hMutex);
	CloseHandle(hSemServer);
	CloseHandle(cd.eventHandle);
	CloseHandle(cd.closingEvent);
	CloseHandle(cd.hTypingEvent);
	return 0;
}