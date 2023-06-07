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
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
DWORD WINAPI WaitEventFunc(LPVOID);	// declaração da função  coisa do código exemplo

HANDLE hEventoTeclaEsc, hEventoTeclaQ, hEventoTeclaS, hEventoTeclaX;

// Casting para terceiro e sexto parâmetros da função _beginthreadex
typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);
typedef unsigned* CAST_LPDWORD;


BOOL desbloqueado = true;

int main() { // Receber parametros

    printf("*** tarefas de exibicao iniciadas ***\n");

    // 1. Abre os eventos vindos de outros processos
    hEventoTeclaEsc = OpenEvent(EVENT_ALL_ACCESS, FALSE, "TeclaESC");
    if (hEventoTeclaEsc == 0) { printf("[EXIBICAO] Erro na abertura do evento <Tecla ESC Pressionada>\n"); exit(-1); }
    hEventoTeclaQ = OpenEvent(EVENT_ALL_ACCESS, FALSE, "TeclaQ");
    if (hEventoTeclaQ == 0) { printf("[EXIBICAO] Erro na abertura do evento <Tecla Q Pressionada>\n"); exit(-1); }
    hEventoTeclaS = OpenEvent(EVENT_ALL_ACCESS, FALSE, "TeclaS");
    if (hEventoTeclaS == 0) { printf("[EXIBICAO] Erro na abertura do evento <Tecla S Pressionada>\n"); exit(-1); }
    hEventoTeclaX = OpenEvent(EVENT_ALL_ACCESS, FALSE, "TeclaX");
    if (hEventoTeclaX == 0) { printf("[EXIBICAO] Erro na abertura do evento <Tecla X Pressionada>\n"); exit(-1); }

    HANDLE Eventos[4] = { hEventoTeclaQ, hEventoTeclaS, hEventoTeclaX, hEventoTeclaEsc };
    DWORD retornoSincronizacao;
    int nTipoEvento;

    // 2. Aguarda a sinalizacao de um evento
    do { 
        retornoSincronizacao = WaitForMultipleObjects(4, Eventos, FALSE, INFINITE);
        nTipoEvento = retornoSincronizacao - WAIT_OBJECT_0;

        if (nTipoEvento == 0) {}            // hEventoTeclaQ: exibição de dados de processo
        else if (nTipoEvento == 1) {}       // hEventoTeclaS: exibição de dados de otimização
        else if (nTipoEvento == 2) {}       // hEventoTeclaX: limpar janela de console de dados de otimização

    } while (nTipoEvento != 4);

    // 3. Lista Circular



    // recebe o evento de bloqueio e o esc
    // recebe ids de comunicação com as tarefas de captura
    // criar objetos de sincronização
        // mutex de leitura/escrita da lista circular
        // evento de temporização
    // criar a lista circular
    // gerar mensagem
        // armazenar mensagem na lista
    // wait for multiple, bloqueio ou ESC


    CloseHandle(hEventoTeclaEsc);
    CloseHandle(hEventoTeclaQ);
    CloseHandle(hEventoTeclaS);
    CloseHandle(hEventoTeclaX);

    return 0;
}

