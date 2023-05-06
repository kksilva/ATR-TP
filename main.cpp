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
#define _CHECKERROR	1
#include "CheckForError.h"   

// *** Constantes ***
#define TECLA_ESC 0x1B
#define WIN32_LEAN_AND_MEAN 
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
DWORD WINAPI WaitEventFunc(LPVOID);	// declaração da função  coisa do código exemplo

// Casting para terceiro e sexto parâmetros da função _beginthreadex
typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);
typedef unsigned* CAST_LPDWORD;

// *** Funções ***
void EsperaTeclado();

int main(){
    
    HANDLE hEventoTeclaEsc, hEventoTeclaD, hEventoTeclaO, hEventoTeclaP, hEventoTeclaQ, hEventoTeclaS, hEventoTeclaX;


    hEventoTeclaEsc = CreateEvent(NULL, TRUE, FALSE, L"TeclaESC");
    //CheckForError(hEventoTeclaEsc);         
    hEventoTeclaD = CreateEvent(NULL, FALSE, FALSE, L"TeclaD");
    hEventoTeclaO = CreateEvent(NULL, FALSE, FALSE, L"TeclaO");
    hEventoTeclaP = CreateEvent(NULL, FALSE, FALSE, L"TeclaP");
    hEventoTeclaQ = CreateEvent(NULL, FALSE, FALSE, L"TeclaQ");
    hEventoTeclaS = CreateEvent(NULL, FALSE, FALSE, L"TeclaS");
    hEventoTeclaX = CreateEvent(NULL, FALSE, FALSE, L"TeclaX");

    // Cria o proceso de leitura do CLP

    // Cria o processo de captura de mensagens

    // Cria o processo de exibição de mensagens


    EsperaTeclado();

    // Destruir handles dos eventos tecla
    // Destruir handles dos processos
    // Destruir outros objetos de sincronização

    return 0;
}

void EsperaTeclado() {

    char tecla; // global ou local?

    do {
        tecla = _getch();

        if (tecla == 'D' || tecla == 'd') {
            printf("tecla D digitada\n");
        }
        else if (tecla == 'O' || tecla == 'o') {
            printf("tecla O digitada\n");
        }
        else if (tecla == 'P' || tecla == 'p') {
            printf("tecla P digitada\n");
        }
        else if (tecla == 'Q' || tecla == 'q') {
            printf("tecla Q digitada\n");
        }
        else if (tecla == 'S' || tecla == 's') {
            printf("tecla S digitada\n");
        }
        else if (tecla == 'X' || tecla == 'x') {
            printf("tecla X digitada\n");
        }
        else if (tecla == TECLA_ESC) {
            printf("tecla ESC digitada\n");
        }
        else {
            printf("a tecla digitada nao esta configurada\n");
        }

    } while (tecla != TECLA_ESC);


}
