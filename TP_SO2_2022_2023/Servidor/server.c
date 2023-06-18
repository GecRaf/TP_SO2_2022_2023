#include "server.h"

void ascii_printer() {
	// This function will print the ascii art of the game

	_tprintf(TEXT("\t   ____                          \n"));
	_tprintf(TEXT("\t  / __/______  ___ ____ ____ ____\n"));
	_tprintf(TEXT("\t / _// __/ _ \\/ _ `/ _ `/ -_) __/\n"));
	_tprintf(TEXT("\t/_/ /_/  \\___/\\_, /\\_, /\\__/_/   \n"));
	_tprintf(TEXT("\t             /___//___/          \n"));
	_tprintf(TEXT("\t[*] SERVER [*]\n"));
	_tprintf(TEXT("\n"));
}

void set_cursor_pos(COORD pos) {
	// This function will set the cursor position to the given position

	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void clear_line(COORD coord) {
	// Function responsible for clearing a fixed size of the screen (800 chars)
	// from a given position, allowing for the constant printing of the server command reader and the operators log

	COORD current_pos = coord;
	COORD print_cord;
	print_cord.X = 0;
	print_cord.Y = current_pos.Y;
	set_cursor_pos(print_cord);
	for (int i = 0; i < 800; i++) {
		_tprintf(TEXT(" "));
	}
	set_cursor_pos(print_cord);
}

void boardInitializer(ControlData* cd) {
	// Function responsible for initializing the board with the given number of lanes
	// It is called only once at the start of the server.

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

	// Initialize the lanes
	for (int i = 0; i < cd->g->number_of_lanes; i++) {
		cd->g->l[i].direction = 0;
		cd->g->l[i].car_speed = cd->g->initial_speed;
		cd->g->l[i].car_distance = 2;
	}

	cd->g->game_time = DEFAULT_GAME_TIME;
	cd->g->game_level = 1;
}

void clear_screen() {
	// Function responsible for clearing the screen

	system("cls");
}

void frogger(ControlData* cd){
	// Function responsible for initializing the frogs in the board
	// Both frogs will be placed in the last lane of the board, in random positions

	cd->f1->position_y = rand() % 20;
	cd->f2->position_y = rand() % 20;

	while (cd->f2->position_y == cd->f1->position_y) {
		cd->f2->position_y = rand() % 20;
	}
	cd->g->board[cd->g->number_of_lanes-1][cd->f1->position_y] = TEXT('s1');
	cd->g->board[cd->g->number_of_lanes-1][cd->f2->position_y] = TEXT('s2');
}

COORD get_cursor_pos() {
	// Function responsible for getting the current cursor position

	CONSOLE_SCREEN_BUFFER_INFO cbsi;
	if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cbsi)) {
		return cbsi.dwCursorPosition;
	}
	else {
		COORD invalid = { 0, 0 };
		return invalid;
	}
}

