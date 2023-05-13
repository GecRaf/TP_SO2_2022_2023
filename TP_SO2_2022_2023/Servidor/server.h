#ifndef SERVER_H

#include <windows.h>
#include <winbase.h>
#include <winerror.h>
#include <winreg.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#define MAX_SERVER 1
#define MAX_FROGS 2
#define SEMAPHORE_SERVER TEXT("SERVER_INSTANCE")

#define MAX_ROAD_LANES 8
#define MAX_CARS_PER_LANE 8
#define MAX_CAR_SPEED 10
#define MAX_BOARD_COL 20
#define MAX_BOARD_ROW 10
#define MAX_GAME_TIME 10000
#define DEFAULT_GAME_TIME 60
#define MAX_BUFFER_SIZE 10

#define KEY_PATH TEXT("Software\\TP_SO2_2223\\")
#define KEY_ROAD_LANES TEXT("RoadLanes")
#define KEY_INIT_SPEED TEXT("InitialSpeed")
#define KEY_SHARED_MEMORY TEXT("SharedMemory")

// Structs

typedef struct {
	TCHAR command[100];
} BufferItem;

typedef struct {
	int direction;
	int number_of_cars;
	int car_speed;
	int car_distance;
} Lanes;

typedef struct {
	int id;
	int score;
	int position_x;
	int position_y;
} Frogs;

typedef struct {
	TCHAR board[MAX_BOARD_ROW][MAX_BOARD_COL];
	int game_time;
	int number_of_frogs;
	int number_of_lanes;
	int initial_speed;
	Lanes l[MAX_ROAD_LANES];
	Frogs f[MAX_FROGS];
	
	BufferItem buffer[MAX_BUFFER_SIZE];
	HANDLE mutexSemaphore;
	HANDLE emptySemaphore;
	HANDLE fullSemaphore;
	DWORD in, out;
} Game;

typedef struct {
	int position_x;
	int position_z;
}Cars;

typedef struct {
	Game* g;
	LPCTSTR shared_memmory_ptr;
	HANDLE hSemWrite;
	HANDLE hSemRead;
	HANDLE hMutex;
	DWORD threadStop;
	HANDLE eventHandle;
	Frogs* f1;
	Frogs* f2;
	Cars* car;
} ControlData;

// Functions
int verifyRegistry();
int createRegistry(int roadLanes, int carSpeed);
int readRegistry(char type[]);

#endif // !SERVER_H
