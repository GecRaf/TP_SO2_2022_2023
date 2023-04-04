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
#define MAX_FROG 2
#define SEMAPHORE_SERVER TEXT("SERVER_INSTANCE")

#define MAX_ROAD_LANES 8
#define MAX_CARS_PER_LANE 8
#define NAX_BOARD_COL 20
#define MAX_BOARD_ROW 10
#define MAX_GAME_TIME 10000
#define DEFAULT_GAME_TIME 60

#define KEY_PATH TEXT("")
#define KEY_ROAD_LANES TEXT("RoadLanes")
#define KEY_INIT_SPEED TEXT("InitialSpeed")


// Structs

// Functions


#endif // !SERVER_H
