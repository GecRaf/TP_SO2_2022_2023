#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <windowsx.h>
#include "frog.h"

LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);

TCHAR szProgName[] = TEXT("Frog");

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {

    HWND hWnd;
    MSG lpMsg;
    WNDCLASSEX wcApp;

    // ============================================================================
    // 1. Definição das características da janela "wcApp" 
    //    (Valores dos elementos da estrutura "wcApp" do tipo WNDCLASSEX)
    // ============================================================================
    wcApp.cbSize = sizeof(WNDCLASSEX);   // Tamanho da estrutura WNDCLASSEX
    wcApp.hInstance = hInst; // Instância da janela actualmente exibida 
    wcApp.lpszClassName = szProgName;  // Nome da janela
    wcApp.lpfnWndProc = TrataEventos;  // Endereço da função de processamento da janela
    wcApp.style = CS_HREDRAW | CS_VREDRAW;  // Estilo da janela
    wcApp.hIcon = LoadIcon(NULL, IDI_WARNING); // "hIcon" = handler do ícon normal
    wcApp.hIconSm = LoadIcon(NULL, IDI_SHIELD); // "hIconSm" = handler do ícon pequeno
    wcApp.hCursor = LoadCursor(NULL, IDC_ARROW); // "hCursor" = handler do cursor (rato) 
    wcApp.lpszMenuName = NULL; // Classe do menu que a janela pode ter
    wcApp.cbClsExtra = 0;
    wcApp.cbWndExtra = 0;
    wcApp.hbrBackground = CreateSolidBrush(RGB(255, 255, 255)); //cor de fundo da janela

    // ============================================================================
    // 2. Registar a classe "wcApp" no Windows
    // ============================================================================
    if (!RegisterClassEx(&wcApp))
        return(0);

    // ============================================================================
    // 3. Criar a janela
    // ============================================================================

    hWnd = CreateWindow(
        szProgName, // Nome da janela (programa) definido acima
        TEXT("Frog"), // Texto que figura na barra do título
        WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU, // Estilo da janela (WS_OVERLAPPED= normal)
        CW_USEDEFAULT, // Posição x pixels (default=à direita da última)
        CW_USEDEFAULT, // Posição y pixels (default=abaixo da última)
        1000, // Largura da janela (em pixels)
        800, // Altura da janela (em pixels)
        (HWND)HWND_DESKTOP, // handle da janela pai (se se criar uma a partir de
        // outra) ou HWND_DESKTOP se a janela for a primeira, 
        // criada a partir do "desktop"
        (HMENU)NULL, // handle do menu da janela (se tiver menu)
        (HINSTANCE)hInst, // handle da instância do programa actual ("hInst" é 
        // passado num dos parâmetros de WinMain()
        0); // Não há parâmetros adicionais para a janela


    // ============================================================================
    // 4. Mostrar a janela
    // ============================================================================
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);



    // ============================================================================
    // 5. Loop de Mensagens
    // ============================================================================

    while (GetMessage(&lpMsg, NULL, 0, 0)) {
        TranslateMessage(&lpMsg);
        DispatchMessage(&lpMsg);
    }

    // ============================================================================
    // 6. Fim do programa
    // ============================================================================
    return((int)lpMsg.wParam);
}

LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam, int nCmdShow) {
    HDC hdc;
    static RECT rect;
    static HBITMAP hBmp;
    static HBITMAP hBmpBoard;
    static HBITMAP hBmpPinkCar;
    static HBITMAP hBmpRedCar;
    static BITMAP bmp = { 0 };
    static BITMAP bmpBoard = { 0 };
    static BITMAP bmpPinkCar = { 0 };
    static BITMAP bmpRedCar = { 0 };
    static HDC bmpDC = NULL;
    static HDC bmpBoardDC = NULL;
    static HDC bmpPinkCarDC = NULL;
    static HDC bmpRedCarDC = NULL;
    ControlData* cd = (ControlData*)lParam;
    static Frogs frog = { 0 }; // Estrutura para representar o sapo
    static Lanes lane = { 0 };
    PAINTSTRUCT ps;
    HANDLE hSemServer;

    switch (messg) {

    case WM_CREATE:

        hSemServer = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, SEMAPHORE_SERVER);
        if (hSemServer == NULL) {
            if (MessageBox(hWnd, TEXT("Server is not running. Please start the server and try again."), TEXT("Error!"), MB_ICONSTOP | MB_OK)) {
                DestroyWindow(hWnd);
            }
        }

        hBmp = (HBITMAP)LoadImage(NULL, TEXT("../../Bitmaps/frog1.bmp"), IMAGE_BITMAP, 35, 35, LR_LOADFROMFILE);
        hBmpBoard = (HBITMAP)LoadImage(NULL, TEXT("../../Bitmaps/areaJogo.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        hBmpPinkCar = (HBITMAP)LoadImage(NULL, TEXT("../../Bitmaps/car1.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        hBmpRedCar = (HBITMAP)LoadImage(NULL, TEXT("../../Bitmaps/car1.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

        GetObject(hBmp, sizeof(bmp), &bmp);
        GetObject(hBmpBoard, sizeof(bmpBoard), &bmpBoard);
        GetObject(hBmpPinkCar, sizeof(bmpPinkCar), &bmpPinkCar);
        GetObject(hBmpRedCar, sizeof(bmpRedCar), &bmpRedCar);

        frog.position_x = 0; // Coordenada x centralizada
        frog.position_y = bmpBoard.bmHeight - bmpBoard.bmHeight; // Coordenada y centralizada
        frog.last_position_x = frog.position_x;
        frog.last_position_y = frog.position_y;

        // Defina as coordenadas iniciais do "frog"
        int frogX = 0;
        int frogY = bmpBoard.bmHeight - bmp.bmHeight;

        int carPositionX = 0; // Coordenada x inicial do carro
        int carPositionY = 9;

        int carPositionz = 0; // Coordenada x inicial do carro
        int carPositionh = 9;

        hdc = GetDC(hWnd);
        bmpDC = CreateCompatibleDC(hdc);
        bmpBoardDC = CreateCompatibleDC(hdc);
        bmpPinkCarDC = CreateCompatibleDC(hdc);
        bmpRedCarDC = CreateCompatibleDC(hdc);

        SelectObject(bmpDC, hBmp);
        SelectObject(bmpBoardDC, hBmpBoard);
        SelectObject(bmpPinkCarDC, hBmpPinkCar);
        SelectObject(bmpRedCarDC, hBmpRedCar);

        ReleaseDC(hWnd, hdc);

        GetClientRect(hWnd, &rect);

        break;

    case WM_KEYDOWN:
        // Guardar a posição anterior do sapo
        frog.last_position_x = frog.position_x;
        frog.last_position_y = frog.position_y;

        switch (wParam) {
        case VK_LEFT:
            if (frog.position_x - FROG_SPEED >= (rect.right - rect.left - bmpBoard.bmWidth) / 2) {
                frog.position_x -= FROG_SPEED;
            }
            break;
        case VK_RIGHT:
            if (frog.position_x + bmp.bmWidth + FROG_SPEED <= (rect.right - rect.left + bmpBoard.bmWidth) / 2) {
                frog.position_x += FROG_SPEED;
            }
            break;
        case VK_UP:
            if (frog.position_y - FROG_SPEED >= (rect.bottom - rect.top - bmpBoard.bmHeight) / 2) {
                frog.position_y -= FROG_SPEED;
            }
            break;
        case VK_DOWN:
            if (frog.position_y + bmp.bmHeight + FROG_SPEED <= (rect.bottom - rect.top + bmpBoard.bmHeight) / 2) {
                frog.position_y += FROG_SPEED;
            }
            break;
        }

        // Redesenha a janela para atualizar a posição do sapo
        InvalidateRect(hWnd, NULL, TRUE);
        break;

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        GetClientRect(hWnd, &rect);

        // Obter as dimensões da janela
        int windowWidth = rect.right - rect.left;
        int windowHeight = rect.bottom - rect.top;

        // Obter as dimensões do bitmap
        int bitmapWidth = bmpBoard.bmWidth;
        int bitmapHeight = bmpBoard.bmHeight;


        // Calcular as coordenadas para centralizar o bitmap
        int x = (windowWidth - bitmapWidth) / 2;
        int y = (windowHeight - bitmapHeight) / 2;

        //calcular as coordenadas para posicionar o "frog" na meta
        frogX = 0;
        frogY = bitmapHeight - bmp.bmHeight;

        //posicões dos carros
        carPositionX = (bmpBoard.bmWidth - bmpPinkCar.bmWidth) / 2; // Centralizado na largura
        carPositionY = (bmpBoard.bmWidth - bmpRedCar.bmWidth) / 2 - 150;

        carPositionz = (bmpBoard.bmWidth - bmpRedCar.bmWidth) / 2; // Centralizado na largura
        carPositionh = (bmpBoard.bmWidth - bmpRedCar.bmWidth) / 2 - 50;

        BitBlt(hdc, x, y, bitmapWidth, bitmapHeight, bmpBoardDC, 0, 0, SRCCOPY);
        // Desenhar o sapo na posição atual
        BitBlt(hdc, frog.position_x, frog.position_y, bmp.bmWidth, bmp.bmHeight, bmpDC, 0, 0, SRCCOPY);


        // Definir as coordenadas e desenhar os carros
        int carSpacing = 100; // Espaçamento entre os carros
        int carPositionR = (bmpBoard.bmHeight - bmpPinkCar.bmHeight) / 2 - 150; // Posição y inicial dos carros

        for (int row = 0; row < 2; row++) {

            int carPositionX = (bmpBoard.bmWidth - bmpPinkCar.bmWidth - carSpacing * 4) / 2; // Posição x inicial dos carros

            for (int carIndex = 0; carIndex < 4; carIndex++) {
                // Desenhar carro rosa
                BitBlt(hdc, carPositionX, carPositionY, bmpPinkCar.bmWidth, bmpPinkCar.bmHeight, bmpPinkCarDC, 0, 0, SRCCOPY);

                // Desenhar carro vermelho
                BitBlt(hdc, carPositionX, carPositionY + 50, bmpRedCar.bmWidth, bmpRedCar.bmHeight, bmpRedCarDC, 0, 0, SRCCOPY);

                // Atualizar a posição x para o próximo carro
                carPositionX += bmpRedCar.bmWidth + carSpacing;
            }

            // Atualizar a posição y para a próxima linha de carros
            carPositionY += bmpRedCar.bmHeight * 2 + carSpacing;
            // Redefinir a posição x para a primeira coluna de carros
            carPositionX = (bmpBoard.bmWidth - bmpPinkCar.bmWidth - carSpacing * 3) / 2;
        }


        // Atualizar as coordenadas dos carros
        carPositionX += lane.car_speed;
        if (carPositionX >= bmpBoard.bmWidth) {
            carPositionX = bmpPinkCar.bmWidth - carSpacing;
        }

        EndPaint(hWnd, &ps);

        break;
    case WM_CLOSE:
        if (MessageBox(hWnd, TEXT("Tem a certeza que quer sair?"), TEXT("Confirmação"), MB_ICONQUESTION | MB_YESNO) == IDYES) {
            DestroyWindow(hWnd);
        }
        break;

    case WM_DESTROY:// Destruir a janela e terminar o programa 
        // "PostQuitMessage(Exit Status)"
        DeleteDC(bmpDC);

        DeleteObject(hBmp);

        PostQuitMessage(0);
        break;
    default:
        // Neste exemplo, para qualquer outra mensagem (p.e. "minimizar","maximizar","restaurar")
        // não é efectuado nenhum processamento, apenas se segue o "default" do Windows
        return(DefWindowProc(hWnd, messg, wParam, lParam));
        break;  // break tecnicamente desnecessário por causa do return
    }
    return(0);
}



int _tmain(int argc, LPTSTR argv[]) {
    TCHAR buf[256];
    HANDLE hPipe;
    int i = 0;
    BOOL ret;
    DWORD n;
    ControlData cd;
    ThreadDados pData;

#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif

    cd.Td = &pData;
    for (int i = 0; i < MAX_FROGS; i++) {
        cd.Td->hEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (cd.Td->hEvents[i] == NULL) {
            _tprintf(_T("[Frog.c/_tmain] Error creating event\n"));
            exit(-1);
        }
        SetEvent(cd.Td->hEvents[i]);
    }

    _tprintf(TEXT("Wait for pipe '%s' (WaitNamedPipe)\n"),
        NOME_PIPE);
    if (!WaitNamedPipe(NOME_PIPE, NMPWAIT_WAIT_FOREVER)) {
        _tprintf(TEXT("[ERROR] Connect to pipe '%s'! (WaitNamedPipe)\n"), NOME_PIPE);
        exit(-1);
    }
    _tprintf(TEXT("Binding to the writer pipe... (CreateFile)\n"));
    hPipe = CreateFile(NOME_PIPE, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hPipe == NULL) {
        _tprintf(TEXT("[ERRO]R Connect to pipe '%s'! (CreateFile)\n"), NOME_PIPE);
        exit(-1);
    }
    _tprintf(TEXT("CONNECTED...\n"));

    while (1) {
        ret = ReadFile(hPipe, buf, sizeof(buf), &n, NULL);
        buf[n / sizeof(TCHAR)] = '\0';
        if (!ret || !n) {
            _tprintf(TEXT(" %d %d... (ReadFile)\n"), ret, n);
            break;
        }

        CharUpperBuff(buf, _tcslen(buf));


        if (!WriteFile(hPipe, buf, (DWORD)_tcslen(buf) * sizeof(TCHAR), &n, NULL)) {
            _tprintf(TEXT("[ERROR] Write on pipe! (WriteFile)\n"));
            exit(-1);
        }

    }
    CloseHandle(hPipe);
    Sleep(200);
    return 0;
}
