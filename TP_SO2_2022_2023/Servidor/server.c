#include "server.h"

void ascii_printer() {
	_tprintf(TEXT("\t   ____                          \n"));
	_tprintf(TEXT("\t  / __/______  ___ ____ ____ ____\n"));
	_tprintf(TEXT("\t / _// __/ _ \\/ _ `/ _ `/ -_) __/\n"));
	_tprintf(TEXT("\t/_/ /_/  \\___/\\_, /\\_, /\\__/_/   \n"));
	_tprintf(TEXT("\t             /___//___/          \n"));
	_tprintf(TEXT("\t[*] SERVER [*]\n"));
	_tprintf(TEXT("\n"));
}

void printBoard(Game* game) {
	for (int i = 0; i < game->number_of_lanes; i++) {
		_tprintf(TEXT("\n"));
		for (int j = 0; j < MAX_BOARD_COL; j++) {
			_tprintf(TEXT("%c"),game->board[i][j]);
		}
	}
	_tprintf(TEXT("\n"));
}

void boardInitializer(ControlData* cd) {
	for (int i = 0; i < cd->g->number_of_lanes; i++) {
		for (int j = 0; j < MAX_BOARD_COL; j++) {
			if (i == 0 || i == cd->g->number_of_lanes - 1) {
				cd->g->board[i][j] = TEXT('-');
			}
			else {
				cd->g->board[i][j] = TEXT('.');
			}
		}
	}
}

void clear_screen() {
	system("cls");
}

void frogger(ControlData* cd){
	cd->f1->position_y = rand() % 20;
	cd->f2->position_y = rand() % 20;

	while (cd->f2->position_y == cd->f1->position_y) {
		cd->f2->position_y = rand() % 20;
	}
	cd->g->board[cd->g->number_of_lanes-1][cd->f1->position_y] = TEXT('s1');
	cd->g->board[cd->g->number_of_lanes-1][cd->f2->position_y] = TEXT('s2');
}

DWORD WINAPI placeCar(LPVOID car) {
	ControlData* cd = (ControlData*)car;
	int row = 1;
	int col = 0;
	int carIndex = 0;

	for (int i = 0; i < cd->g->number_of_lanes-2; i++) {
		for (int j = col; j < 4; j++) {
			if (row >= 0 && row < cd->g->number_of_lanes && col >= 0 && col < MAX_BOARD_COL) {
				cd->g->board[row][col] = TEXT('C');
				cd->car[carIndex].position_x = row;;
				cd->car[carIndex].position_y = col;
				carIndex++;
				col+=2;
			}
		}
		col = 0;
		row++;	
	}
}

DWORD WINAPI runCar(LPVOID carRun) {
	ControlData* cd = (ControlData*)carRun;

	int carNumber = 12;
	int previousPos = 0;
	int newPos = 0;
	while (!cd->threadStop)
	{
		for (int i = 0; i < carNumber; i++) {
			Sleep(10000);
			previousPos = cd->car[i].position_y;
			
			if (cd->g->invert == 0) {
				int difference = (MAX_BOARD_COL - 1) - previousPos;
				if (difference < cd->g->initial_speed)
				{
					newPos = cd->car[i].position_y = 0 + difference;
				}
				else
				{
					newPos = cd->car[i].position_y += cd->g->initial_speed;
				}
			}
			else if (cd->g->invert == 1) {
				int difference = 0 + previousPos;
				if (difference < cd->g->initial_speed) {
					newPos = cd->car[i].position_y = 19 - difference;
				}
				else {
					newPos = cd->car[i].position_y -= cd->g->initial_speed;
				}
			}
			cd->g->board[cd->car[i].position_x][previousPos] = TEXT('.');
			cd->g->board[cd->car[i].position_x][newPos] = TEXT('C');
			SetEvent(cd->eventHandle);
		}
		
	}
	
}

