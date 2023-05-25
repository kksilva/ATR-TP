#include <iostream>

// *** Bibliotecas ***
// Rever as bibliotecas 
#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <errno.h>
#include <signal.h>
#include <conio.h>		                                                        
#include <math.h>                                                               
#include <time.h>
#include <string.h>
#include <locale.h>
//#include <pthread.h>
#include <string>
//#define _CHECKERROR	1
//#include "CheckForError.h"   

// *** Constantes ***
#define TECLA_ESC 0x1B
#define WIN32_LEAN_AND_MEAN 
#define WNT_LEAN_AND_MEAN
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define TAM_MSG 42

HANDLE hEventoTeclaEsc, hEventoTecla;
HANDLE hMensagemDisponivel, hLerMensagem;
DWORD WINAPI WaitEventFunc(LPVOID);	// declara��o da fun��o  coisa do c�digo exemplo

// Casting para terceiro e sexto par�metros da fun��o _beginthreadex
typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);
typedef unsigned* CAST_LPDWORD;

int main() { // Receber parametros
    HANDLE hListaCircular;
    BOOL bStatus;
    char* lpImage;

    hListaCircular = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,               // Direitos de acesso
        FALSE,                              // O handle n�o ser� herdado
        "LISTA_CIRCULAR"                    // Apontador para o nome do objeto
    );
    //CheckForError(hListaCircular);

    lpImage = (char*)MapViewOfFile(
        hListaCircular,             //Handle para o arquivo mapeado
        FILE_MAP_WRITE,		        // Direitos de acesso: leitura e escrita
        0,                          // Bits mais significativos da "vis�o"
        0,                          // Bits menos significativos da "vis�o"
        TAM_MSG
    );
    //CheckForError(lpImage);

    hEventoTeclaEsc = OpenEvent(EVENT_ALL_ACCESS, FALSE, "TeclaESC");
    if (hEventoTeclaEsc == 0) { printf("[CAPTURA] Erro na abertura do evento <Tecla ESC Pressionada>\n"); exit(-1); }
    
    //abrir outro evento do teclado
    hEventoTecla = OpenEvent(EVENT_ALL_ACCESS, FALSE, "TeclaD");

    hMensagemDisponivel = OpenEvent(EVENT_ALL_ACCESS, FALSE, "hMensagemDisponivel");
    if (hMensagemDisponivel == 0) { printf("[CAPTURA] Erro na abertura do evento <Mensagem Disponivel>\n"); exit(-1); }
    hLerMensagem = OpenEvent(EVENT_ALL_ACCESS, FALSE, "MsgRead");
    if (hMensagemDisponivel == 0) { printf("[CAPTURA] Erro na abertura do evento <Ler Mensagem>\n"); exit(-1); }

    do {
        // Espera que outro processo escreva mensagem 
        WaitForSingleObject(hMensagemDisponivel, INFINITE);
        ResetEvent(hMensagemDisponivel);
        printf("Mensagem Recebida= %s\n", lpImage);
        if (strcmp(lpImage, "") == 0) break;

        // Limpa mem�ria compartilhada
        strcpy(lpImage, "");
        SetEvent(hLerMensagem);	// Avisa ao outro processo para ler mensagem	

    } while (TRUE);

    // Elimina mapeamento
    bStatus = UnmapViewOfFile(lpImage);
    //CheckForError(bStatus);

    printf("*** tarefas de captura iniciadas ***\n");

    // recebe o evento de bloqueio e o esc
    // recebe ids de comunica��o com as tarefas de captura
    // criar objetos de sincroniza��o
        // mutex de leitura/escrita da lista circular
        // evento de temporiza��o
    // criar a lista circular
    // gerar mensagem
        // armazenar mensagem na lista
    // wait for multiple, bloqueio ou ESC

    bStatus = UnmapViewOfFile(lpImage);
    //CheckForError(bStatus);
    CloseHandle(hListaCircular);

    return 0;
}

