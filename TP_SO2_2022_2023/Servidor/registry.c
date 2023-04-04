#include <windows.h>
#include <winreg.h>
#include <tchar.h>
#include <fcntl.h>
#include <synchapi.h>

#include "server.h"

#define TAM 200


BOOL serverRegistry(DWORD*, DWORD*) {

    HKEY chave;
    TCHAR par_nome[TAM] = TEXT("ROAD_LANES");
    DWORD par_valor = MAX_ROAD_LANES;
    TCHAR chave_nome[TAM] = TEXT("SOFTWARE\\TP_SO2_2122");
    TCHAR par_nome_carspeed[TAM] = TEXT("CARSPEED");
    DWORD par_valor_carspeed = 5;
    DWORD resultado;

#ifdef UNICODE 
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif


    //Tentar abrir a chave
    if (RegOpenKeyEx(HKEY_CURRENT_USER, chave_nome, 0, KEY_ALL_ACCESS, &chave) != ERROR_SUCCESS) {

        //Erro ao abrir a chave
        _tprintf(TEXT("[Registry.c/serverRegistry] A chave ainda não foi criada\n"));

        //tentar criar a chave
        if (RegCreateKeyEx(HKEY_CURRENT_USER, chave_nome, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &chave, &resultado) != ERROR_SUCCESS) {
            //Não foi possível criar chave, mostrar o erro
            DWORD error = GetLastError();
            _tprintf(TEXT("[Registry.c/serverRegistry] Erro ao criar a chave: [%d]\n"), error);
            return FALSE;

        }
        else {
            if (resultado == REG_CREATED_NEW_KEY) {
                _tprintf(TEXT("[Registry.c/serverRegistry] Chave criada\n"));

                //criar registo par chave-valor 


            }
        }

    }
}