DWORD WINAPI server_manager(LPVOID lparam) {
	ControlData *cd = (ControlData*)lparam;
	BufferItem buffer_item;
	TCHAR command[100];
	TCHAR command_buffer[100];
	cd->f1 = (Frogs*)malloc(sizeof(Frogs));
	cd->f2 = (Frogs*)malloc(sizeof(Frogs));
	frogger(cd);
	placeCar(cd);
	clear_screen();
	ascii_printer();

	while (!cd->threadStop) {
		_tprintf(TEXT("[Server.c/ server_manager] Enter a command: "));
		Sleep(5000);
		_fgetts(command, 100, stdin);
		command[_tcslen(command) - 1] = '\0';
		_tcscpy_s(command_buffer, 100, command);

		if (_tcscmp(command, TEXT("")) == 0) {
			continue;
		}

		ZeroMemory(buffer_item.command, 100 * sizeof(TCHAR));

		if (_tcscmp(command, TEXT("help")) == 0) {
			_tprintf(TEXT("\t[Server.c/ server_manager] Available commands:\n"));
			_tprintf(TEXT("\t\t[Server.c/ server_manager] help\n"));
			_tprintf(TEXT("\t\t[Server.c/ server_manager] clear\n"));
			_tprintf(TEXT("\t\t[Server.c/ server_manager] exit\n"));
		}
		else if (_tcscmp(command, TEXT("exit")) == 0) {
			_tprintf(TEXT("\t[!] Server shutting down...\n"));
			_tprintf(TEXT("\t[~] Exiting...\n"));
			// Send he exit command to the client
			_tcscpy_s(buffer_item.command, 100, TEXT("exit"));
			WaitForSingleObject(cd->hSemWrite, INFINITE);
			WaitForSingleObject(cd->hMutex, INFINITE);
			ZeroMemory(&cd->g->buffer[cd->g->in], sizeof(BufferItem));
			CopyMemory(&cd->g->buffer[cd->g->in], &buffer_item, sizeof(BufferItem));
			cd->g->in++;
			if (cd->g->in == MAX_BUFFER_SIZE) cd->g->in = 0;

			ReleaseMutex(cd->hMutex);
			ReleaseSemaphore(cd->hSemRead, 1, NULL);
			cd->threadStop = 1;
			SetEvent(cd->eventHandle);
			Sleep(1000);
			exit(0);
		}
		else if (_tcscmp(command, TEXT("clear")) == 0) {
			clear_screen();
			ascii_printer();
		}
		else {
			_tprintf(TEXT("\tUnknown command.\n"));
			Sleep(1000);
			clear_screen();
			ascii_printer();
		}
	}
}

DWORD WINAPI operator_command_receiver(LPVOID lparam) {
	ControlData* cd = (ControlData*)lparam;
	TCHAR command[50][50];
	BufferItem buffer_item;
	TCHAR** args;

	while (!cd->threadStop) {
		WaitForSingleObject(cd->hSemRead, INFINITE);
		WaitForSingleObject(cd->hMutex, INFINITE);
		CopyMemory(&buffer_item, &cd->g->buffer[cd->g->out], sizeof(BufferItem));
		cd->g->out++;

		if (cd->g->out == MAX_BUFFER_SIZE) cd->g->out = 0;

		ReleaseMutex(cd->hMutex);
		ReleaseSemaphore(cd->hSemWrite, 1, NULL);

		_tprintf(TEXT("\t[*] Command received: %s\n"), buffer_item.command);

		args = (TCHAR**)malloc(sizeof(TCHAR*) * 10);
		int i = 0;
		TCHAR *next_token = NULL;
		TCHAR *token = _tcstok_s(command, TEXT(" "), &next_token);
		while (token != NULL) {
			args[i] = token;
			token = _tcstok_s(NULL, TEXT(" "), &next_token);
			i++;
		}
		args[i] = NULL;

		if (_tcscmp(args[0], _T("stop")) == 0) {
			DWORD SuspendThread(HANDLE runCar);
			Sleep(args[1]);
			DWORD ResumeThread(HANDLE runCar);
		}
		else if(_tcscmp(args[0], _T("obstacle")) == 0) {
			TCHAR y = args[1];
			TCHAR x = args[2];
			y = _wtoi(args[1]);
			x = _wtoi(args[2]);

			cd->g->board[y][x] = _T('@');
			SetEvent(cd->eventHandle);
			ResetEvent(cd->eventHandle);
		}
		else if (_tcscmp(args[0], _T("invert")) == 0) {
			cd->g->invert = !cd->g->invert;
		}
	}
	return 0;
}

