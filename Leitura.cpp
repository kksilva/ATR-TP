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


int main() { // Receber parametros
    
    // recebe o evento de bloqueio e o esc
    // recebe ids de comunicação com as tarefas de captura
    // criar objetos de sincronização
        // mutex de leitura/escrita da lista circular
        // evento de temporização
    // criar a lista circular
    // gerar mensagem
        // armazenar mensagem na lista
    // wait for multiple, bloqueio ou ESC

    return 0;
}