DWORD WINAPI game_verifications(LPVOID params) {
	// This thread will be responsible for verifying all the game rules and plays
	// It will run until the game is over

	ControlData* cd = (ControlData*)params;
	Game* game = cd->g;
	Frogs* frog = cd->f1;  // Assuming f1 is the player's frog
	Cars* cars = cd->car;  // Assuming car is the array of cars
	Lanes* lanes = game->l;  // Assuming l is the array of lanes
	ThreadDados* tData = cd->Td;  // Assuming td is the array of threads data
	Data data;

	while (!cd->threadStop) {
		TCHAR keyPressed[100];
		DWORD bytesRead;

		// Wait for the signal hTypingEvent to be set
		WaitForSingleObject(cd->hTypingEvent, INFINITE);

		// Check what named pipe was written to
		// There can be two named pipes, one for each player
		// The named pipe that was written to will be the one that has Data structure with the pressed key
		// The other named pipe will have the Data structure with the pressed key empty

		// Check if the named pipe that has the pressed key is the player's named pipe
		if (WaitForSingleObject(cd->Td->hPipes[0].hInstancia, 0) == WAIT_OBJECT_0) {
			// Read the pressed key from the player's named pipe
			ReadFile(cd->Td->hPipes[0].hInstancia, &data, sizeof(Data), &bytesRead, NULL);
		}
		// Check if the named pipe that has the pressed key is the other player's named pipe
		else if (WaitForSingleObject(cd->Td->hPipes[1].hInstancia, 0) == WAIT_OBJECT_0) {
			// Read the pressed key from the other player's named pipe
			ReadFile(cd->Td->hPipes[1].hInstancia, &data, sizeof(Data), &bytesRead, NULL);
		}

		// Copy the pressed key from the Data structure
		EnterCriticalSection(&cd->cs);
		_tcscpy_s(keyPressed, 100, data.pressedKey);  // Assuming index 0 is the player's index
		LeaveCriticalSection(&cd->cs);

		// Check if the game_time is at 0, meaning the game is over
		// If it is, break the loop
		if (game->game_time == 0) {
			break;
		}

		// Perform game verifications based on the pressed key
		if (!_tcscmp(keyPressed, TEXT("VK_LEFT"))) {
			// Check if the player's frog is in the first lane
			// If it is, move the frog
			if (frog->position_y == 0) {
				// Check if the desired position exists
				// If it doesn't, move the frog to the other side of the board
				if (game->board[game->number_of_lanes - 1][frog->position_y + 1] == TEXT('-')) {
					game->board[game->number_of_lanes - 1][frog->position_y] = TEXT('.');
					game->board[game->number_of_lanes - 1][frog->position_y + 1] = TEXT('s');
					frog->position_y++;
				}
			}
			// Check if the desired position is empty
			// If it is, move the frog
			else if (game->board[game->number_of_lanes - 1][frog->position_y - 1] == TEXT('.')) {
				game->board[game->number_of_lanes - 1][frog->position_y] = TEXT('.');
				game->board[game->number_of_lanes - 1][frog->position_y - 1] = TEXT('s');
				frog->position_y--;
			}
			// Check if the desired position is a car
			// If it is, the game must display a game over message
			else if (game->board[game->number_of_lanes - 1][frog->position_y - 1] == TEXT('C')) {
				// Display game over message
				// Set the game over flag to true
				// Stop the game
				_tprintf(TEXT("Game Over!"));
				cd->threadStop = 1;
			}
			// Check if the desired position is part of the final lane
			// If it is, check if the game_level is at 3, that is the max, if it is, the player won, if not advances to the next level
			else if (game->board[game->number_of_lanes - 1][frog->position_y - 1] == TEXT('-')) {
				if (game->game_level == 3) {
					// Display game won message
					// Set the game over flag to true
					// Stop the game
					_tprintf(TEXT("You Won!"));
					cd->threadStop = 1;
				}
				else {
					// Display level up message
					// Increase the game_level
					// Reset the player's frog position
					_tprintf(TEXT("Level Up!"));
					game->game_level++;
					game->board[game->number_of_lanes - 1][frog->position_y] = TEXT('.');
					frog->position_y = rand() % 20;
					game->board[game->number_of_lanes - 1][frog->position_y] = TEXT('S');
					// Each time the player advances to the next level, the cars must move faster 15% and the game_time must decrease by a percentage of 15%
					for (int i = 0; i < game->number_of_lanes - 1; i++) {
						//lanes[i].lane_speed = lanes[i].lane_speed * 0.85;
					}
					game->game_time = game->game_time * 0.85;
				}
			}
		}
		else if (!_tcscmp(keyPressed, TEXT("VK_RIGHT"))) {
			// Check if the player's frog is in the first lane
			// If it is, move the frog
			if (frog->position_y == 0) {
				// Check if the desired position exists
				// If it doesn't, move the frog to the other side of the board
				if (game->board[game->number_of_lanes - 1][frog->position_y + 1] == TEXT('-')) {
					game->board[game->number_of_lanes - 1][frog->position_y] = TEXT('.');
					game->board[game->number_of_lanes - 1][frog->position_y + 1] = TEXT('s');
					frog->position_y++;
				}
			}
			// Check if the desired position is empty
			// If it is, move the frog
			else if (game->board[game->number_of_lanes - 1][frog->position_y + 1] == TEXT('.')) {
				game->board[game->number_of_lanes - 1][frog->position_y] = TEXT('.');
				game->board[game->number_of_lanes - 1][frog->position_y + 1] = TEXT('s');
				frog->position_y++;
			}
			// Check if the desired position is a car
			// If it is, the game must display a game over message
			else if (game->board[game->number_of_lanes - 1][frog->position_y + 1] == TEXT('C')) {
				// Display game over message
				// Set the game over flag to true
				// Stop the game
				_tprintf(TEXT("Game Over!"));
				cd->threadStop = 1;
			}
		}
		else if (!_tcscmp(keyPressed, TEXT("VK_UP"))) {
			// Check if the desired position is part of the final lane
			// If it is, check if the game_level is at 3, that is the max, if it is, the player won, if not advances to the next level
			if (game->board[game->number_of_lanes - 1][frog->position_y] == TEXT('-')) {
				if (game->game_level == 3) {
					// Display game won message
					// Set the game over flag to true
					// Stop the game
					_tprintf(TEXT("You Won!"));
					cd->threadStop = 1;
				}
				else {
					// Display level up message
					// Increase the game_level
					// Reset the player's frog position
					_tprintf(TEXT("Level Up!"));
					game->game_level++;
					game->board[game->number_of_lanes - 1][frog->position_y] = TEXT('.');
					frog->position_y = rand() % 20;
					game->board[game->number_of_lanes - 1][frog->position_y] = TEXT('S');
					// Each time the player advances to the next level, the cars must move faster 15% and the game_time must decrease by a percentage of 15%
					for (int i = 0; i < game->number_of_lanes - 1; i++) {
						//lanes[i].lane_speed = lanes[i].lane_speed * 0.85;
					}
					game->game_time = game->game_time * 0.85;
				}
			}
			// Else , check if the desired position is empty
			// If it is, move the frog
			else if (game->board[game->number_of_lanes - 1][frog->position_y] == TEXT('.')) {
				game->board[game->number_of_lanes - 1][frog->position_y] = TEXT('s');
				game->board[game->number_of_lanes - 2][frog->position_y] = TEXT('.');
				frog->position_x--;
			}
			// Else, check if the desired position is a car
			// If it is, the game must display a game over message
			else if (game->board[game->number_of_lanes - 1][frog->position_y] == TEXT('C')) {
				// Display game over message
				// Set the game over flag to true
				// Stop the game
				_tprintf(TEXT("Game Over!"));
				cd->threadStop = 1;
			}
		}
		else if (!_tcscmp(keyPressed, TEXT("VK_DOWN"))) {
			// Handle down movement logic
			// Check if the desired position is out of bounds
			// If it is, does nothing, else, move the frog
			if (frog->position_x == game->number_of_lanes - 1) {
				// Does nothing
			}
			else {
				// Check if the desired position is empty
				// If it is, move the frog
				if (game->board[game->number_of_lanes - 2][frog->position_y] == TEXT('.')) {
					game->board[game->number_of_lanes - 2][frog->position_y] = TEXT('s');
					game->board[game->number_of_lanes - 1][frog->position_y] = TEXT('.');
					frog->position_x++;
				}
				// Else, check if the desired position is a car
				// If it is, the game must display a game over message
				else if (game->board[game->number_of_lanes - 2][frog->position_y] == TEXT('C')) {
					// Display game over message
					// Set the game over flag to true
					// Stop the game
					_tprintf(TEXT("Game Over!"));
					cd->threadStop = 1;
				}
			}
		}
		else {
			// Invalid key pressed, ignore or handle accordingly
		}

		// Reset the pressed key in the Data structure
		EnterCriticalSection(&cd->cs);
		//_tcscpy_s(data->pressedKey, 100, TEXT(""));  // Assuming index 0 is the player's index
		LeaveCriticalSection(&cd->cs);
	}
}