int _tmain(int argc, TCHAR* argv[]) {
	#ifdef UNICODE
		_setmode(_fileno(stdin), _O_WTEXT);
		_setmode(_fileno(stdout), _O_WTEXT);
		_setmode(_fileno(stderr), _O_WTEXT);
	#endif // UNICODE
	
		ControlData cd;
		int nr_of_lanes = 0;
		int init_speed = 0;
		srand(time(NULL));

		// Server instance controller

		HANDLE hSemaphoreServer = CreateSemaphore(
			NULL,
			1,
			MAX_SERVER,
			SEMAPHORE_SERVER
		);

		if (GetLastError() == ERROR_ALREADY_EXISTS) {
			_ftprintf(stderr, TEXT("\n\t[!] Error! Another instance of 'server' already running\n"));
			return -1;
		}

		ascii_printer();

		// Verify if there is a registry file already
		BOOL result = verifyRegistry();

		if (result == -1)
		{
			_tprintf(TEXT("[Server.c/init_server] No values stored in Registry!\n"));
			// Ask for number of lanes and initial speed to the user

			if (argc == 3) {
				nr_of_lanes = _ttoi(argv[1]);
				init_speed = _ttoi(argv[2]);
				// Print values
				_tprintf(TEXT("[Server.c/init_server] Number of lanes: %d\n"), nr_of_lanes);
				_tprintf(TEXT("[Server.c/init_server] Initial speed: %d\n"), init_speed);
				BOOL result = createRegistry(nr_of_lanes, init_speed);
				if (result == -1) {
					_tprintf(TEXT("\t[Server.c/init_server] Error creating registry file!\n"));
					return -1;
				}
			}
			else
			{
				_tprintf(TEXT("[Server.c/init_server] Invalid number of arguments!\n"));
				return -1;
			}
		}
		else
		{
			_tprintf(TEXT("[Server.c/init_server] Values stored in Registry!\n"));
			// Read values from registry
			BOOL result = readRegistry(KEY_ROAD_LANES);
			if (result == -1) {
				_tprintf(TEXT("\t[Server.c/init_server] Error reading registry file!\n"));
				return -1;
			}
			else {
				nr_of_lanes = result;
			}

			result = readRegistry(KEY_INIT_SPEED);
			if (result == -1) {
				_tprintf(TEXT("\t[Server.c/init_server] Error reading registry file!\n"));
				return -1;
			}
			else {
				init_speed = result;
			}
			// Print values
			_tprintf(TEXT("\t[Server.c/init_server] Number of lanes: %d\n"), nr_of_lanes);
			_tprintf(TEXT("\t[Server.c/init_server] Initial speed: %d\n"), init_speed);
		}


		// Shared memory 

		HANDLE hMapFile = CreateFileMapping(
			INVALID_HANDLE_VALUE,    // use paging file
			NULL,                    // default security
			PAGE_READWRITE,          // read/write access
			0,                       // maximum object size (high-order DWORD)
			sizeof(Game),     // maximum object size (low-order DWORD)
			KEY_SHARED_MEMORY);   // name of mapping object

		if (hMapFile == NULL)
		{
			_tprintf(TEXT("[Server.c/init_server] Error creating shared memory\n"));
			return 1;
		}

		_tprintf(TEXT("[Server.c/init_server] Shared memory created successfully\n"));

		/*HMODULE hDLL = LoadLibrary(TEXT("DLL.dll"), NULL, 0);
		if (hDLL == NULL) {
			_tprintf(TEXT("[DLL_main.c/_tmain] Error loading DLL.dll.\n"));
			return 1;
		}
		else {
			_tprintf(TEXT("[DLL_main.c/_tmain] DLL.dll loaded successfully.\n"));
		}*/


		// Create pointer for Shared Memory
		cd.g = (Game*)MapViewOfFile(hMapFile,   // handle to map object
			FILE_MAP_ALL_ACCESS, // read/write permission
			0,
			0,
			0);

		if (cd.g == NULL || cd.g == INVALID_HANDLE_VALUE) {
			_tprintf(TEXT("[Server.c/init_server] Error creating shared memory pointer\n"));
			return 1;
		}
		_tprintf(TEXT("[Server.c/init_server] Shared memory pointer created successfully\n"));

		// Initialize shared memory

		cd.eventHandle = CreateEvent(NULL, TRUE, FALSE, TEXT("BoardEvent"));
		if (cd.eventHandle == NULL) {
			_tprintf(TEXT("[Server.c/init_server] Error creating event\n"));
			return -1;
		}
		cd.hMutex = CreateMutex(NULL, FALSE, TEXT("SemMutex"));
		if (cd.hMutex == NULL) {
			_tprintf(TEXT("[Server.c/init_server] Error creating mutex\n"));
			return -1;
		}
		cd.hSemRead = CreateSemaphore(NULL, 0, MAX_BUFFER_SIZE, TEXT("SemRead"));
		if (cd.hSemRead == NULL) {
			_tprintf(TEXT("[Server.c/init_server] Error creating semaphore\n"));
			return -1;
		}
		cd.hSemWrite = CreateSemaphore(NULL, MAX_BUFFER_SIZE, MAX_BUFFER_SIZE, TEXT("SemWrite"));
		if (cd.hSemWrite == NULL) {
			_tprintf(TEXT("[Server.c/init_server] Error creating semaphore\n"));
			return -1;
		}

		cd.g->number_of_lanes = nr_of_lanes;
		cd.g->initial_speed = init_speed;
		cd.g->in = 0;
		cd.g->out = 0;
		cd.threadStop = 0;

		// Create a thread to manage the server
		// Lots of threads going on, one for acceptiing clients, others for managing the clients and messages
		Sleep(3000);

		HANDLE server_manager_thread = CreateThread(
			NULL,
			0,
			server_manager,
			&cd,
			0,
			NULL
		);

		if (server_manager_thread == NULL)
		{
			_tprintf(TEXT("[Server.c/init_server] Error creating server manager thread\n"));
			return;
		}

		HANDLE operator_command_receiver_thread = CreateThread(
			NULL,
			0,
			operator_command_receiver,
			&cd,
			0,
			NULL
		);

		if (operator_command_receiver_thread == NULL)
		{
			_tprintf(TEXT("[Server.c/init_server] Error creating server manager thread\n"));
			return;
		}

		HANDLE carThread = CreateThread(
			NULL,
			0,
			runCar,
			&cd,
			0,
			NULL
		);


		if (carThread == NULL) {
			_tprintf(TEXT("[Server.c/init_server] Error creating server manager thread\n"));
			return;
		}



		//Comunicação server frog
		int i;
		HANDLE hPipe, hThread, hEventTemp;
		int numFrogs = 0;
		TCHAR buf[256];
		BOOL ret;
		DWORD n, offset, nBytes;

		for (i = 0; i < MAX_FROGS; i++) {

			//criar evento que vai ser associado à estrutra overlaped
			hEventTemp = CreateEvent(NULL, TRUE, FALSE, NULL);

			if (hEventTemp == NULL) {
				_tprintf(TEXT("[ERROR] creating event\n"));
				return -1;
			}


			// aqui passamos a constante FILE_FLAG_OVERLAPPED para o named pipe aceitar comunicações assincronas
			hPipe = CreateNamedPipe(NOME_PIPE,
				PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
				PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
				2,
				256 * sizeof(TCHAR),
				256 * sizeof(TCHAR),
				1000,
				NULL);

			if (hPipe == INVALID_HANDLE_VALUE) {
				_tprintf(TEXT("[ERROR] creating Named Pipe! (CreateNamedPipe)\n"));
				exit(-1);
			}


			ZeroMemory(&cd.data->hPipes[i].overlap, sizeof(cd.data->hPipes[i].overlap));
			cd.data->hPipes[i].hInstancia = hPipe;
			cd.data->hPipes[i].overlap.hEvent = hEventTemp;
			cd.data->hEvents[i] = hEventTemp;
			cd.data->hPipes[i].activo = FALSE;

			//repetir codigo thread
			//fora do for esperar pelos eventos
			//em cada read fazer zero memory e colocar eventos
			//mandar estrutura overlapped nos reads e writes

			if (ConnectNamedPipe(hPipe, &cd.data->hPipes[i].overlap)) {
				_tprintf(_T("\nError while connecting to the client...\n"));
				exit(-1);
			}
		}

		while (numFrogs < MAX_FROGS + 1) {
			DWORD result = WaitForMultipleObjects(MAX_FROGS, cd.data->hEvents, FALSE, 1000);
			i = result - WAIT_OBJECT_0;
			if (i >= 0 && i < MAX_FROGS) {
				_tprintf(_T("\nClient connected...\n"));

				if (GetOverlappedResult(cd.data->hPipes[i].hInstancia, &cd.data->hPipes[i].overlap, &nBytes, FALSE)) {
					ResetEvent(cd.data->hEvents[i]);
					WaitForSingleObject(cd.data->hMutex, INFINITE);
					cd.data->hPipes[i].activo = TRUE;
					ReleaseMutex(cd.data->hMutex);
				}
				numFrogs++;
			}

		}

		for (i = 0; i < MAX_FROGS; i++) {
			_tprintf(_T("\nShutting down the the pipe.\n"));
			if (!DisconnectNamedPipe(cd.data->hPipes[i].hInstancia)) {
				_tprintf(_T("\nError shutting down the pipe (DisconnectNamedPipe) %d.\n"), GetLastError());
				exit(-1);
			}
			CloseHandle(cd.data->hPipes[i].hInstancia);
		}



		return 0;
}

		boardInitializer(&cd);

		WaitForSingleObject(server_manager_thread, INFINITE);
		WaitForSingleObject(operator_command_receiver_thread, INFINITE);
		WaitForSingleObject(carThread, INFINITE);

		// Wait for the threads to finish with WaitForSingleObject
		UnmapViewOfFile(cd.g);
		CloseHandle(hMapFile);
		CloseHandle(hSemaphoreServer);
		CloseHandle(cd.eventHandle);
		CloseHandle(cd.hMutex);
		CloseHandle(cd.hSemRead);
		CloseHandle(cd.hSemWrite);

	return 0;
}