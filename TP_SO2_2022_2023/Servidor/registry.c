#include <windows.h>
#include <winreg.h>
#include <tchar.h>
#include <fcntl.h>
#include <synchapi.h>

#include "server.h"

#define TAM 200


BOOL RegistryVerify(DWORD* roadLanes) {

    HKEY chave;
    TCHAR par_nome[TAM] = TEXT("ROAD_LANES");
    DWORD par_valor = MAX_ROAD_LANES;
    TCHAR chave_nome[TAM] = TEXT("Software\\TP_SO2_2122");
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
        return FALSE;
    }
    else {
        _tprintf(TEXT("[Registry.c/serverRegistry]Chave aberta\n"));

        //ler valor da chave
        _tprintf(_T("[Registry.c/serverRegistry] A obter valor do registry...\n"));
        DWORD valor = 0;
        DWORD length = sizeof(DWORD);

        if (RegQueryValueEx(chave, par_nome, NULL, NULL, (LPBYTE)(&valor), &length) != ERROR_SUCCESS) {
            _tprintf(_T("[Registry.c/serverRegistry] Não foi possível obter o valor a partir da chave\n"));

            RegistryCreate(par_valor, par_nome_carspeed);
            
        }
        else {
            _tprintf(_T("[Registry.c/serverRegistry] Valor obtido do registry: %ld\n"), valor);

            if (valor < 0 || valor > 8) {
                _tprintf(_T("[Registry.c/serverRegistry] O Valor obtido do registry é invalido. Será aplicado o valor default de 8\n"));
                valor = 8;
            }

            *roadLanes = valor;

            if (RegQueryValueEx(chave, par_nome_carspeed, NULL, NULL, (LPBYTE)(&valor), &length) != ERROR_SUCCESS) {

                _tprintf(_T("[Registry.c/serverRegistry] Não foi possível obter o valor a partir da chave\n"));

                RegistryCreate(par_valor, par_nome_carspeed);

            }

        }

        RegCloseKey(chave);
        return TRUE;
    }
}


BOOL RegistryCreate(DWORD* roadLanes, DWORD* carspeed) {
    HKEY chave;
    TCHAR par_nome[TAM] = TEXT("ROAD_LANES");
    DWORD par_valor = MAX_ROAD_LANES;
    TCHAR chave_nome[TAM] = TEXT("Software\\TP_SO2_2223\\");
    TCHAR par_nome_carspeed[TAM] = TEXT("CARSPEED");
    DWORD par_valor_carspeed = 5;
    DWORD resultado;

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

            //criar registo par chave-valor road lanes
            if (RegSetValue(chave, par_nome, 0, &par_valor, sizeof(par_valor)) == ERROR_SUCCESS) {
                _tprintf(_T("[Registry.c/serverRegistry] Par criado: chave %s | valor %i\n"), par_nome, par_valor);
                *roadLanes = par_valor;
            }
            else {
                _tprintf(TEXT("[Registry.c/serverRegistry] Houve um erro ao criar o par chave-valor\n"));
                DWORD error = GetLastError();
                _tprintf(TEXT("Error [%d]\n"), error);
                return FALSE;
            }

            //criar registo par chave-valor car speed
            if (RegSetValue(chave, par_nome_carspeed, 0, REG_DWORD, &par_valor_carspeed, sizeof(par_valor_carspeed)) == ERROR_SUCCESS) {
                _tprintf(_T("[Registry.c/serverRegistry] Par criado: chave %s | valor %i\n"), par_nome_carspeed, par_valor_carspeed);
                *carspeed = par_valor_carspeed;
            }
            else {
                _tprintf(TEXT("[Registry.c/serverRegistry] Houve um erro ao criar o par chave-valor\n"));
                DWORD error = GetLastError();
                _tprintf(TEXT("Error [%d]\n"), error);
                return FALSE;
            }
        }
        else { //impossível criar a chave
            _tprintf(_T("[Registry.c/serverRegistry] Não foi possível criar a chave\n"));
            DWORD error = GetLastError();
            _tprintf(TEXT("Error [%d]\n"), error);
            return FALSE;
        }
        return TRUE;
    }

}
