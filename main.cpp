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

// Variáveis Globais
HANDLE hEventoTeclaEsc, hEventoTeclaD, hEventoTeclaO, hEventoTeclaP, hEventoTeclaQ, hEventoTeclaS, hEventoTeclaX;

int main(){
    
    // 1. Inicialização
    SetConsoleTitle("Integrador SCADA e CLP");
    printf("============ Integrador SCADA e CLP ============\n");
    
    // 2. Eventos de Tecla
        // 2.1. Declaração dos Handles de Eventos
            // Globais
        // 2.2. Criação dos Eventos
    hEventoTeclaEsc = CreateEvent(NULL, TRUE, FALSE, "TeclaESC");
        if (hEventoTeclaEsc == 0) {printf("Erro na criacao do Evento <Tecla ESC Pressionada>\n"); exit(-1);}
        CheckForError(hEventoTeclaEsc);
    hEventoTeclaD = CreateEvent(NULL, FALSE, FALSE, "TeclaD");
        if (hEventoTeclaD == 0) { printf("Erro na criacao do Evento <Tecla D Pressionada>\n"); exit(-1);}
    hEventoTeclaO = CreateEvent(NULL, FALSE, FALSE, "TeclaO");
        if (hEventoTeclaO == 0) { printf("Erro na criacao do Evento <Tecla O Pressionada>\n"); exit(-1);}
    hEventoTeclaP = CreateEvent(NULL, FALSE, FALSE, "TeclaP");
        if (hEventoTeclaP == 0) { printf("Erro na criacao do Evento <Tecla P Pressionada>\n"); exit(-1);}
    hEventoTeclaQ = CreateEvent(NULL, FALSE, FALSE, "TeclaQ");
        if (hEventoTeclaQ == 0) { printf("Erro na criacao do Evento <Tecla Q Pressionada>\n"); exit(-1);}
    hEventoTeclaS = CreateEvent(NULL, FALSE, FALSE, "TeclaS");
        if (hEventoTeclaS == 0) { printf("Erro na criacao do Evento <Tecla S Pressionada>\n"); exit(-1);}
    hEventoTeclaX = CreateEvent(NULL, FALSE, FALSE, "TeclaX");
        if (hEventoTeclaX == 0) { printf("Erro na criacao do Evento <Tecla X Pressionada>\n"); exit(-1);}

    // 3. Criação de processos
        // 3.1 Declaração de Variáveis
    BOOL status;
    STARTUPINFO si;						    // StartUpInformation para novo processo
    PROCESS_INFORMATION ProcessoLeitura;	// Informações sobre novo processo criado
    PROCESS_INFORMATION ProcessoCaptura;	// Informações sobre novo processo criado
    PROCESS_INFORMATION ProcessoExibicao;	// Informações sobre novo processo criado


        // 3.2. Cria o Processo de Leitura de Mensagens
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);	// Tamanho da estrutura em bytes
    status = CreateProcess(
        "..\\Leitura\\TarefaLeitura\\x64\\Debug\\TarefaLeitura.exe",
        NULL,											// linha de comando
        NULL,											// atributos de segurança do processo
        NULL,											// atributos de segurança da thread
        FALSE,											// herança de handles
        NORMAL_PRIORITY_CLASS,							// CreationFlags
        NULL,											// lpEnvironment
        "..\\Leitura\\TarefaLeitura\\x64\\Debug\\",		// diretório corrente do filho
        &si,											// lpStartUpInfo
        &ProcessoLeitura
    );
    if (!status) printf("Erro na criacao do <Processo de Leitura>! Codigo = %d\n", GetLastError());

        // 3.3. Cria o Processo de Captura de Mensagens
    status = CreateProcess(
        "..\\Capturas\\TarefasCaptura\\x64\\Debug\\TarefasCaptura.exe",
        NULL,											// linha de comando
        NULL,											// atributos de segurança do processo
        NULL,											// atributos de segurança da thread
        FALSE,											// herança de handles
        NORMAL_PRIORITY_CLASS,							// CreationFlags
        NULL,											// lpEnvironment
        "..\\Capturas\\TarefasCaptura\\x64\\Debug\\",   // diretório corrente do filho
        &si,											// lpStartUpInfo
        &ProcessoCaptura
    );
    if (!status) printf("Erro na criacao do <Processo de Captura>! Codigo = %d\n", GetLastError());

        // 3.4. Cria o processo de Exibição de Mensagens
    status = CreateProcess(
        "..\\Exibicao\\TarefasExibicao\\x64\\Debug\\TarefasExibicao.exe",
        NULL,											// linha de comando
        NULL,											// atributos de segurança do processo
        NULL,											// atributos de segurança da thread
        FALSE,											// herança de handles
        NORMAL_PRIORITY_CLASS,							// CreationFlags
        NULL,											// lpEnvironment
        "..\\Exibicao\\TarefasExibicao\\x64\\Debug\\",  // diretório corrente do filho
        &si,											// lpStartUpInfo
        &ProcessoExibicao
    );
    if (!status) printf("Erro na criacao do <Processo de Exibicao>! Codigo = %d\n", GetLastError());



    // 5. Inicia e Mantém a Rotina
    printf("Tarefa de Leitura do Teclado corretamente iniciada\n");
    printf("Esperando uma tecla...\n");
        // Até a tecla ESC ser digitada, o processo permanecerá em loop dentro desta função:
    EsperaTeclado();        


    // 6. Finalização de Handles
        // 6.1. Handles de Evento
    CloseHandle(hEventoTeclaEsc);
    CloseHandle(hEventoTeclaD);
    CloseHandle(hEventoTeclaO);
    CloseHandle(hEventoTeclaP);
    CloseHandle(hEventoTeclaQ);
    CloseHandle(hEventoTeclaS);
    CloseHandle(hEventoTeclaX);

        // 6.2. Handles de Processo
    CloseHandle(ProcessoLeitura.hProcess);
    CloseHandle(ProcessoLeitura.hThread);
    CloseHandle(ProcessoCaptura.hProcess);
    CloseHandle(ProcessoCaptura.hThread);
    CloseHandle(ProcessoExibicao.hProcess);
    CloseHandle(ProcessoExibicao.hThread);

    // Destruir outros objetos de sincronização

    return 0;
}

void EsperaTeclado() {

    char tecla;

    do {
        tecla = _getch();

        if (tecla == 'D' || tecla == 'd') {
            printf("> Tecla D digitada\n");
            SetEvent(hEventoTeclaD);
            
        }
        else if (tecla == 'O' || tecla == 'o') {
            printf("> Tecla O digitada\n");
            SetEvent(hEventoTeclaO);
            
        }
        else if (tecla == 'P' || tecla == 'p') {
            printf("> Tecla P digitada\n");
            SetEvent(hEventoTeclaP);
            
        }
        else if (tecla == 'Q' || tecla == 'q') {
            printf("> Tecla Q digitada\n");
            SetEvent(hEventoTeclaQ);
            
        }
        else if (tecla == 'S' || tecla == 's') {
            printf("> Tecla S digitada\n");
            SetEvent(hEventoTeclaX);
            
        }
        else if (tecla == 'X' || tecla == 'x') {
            printf("> Tecla X digitada\n");
            SetEvent(hEventoTeclaX);
            
        }
        else if (tecla == TECLA_ESC) {
            printf("> Tecla ESC digitada\n");
            PulseEvent(hEventoTeclaEsc);
            
        }
        else {
            printf("> A tecla digitada nao esta configurada\n");
        }

    } while (tecla != TECLA_ESC);
}