DWORD WINAPI placeCar(LPVOID car) {
	// This thread will be responsible for placing the cars in the board
	
	ControlData* cd = (ControlData*)car;
	Lanes *l = cd->g->l;
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
				// Fill the lane with the car
				l[row].number_of_cars++;
			}
		}
		col = 0;
		row++;	
	}
}

DWORD WINAPI runCar(LPVOID carRun) {
	// This thread will be responsible for moving the cars in the board

	ControlData* cd = (ControlData*)carRun;
	CRITICAL_SECTION* cs = &cd->cs;
	Lanes *l = cd->g->l;
	int carNumber = 12;
	int previousPos = 0;
	int newPos = 0;

	COORD current_pos = get_cursor_pos();

	COORD command_pos;
	command_pos.X = 43;
	command_pos.Y = 8;

	COORD print_cord;
	print_cord.X = 5;
	print_cord.Y = 15;
	set_cursor_pos(print_cord);

	while (!cd->threadStop)
	{
		WaitForSingleObject(cd->restartEvent, INFINITE); // TODO: Review this later, it has to be another event
		for (int i = 0; i < carNumber; i++) {
			Sleep(1000);
			// Update game time converting seconds to minutes and seconds
			if (cd->g->game_time == 0) {
				EnterCriticalSection(cs);

				COORD game_over_cord;
				game_over_cord.X = 30;
				game_over_cord.Y = 10;
				set_cursor_pos(game_over_cord);

				_tprintf(TEXT("Game Over!"));
				cd->g->game_time = DEFAULT_GAME_TIME;
				LeaveCriticalSection(cs);
				set_cursor_pos(command_pos);
				break;
			}
			cd->g->game_time = cd->g->game_time - 1;
			previousPos = cd->car[i].position_y;
			
			// Check what's the lane direction and move the car accordingly
			// TODO: Review this logic, it's not working properly
			if (l[i].direction == 0) {
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
			else if (l[i].direction == 1) {
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
	// Function that processes the commands entered by the user in the server console
	
	ControlData *cd = (ControlData*)lparam;
	CRITICAL_SECTION* cs = &cd->cs;
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
		EnterCriticalSection(cs);
		
		COORD print_cord;
		print_cord.X = 0;
		print_cord.Y = 8;
		set_cursor_pos(print_cord);
		fflush(stdin);
		_tprintf(TEXT("[Server.c/server_manager] Enter a command: "));

		COORD current_pos;
		current_pos.X = 43;
		current_pos.Y = 8;

		COORD title_pos;
		title_pos.X = 0;
		title_pos.Y = 15;

		set_cursor_pos(title_pos);

		_tprintf(TEXT("\n[Server.c/operator_command_receiver] Operator Log:"));

		set_cursor_pos(current_pos);

		LeaveCriticalSection(cs);
		_fgetts(command, 100, stdin);
		command[_tcslen(command) - 1] = '\0';
		_tcscpy_s(command_buffer, 100, command);

		EnterCriticalSection(cs);

		if (_tcscmp(command, TEXT("")) == 0) {
			continue;
		}

		ZeroMemory(buffer_item.command, 100 * sizeof(TCHAR));

		if (_tcscmp(command, TEXT("help")) == 0) {
			clear_line(print_cord);
			_tprintf(TEXT("\n\n\t[Server.c/server_manager] Available commands:\n"));
			_tprintf(TEXT("\t\t[Server.c/server_manager] help - list of available commands\n"));
			_tprintf(TEXT("\t\t[Server.c/server_manager] clear - clear the screen\n"));
			_tprintf(TEXT("\t\t[Server.c/server_manager] restart - restart the game\n"));
			_tprintf(TEXT("\t\t[Server.c/server_manager] exit - exit the game\n"));
		}
		else if (_tcscmp(command, TEXT("exit")) == 0) {
			_tprintf(TEXT("\t[!] Server shutting down...\n"));
			_tprintf(TEXT("\t[~] Exiting...\n"));
			// Send he exit command to the client
			_tcscpy_s(buffer_item.command, 100, TEXT("exit"));
			buffer_item.pid = GetCurrentProcessId();
			SetEvent(cd->closingEvent);
			cd->threadStop = 1;
			Sleep(1000);
			exit(0);
		}
		else if (_tcscmp(command, TEXT("clear")) == 0) {
			clear_screen();
			ascii_printer();
		}
		else if (_tcscmp(command, TEXT("restart")) == 0) {
			clear_line(current_pos);
			boardInitializer(cd);
			SetEvent(cd->restartEvent);
			SetEvent(cd->eventHandle);
			_tprintf(TEXT("\n\n\t[Server.c/server_manager] Game restarted!\n"));
			Sleep(1000);
			clear_line(current_pos);
		}
		else {
			clear_line(current_pos);
			_tprintf(TEXT("\tUnknown command.\n"));
			Sleep(1000);
			clear_screen();
			ascii_printer();
		}

		set_cursor_pos(current_pos);

		LeaveCriticalSection(cs);
	}
}

TCHAR** split_command(TCHAR* command) {
	// Function that splits the command into arguments
	TCHAR** args = (TCHAR**)malloc(50 * sizeof(TCHAR*));
	TCHAR* next_token;
	TCHAR* token;
	int i = 0;
	token = _tcstok_s(command, TEXT(" "), &next_token);
	while (token != NULL) {
		args[i] = (TCHAR*)malloc(50 * sizeof(TCHAR));
		_tcscpy_s(args[i], 50, token);
		token = _tcstok_s(NULL, TEXT(" "), &next_token);
		i++;
	}
	args[i] = NULL;
	return args;
}

DWORD WINAPI operator_command_receiver(LPVOID lparam) {
	// Function responsible for receiving the commands from the operator and applying them to the game

	ControlData* cd = (ControlData*)lparam;
	CRITICAL_SECTION* cs = &cd->cs;
	TCHAR command[50][50];
	BufferItem buffer_item;
	TCHAR** args;

	COORD print_cord;
	print_cord.X = 1;
	print_cord.Y = 14;

	while (!cd->threadStop) {
		WaitForSingleObject(cd->hSemRead, INFINITE);
		WaitForSingleObject(cd->hMutex, INFINITE);
		ZeroMemory(&buffer_item, sizeof(BufferItem));
		CopyMemory(&buffer_item, &cd->g->buffer[cd->g->out], sizeof(BufferItem));
		if (buffer_item.pid != GetCurrentProcessId()) {
			SYSTEMTIME systemTime;

			// Get the current local time
			GetLocalTime(&systemTime);

			int year = systemTime.wYear;
			int month = systemTime.wMonth;
			int day = systemTime.wDay;
			int hour = systemTime.wHour;
			int minute = systemTime.wMinute;
			int second = systemTime.wSecond;

			// Print the pid of the process that sent the command
			EnterCriticalSection(cs);
			COORD current_pos = get_cursor_pos();
			
			print_cord.Y += 2;

			if (print_cord.Y > 25) {
				print_cord.Y = 16;
			}

			set_cursor_pos(print_cord);

			wprintf(L"\n\t[%04d-%02d-%02d || %02d:%02d:%02d] Operator PID: %d\n", year, month, day, hour, minute, second, buffer_item.pid);
			cd->g->out++;

			if (cd->g->out == MAX_BUFFER_SIZE) cd->g->out = 0;

			ReleaseMutex(cd->hMutex);
			ReleaseSemaphore(cd->hSemWrite, 1, NULL);

			wprintf(L"\t[%04d-%02d-%02d || %02d:%02d:%02d] Command received: %s\n", year, month, day, hour, minute, second, buffer_item.command);

			// Split the command into arguments
			args = split_command(buffer_item.command);

			set_cursor_pos(current_pos);
			LeaveCriticalSection(cs);

			if (_tcscmp(args[0], _T("stop")) == 0) {
				DWORD SuspendThread(HANDLE runCar);
				Sleep(_wtoi(args[1])*1000);
				DWORD ResumeThread(HANDLE runCar);
			}
			else if(_tcscmp(args[0], _T("obstacle")) == 0) {
				TCHAR y = args[1];
				TCHAR x = args[2];
				y = _wtoi(args[1]);
				x = _wtoi(args[2]);

				cd->g->board[y][x] = '@';
				SetEvent(cd->eventHandle);
				ResetEvent(cd->eventHandle);
			}
			else if (_tcscmp(args[0], _T("invert")) == 0) {
				//cd->g->invert = !cd->g->invert;
				cd->g->l[_wtoi(args[1])].direction = 1;
			}
		}
		else {
			ReleaseMutex(cd->hMutex);
			ReleaseSemaphore(cd->hSemWrite, 1, NULL);

			WaitForSingleObject(cd->hSemWrite, INFINITE);
			WaitForSingleObject(cd->hMutex, INFINITE);
			ZeroMemory(&cd->g->buffer[cd->g->in], sizeof(BufferItem));
			CopyMemory(&cd->g->buffer[cd->g->in], &buffer_item, sizeof(BufferItem));
			cd->g->in++;
			if (cd->g->in == MAX_BUFFER_SIZE) cd->g->in = 0;
			ReleaseMutex(cd->hMutex);
			ReleaseSemaphore(cd->hSemRead, 1, NULL);
		}
	}
	return 0;
}

DWORD WINAPI connectFrogs(LPVOID param) {
	// Function responsible for connecting the frogs to the game

	ThreadDados* pData = (Data*)param;
	TCHAR buff[BUF_TAM];
	DWORD n;

	for (int i = 0; i < MAX_FROGS; i++) {

		WaitForSingleObject(pData->hMutex, INFINITE);
		if (pData->hPipes[i].ativo) {
			if (!WriteFile(pData->hPipes[i].hInstancia, buff, (DWORD)_tcslen(buff) * sizeof(TCHAR), &n, NULL)) {
				_tprintf(TEXT("[ERROR] Write on pipe! (WriteFile)\n"));
			}
			else {
				BOOL ret = ReadFile(pData->hPipes[i].hInstancia, buff, sizeof(buff), &n, NULL);
			}
		}
		ReleaseMutex(pData->hMutex);
	}
	pData->terminar = TRUE;
	_tprintf(TEXT("(DisconnectNamedPipe)\n"));

	for (int i = 0; i < MAX_FROGS; i++)
		SetEvent(pData->hEvents[i]);

	return 0;
}

int _tmain(int argc, TCHAR* argv[]) {

	ThreadDados dados;

	HANDLE hEventTemp;
	HANDLE hPipe;
	HANDLE hThread;

	DWORD numFrogs = 0;
	DWORD i = 0;
	DWORD offset = 0;
	DWORD nbytes = 0;

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
			_tprintf(TEXT("[Server.c/_tmain] No values stored in Registry!\n"));
			// Ask for number of lanes and initial speed to the user

			if (argc == 3) {
				nr_of_lanes = _ttoi(argv[1]);
				init_speed = _ttoi(argv[2]);
				// Print values
				_tprintf(TEXT("[Server.c/_tmain] Number of lanes: %d\n"), nr_of_lanes);
				_tprintf(TEXT("[Server.c/_tmain] Initial speed: %d\n"), init_speed);
				BOOL result = createRegistry(nr_of_lanes, init_speed);
				if (result == -1) {
					_tprintf(TEXT("\t[Server.c/_tmain] Error creating registry file!\n"));
					return -1;
				}
			}
			else
			{
				_tprintf(TEXT("[Server.c/_tmain] Invalid number of arguments!\n"));
				return -1;
			}
		}
		else
		{
			_tprintf(TEXT("[Server.c/_tmain] Values stored in Registry!\n"));
			// Read values from registry
			BOOL result = readRegistry(KEY_ROAD_LANES);
			if (result == -1) {
				_tprintf(TEXT("\t[Server.c/_tmain] Error reading registry file!\n"));
				return -1;
			}
			else {
				nr_of_lanes = result;
			}

			result = readRegistry(KEY_INIT_SPEED);
			if (result == -1) {
				_tprintf(TEXT("\t[Server.c/_tmain] Error reading registry file!\n"));
				return -1;
			}
			else {
				init_speed = result;
			}
			// Print values
			_tprintf(TEXT("\t[Server.c/_tmain] Number of lanes: %d\n"), nr_of_lanes);
			_tprintf(TEXT("\t[Server.c/_tmain] Initial speed: %d\n"), init_speed);
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
			_tprintf(TEXT("[Server.c/_tmain] Error creating shared memory\n"));
			return 1;
		}

		_tprintf(TEXT("[Server.c/_tmain] Shared memory created successfully\n"));

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
			_tprintf(TEXT("[Server.c/_tmain] Error creating shared memory pointer\n"));
			return 1;
		}
		_tprintf(TEXT("[Server.c/_tmain] Shared memory pointer created successfully\n"));

		// Initialize shared memory

		cd.eventHandle = CreateEvent(NULL, TRUE, FALSE, TEXT("BoardEvent"));
		if (cd.eventHandle == NULL) {
			_tprintf(TEXT("[Server.c/_tmain] Error creating event\n"));
			return -1;
		}
		cd.hMutex = CreateMutex(NULL, FALSE, TEXT("SemMutex"));
		if (cd.hMutex == NULL) {
			_tprintf(TEXT("[Server.c/_tmain] Error creating mutex\n"));
			return -1;
		}
		cd.hSemRead = CreateSemaphore(NULL, 0, MAX_BUFFER_SIZE, TEXT("SemRead"));
		if (cd.hSemRead == NULL) {
			_tprintf(TEXT("[Server.c/_tmain] Error creating semaphore\n"));
			return -1;
		}
		cd.hSemWrite = CreateSemaphore(NULL, MAX_BUFFER_SIZE, MAX_BUFFER_SIZE, TEXT("SemWrite"));
		if (cd.hSemWrite == NULL) {
			_tprintf(TEXT("[Server.c/_tmain] Error creating semaphore\n"));
			return -1;
		}
		cd.closingEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("ClosingEvent"));
		if (cd.closingEvent == NULL) {
			_tprintf(TEXT("[Server.c/_tmain] Error creating event\n"));
			return -1;
		}
		cd.restartEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("RestartEvent"));
		if (cd.restartEvent == NULL) {
			_tprintf(TEXT("[Server.c/_tmain] Error creating event\n"));
			return -1;
		}

		cd.g->number_of_lanes = nr_of_lanes;
		cd.g->initial_speed = init_speed;
		cd.g->in = 0;
		cd.g->out = 0;
		cd.g->game_time = 100;
		cd.threadStop = 0;
		InitializeCriticalSectionAndSpinCount(&cd.cs, 200);

		//Comunicaçao com os sapos
		dados.terminar = FALSE;
		dados.hMutex = CreateMutex(NULL, FALSE, NULL);
		if (dados.hMutex == NULL) {
			_tprintf("[Server.c/_tmain] Error creating mutex\n");
			return (-1);
		}

		for (int i = 0; i < MAX_FROGS; i++) {
			ZeroMemory(&dados.hPipes[i].overlap, sizeof(dados.hPipes[i].overlap));      //Limpar este pedaço de memoria
			dados.hEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
			if (dados.hEvents[i] == NULL) {
				_tprintf(_T("[Server.c/_tmain] Error creating event\n"));
				exit(-1);
			}
			dados.hPipes[i].hInstancia = CreateNamedPipe(NOME_PIPE, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, MAX_FROGS, 256 * sizeof(TCHAR), sizeof(TCHAR) * 256, 1000, NULL);
			if (dados.hPipes[i].hInstancia == INVALID_HANDLE_VALUE) {
				_tprintf(_T("[Server.c/_tmain] Error creating named pipe\n"));
				exit(-1);
			}
			dados.hPipes[i].ativo = FALSE;
			dados.hPipes[i].overlap.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			if (dados.hPipes[i].overlap.hEvent == NULL) {
				_tprintf(_T("[Server.c/_tmain] Error creating event\n"));
				exit(-1);
			}

			if (ConnectNamedPipe(dados.hPipes[i].hInstancia, &dados.hPipes[i].overlap)) {
				_tprintf(TEXT("[Server.c/_tmain] Connection successful! (ConnectNamedPipe)\n"));
				exit(-1);
			}
			_tprintf(_T("[Server.c/_tmain] Instance succefully created\n"));
		}

		hThread = CreateThread(NULL, 0, connectFrogs, &dados, 0, NULL);
		if (hThread == NULL) {
			_tprintf(_T("[Server.c/_tmain] Error creating thread\n"));
			return (-1);
		}

		while (!dados.terminar && numFrogs < MAX_FROGS) {

			_tprintf(_T("[Server.c/_tmain] Waiting for the connection to the client\n"));

			offset = WaitForMultipleObjects(MAX_FROGS, dados.hEvents, FALSE, INFINITE);
			i = offset - WAIT_OBJECT_0;

			if (i >= 0 && i < MAX_FROGS) {
				_tprintf(_T("[Server.c/_tmain] Client %d connected\n"), i);
				if (GetOverlappedResult(dados.hPipes[i].hInstancia, &dados.hPipes[i].overlap, &nbytes, FALSE)) {
					ResetEvent(dados.hEvents[i]);

					WaitForSingleObject(dados.hMutex, INFINITE);
					dados.hPipes[i].ativo = TRUE;
					ReleaseMutex(dados.hMutex);
				}
				numFrogs++;
			}

		}

		for (i = 0; i < MAX_FROGS; i++) {
			_tprintf(_T("\n[Server.c/_tmain] Shutting down the the pipe.\n"));
			if (!DisconnectNamedPipe(dados.hPipes[i].hInstancia)) {
				_tprintf(_T("\n[Server.c/_tmain] Error shutting down the pipe (DisconnectNamedPipe) %d.\n"), GetLastError());
				exit(-1);
			}
			CloseHandle(dados.hPipes[i].hInstancia);
		}

		_tprintf(_T("[Server.c/_tmain] Exiting the server\n"));

		WaitForSingleObject(hThread, INFINITE);


		for (int i = 0; i < MAX_FROGS; i++) {
			DisconnectNamedPipe(dados.hPipes[i].hInstancia);
			CloseHandle(dados.hEvents[i]);
			CloseHandle(dados.hPipes[i].hInstancia);
			_tprintf(_T("[Server.c/_tmain] Instance closed\n"));
		}

		CloseHandle(dados.hMutex);
		CloseHandle(dados.hEvents);

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
			_tprintf(TEXT("[Server.c/_tmain] Error creating server manager thread\n"));
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
			_tprintf(TEXT("[Server.c/_tmain] Error creating server manager thread\n"));
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
			_tprintf(TEXT("[Server.c/_tmain] Error creating server manager thread\n"));
			return;
		}

		/*HANDLE game_verifications_thread = CreateThread(
			NULL,
			0,
			game_verifications,
			&cd,
			0,
			NULL
		);

		if (game_verifications_thread == NULL) {
			_tprintf(TEXT("[Server.c/_tmain] Error creating server manager thread\n"));
			return;
		}*/

		boardInitializer(&cd);
		SetEvent(cd.restartEvent);

		WaitForSingleObject(server_manager_thread, INFINITE);
		WaitForSingleObject(operator_command_receiver_thread, INFINITE);
		WaitForSingleObject(carThread, INFINITE);
		//WaitForSingleObject(game_verifications_thread, INFINITE);

		// Wait for the threads to finish with WaitForSingleObject
		UnmapViewOfFile(cd.g);
		CloseHandle(hMapFile);
		CloseHandle(hSemaphoreServer);
		CloseHandle(cd.eventHandle);
		CloseHandle(cd.hMutex);
		CloseHandle(cd.hSemRead);
		CloseHandle(cd.hSemWrite);
		CloseHandle(cd.closingEvent);

	return 0;
}