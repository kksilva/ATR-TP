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
#define _CRT_SECURE_NO_DEPRECATE 1

// *** Constantes ***
#define TECLA_ESC 0x1B
#define WIN32_LEAN_AND_MEAN 
#define WNT_LEAN_AND_MEAN
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define TAMANHO_MSG_MAX 43
#define TAMANHO_LISTA_CIRCULAR 200
#define TAMANHO_BYTES_LISTA 43*200

char* listaCircular[TAMANHO_LISTA_CIRCULAR];
HANDLE hEventoTeclaEsc, hEventoTeclaO, hEventoTeclaP;
HANDLE hMensagemDisponivel, hLerMensagem;
DWORD WINAPI WaitEventFunc(LPVOID);	// declaração da função  coisa do código exemplo

// Casting para terceiro e sexto parâmetros da função _beginthreadex
typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);
typedef unsigned* CAST_LPDWORD;

int main() { // Receber parametros
    HANDLE hListaCircular;
    BOOL bStatus;

    // 1. Abre os eventos vindos de outros processos
    hEventoTeclaEsc = OpenEvent(EVENT_ALL_ACCESS, FALSE, "TeclaESC");
    if (hEventoTeclaEsc == 0) { printf("[CAPTURA] Erro na abertura do evento <Tecla ESC Pressionada> Codigo = %d\n", GetLastError()); exit(-1); }
    hEventoTeclaO = OpenEvent(EVENT_ALL_ACCESS, FALSE, "TeclaO");
    if (hEventoTeclaO == 0) { printf("[CAPTURA] Erro na abertura do evento <Tecla O Pressionada> Codigo = %d\n", GetLastError()); exit(-1); }
    hEventoTeclaP = OpenEvent(EVENT_ALL_ACCESS, FALSE, "TeclaP");
    if (hEventoTeclaP == 0) { printf("[CAPTURA] Erro na abertura do evento <Tecla P Pressionada> Codigo = %d\n", GetLastError()); exit(-1); }

    // 2. Abre o mapeamento em memória
    hListaCircular = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,                // Direitos de acesso
        FALSE,                              // O handle não será herdado
        "LISTA_CIRCULAR"                    // Apontador para o nome do objeto
    );
    if (hListaCircular == NULL) { printf("[CAPTURA] Erro na abertura do mapeamento em memoria <hListaCircular> Codigo = %d\n", GetLastError()); exit(-1); }

    // 3. Lista Circular
    DWORD offsetListaCircular = 0;
    for (int i = 0; i < TAMANHO_LISTA_CIRCULAR; i++) {
        listaCircular[i] = (char*)MapViewOfFile(
            hListaCircular,             // Handle para o arquivo mapeado
            FILE_MAP_WRITE,		        // Direitos de acesso: leitura e escrita
            0,                          // Bits mais significativos da "visão"
            (DWORD) 0,                  // Bits menos significativos da "visão"
            0                           // Tamanho da visao, em bytes
        );
        if (listaCircular[i] == NULL) { printf("[CAPTURA] Erro na abertura da visao do mapeamento em memoria <listaCircular> Codigo = %d\n", GetLastError()); exit(-1); }
        offsetListaCircular += TAMANHO_MSG_MAX;
    }

    hMensagemDisponivel = OpenEvent(EVENT_ALL_ACCESS, FALSE, "hMensagemDisponivel");
    if (hMensagemDisponivel == 0) { printf("[CAPTURA] Erro na abertura do evento <Mensagem Disponivel> Codigo = %d\n", GetLastError()); exit(-1); }
    hLerMensagem = OpenEvent(EVENT_ALL_ACCESS, FALSE, "MsgRead");
    if (hMensagemDisponivel == 0) { printf("[CAPTURA] Erro na abertura do evento <Ler Mensagem> Codigo = %d\n", GetLastError()); exit(-1); }

    printf("*** tarefas de captura iniciadas ***\n");
    do {
        for (int i = 0; i < TAMANHO_LISTA_CIRCULAR; i++) {
            // Espera que outro processo escreva mensagem 
            WaitForSingleObject(hMensagemDisponivel, INFINITE);
            ResetEvent(hMensagemDisponivel);
            printf("Mensagem Recebida= %s\n", listaCircular[i]);
            if (strcmp(listaCircular[i], "") == 0) break;

            // Limpa memória compartilhada
            strcpy_s(listaCircular[i], sizeof(listaCircular[i]), "");
            SetEvent(hLerMensagem);	// Avisa ao outro processo para ler mensagem	
        }
    } while (TRUE);

    // Elimina mapeamento
    for (int i = 0; i < TAMANHO_LISTA_CIRCULAR; i++) 
        bStatus = UnmapViewOfFile(listaCircular[i]);

    printf("[CAPTURA] Erro ao cancelar a visao mapeada <bStatus> Codigo = %d\n", GetLastError()); exit(-1);


    // recebe o evento de bloqueio e o esc
    // recebe ids de comunicação com as tarefas de captura
    // criar objetos de sincronização
        // mutex de leitura/escrita da lista circular
        // evento de temporização
    // criar a lista circular
    // gerar mensagem
        // armazenar mensagem na lista
    // wait for multiple, bloqueio ou ESC
    //CheckForError(bStatus);

    CloseHandle(hListaCircular);
    CloseHandle(hEventoTeclaEsc);
    CloseHandle(hEventoTeclaO);
    CloseHandle(hEventoTeclaP);
    CloseHandle(hMensagemDisponivel);
    CloseHandle(hLerMensagem);
    

    return 0;
}

