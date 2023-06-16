// *** Bibliotecas ***
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
DWORD WINAPI WaitEventFunc(LPVOID);	// declara��o da fun��o  coisa do c�digo exemplo

// Casting para terceiro e sexto par�metros da fun��o _beginthreadex
typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);
typedef unsigned* CAST_LPDWORD;

// *** Fun��es ***
void EsperaTeclado();
void CriaObjetos();
void FechaObjetos();

// Vari�veis Globais
PROCESS_INFORMATION ProcessoLeitura;	// Informa��es sobre novo processo criado
PROCESS_INFORMATION ProcessoCaptura;	// Informa��es sobre novo processo criado
PROCESS_INFORMATION ProcessoExibicao;	// Informa��es sobre novo processo criado

HANDLE hEventoTeclaEsc, hEventoTeclaD, hEventoTeclaO, hEventoTeclaP, hEventoTeclaQ, hEventoTeclaS, hEventoTeclaX;
HANDLE hEventoTudoPronto, hEventoLeituraPronta, hEventoCapturaProcessoPronta, hEventoCapturaOtimizacaoPronta,
hEventoExibicaoProcessoPronta, hEventoExibicaoOtimizacaoPronta;

int main() {

    // 1. Inicializa��o
    SetConsoleTitle("Integrador SCADA e CLP");
    printf("============ Integrador SCADA e CLP ============\n");

    // 2. Eventos de Tecla
    CriaObjetos();

    // 3. Cria��o de processos
        // 3.1 Declara��o de Vari�veis
    BOOL status;
    STARTUPINFO si;						    // StartUpInformation para novo processo
    DWORD retornoWait;

    // 3.2. Cria o Processo de Leitura de Mensagens
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);	// Tamanho da estrutura em bytes
    status = CreateProcess(
        "..\\Leitura\\TarefaLeitura\\x64\\Debug\\TarefaLeitura.exe",
        NULL,											// linha de comando
        NULL,											// atributos de seguran�a do processo
        NULL,											// atributos de seguran�a da thread
        FALSE,											// heran�a de handles
        NORMAL_PRIORITY_CLASS,							// CreationFlags
        NULL,											// lpEnvironment
        "..\\Leitura\\TarefaLeitura\\x64\\Debug\\",		// diret�rio corrente do filho
        &si,											// lpStartUpInfo
        &ProcessoLeitura
    );
    if (!status) printf("[Main] Erro na criacao do <Processo de Leitura>! Codigo = %d\n", GetLastError());


    // Espera o Primeiro Processo Filho para Iniciar o Segundo 
    // (de maneira a garantir que os mailslots e arquivos em disco sejam devidamente abertos S� depois de serem criados)
    HANDLE hEsperaProcessoLeituraCaptura[3];

    hEsperaProcessoLeituraCaptura[0] = hEventoLeituraPronta;
    hEsperaProcessoLeituraCaptura[1] = hEventoCapturaProcessoPronta;
    hEsperaProcessoLeituraCaptura[2] = hEventoCapturaOtimizacaoPronta;
    retornoWait = WaitForMultipleObjects(3, hEsperaProcessoLeituraCaptura, TRUE, INFINITE);


    // 3.3. Cria o processo de Exibi��o de Mensagens
    status = CreateProcess(
        "..\\Exibicao\\TarefasExibicao\\x64\\Debug\\TarefasExibicao.exe",
        NULL,											// linha de comando
        NULL,											// atributos de seguran�a do processo
        NULL,											// atributos de seguran�a da thread
        FALSE,											// heran�a de handles
        NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE,		// CreationFlags
        NULL,											// lpEnvironment
        "..\\Exibicao\\TarefasExibicao\\x64\\Debug\\",  // diret�rio corrente do filho
        &si,											// lpStartUpInfo
        &ProcessoExibicao
    );
    if (!status) printf("Erro na criacao do <Processo de Exibicao>! Codigo = %d\n", GetLastError());

    // Espera o Segundo Proceso Filho para Iniciar a rotina geral
    HANDLE hEsperaProcessoExibicao[2];
    hEsperaProcessoExibicao[0] = hEventoExibicaoProcessoPronta;
    hEsperaProcessoExibicao[1] = hEventoExibicaoOtimizacaoPronta;
    retornoWait = WaitForMultipleObjects(2, hEsperaProcessoExibicao, TRUE, INFINITE);

    SetEvent(hEventoTudoPronto);

    // 5. Inicia e Mant�m a Rotina
    printf("Tarefa de Leitura do Teclado Executando Rotina\n");
    printf("Esperando uma tecla...\n");
    // At� a tecla ESC ser digitada, o processo permanecer� em loop dentro desta fun��o:
    EsperaTeclado();

    Sleep(5000);
    // 6. Finaliza��o de Handles
        // Destrui��o de outros objetos de sincroniza��o
    FechaObjetos();

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

void CriaObjetos() {
    // 2.1. Declara��o dos Handles de Eventos
           // Globais
    // 2.2. Cria��o dos Eventos
    hEventoTeclaEsc = CreateEvent(NULL, TRUE, FALSE, "TeclaESC");
    if (hEventoTeclaEsc == 0) { printf("Erro na criacao do Evento <Tecla ESC Pressionada>\n"); exit(-1); }
    hEventoTeclaD = CreateEvent(NULL, FALSE, FALSE, "TeclaD");
    if (hEventoTeclaD == 0) { printf("Erro na criacao do Evento <Tecla D Pressionada>\n"); exit(-1); }
    hEventoTeclaO = CreateEvent(NULL, FALSE, FALSE, "TeclaO");
    if (hEventoTeclaO == 0) { printf("Erro na criacao do Evento <Tecla O Pressionada>\n"); exit(-1); }
    hEventoTeclaP = CreateEvent(NULL, FALSE, FALSE, "TeclaP");
    if (hEventoTeclaP == 0) { printf("Erro na criacao do Evento <Tecla P Pressionada>\n"); exit(-1); }
    hEventoTeclaQ = CreateEvent(NULL, FALSE, FALSE, "TeclaQ");
    if (hEventoTeclaQ == 0) { printf("Erro na criacao do Evento <Tecla Q Pressionada>\n"); exit(-1); }
    hEventoTeclaS = CreateEvent(NULL, FALSE, FALSE, "TeclaS");
    if (hEventoTeclaS == 0) { printf("Erro na criacao do Evento <Tecla S Pressionada>\n"); exit(-1); }
    hEventoTeclaX = CreateEvent(NULL, FALSE, FALSE, "TeclaX");
    if (hEventoTeclaX == 0) { printf("Erro na criacao do Evento <Tecla X Pressionada>\n"); exit(-1); }

    // Evento que autoriza o come�o da rotina de todas as tarefas estiverem operantes
    hEventoTudoPronto = CreateEvent(NULL, TRUE, FALSE, "TudoPronto");
    if (hEventoTudoPronto == 0) { printf("Erro na criacao do Evento <Tudo Pronto>\n"); exit(-1); }
    hEventoLeituraPronta = CreateEvent(NULL, FALSE, FALSE, "LeituraPronta");
    if (hEventoLeituraPronta == 0) { printf("Erro na criacao do Evento <Leitura Pronta>\n"); exit(-1); }
    hEventoCapturaProcessoPronta = CreateEvent(NULL, FALSE, FALSE, "CapturaProcessoPronta");
    if (hEventoCapturaProcessoPronta == 0) { printf("Erro na criacao do Evento <Captura Processo Pronta>\n"); exit(-1); }
    hEventoCapturaOtimizacaoPronta = CreateEvent(NULL, FALSE, FALSE, "CapturaOtimizacaoPronta");
    if (hEventoCapturaOtimizacaoPronta == 0) { printf("Erro na criacao do Evento <Captura Otimizacao Pronta>\n"); exit(-1); }
    hEventoExibicaoProcessoPronta = CreateEvent(NULL, FALSE, FALSE, "ExibicaoProcessoPronta");
    if (hEventoExibicaoProcessoPronta == 0) { printf("Erro na criacao do Evento <Exibicao Processo Pronta>\n"); exit(-1); }
    hEventoExibicaoOtimizacaoPronta = CreateEvent(NULL, FALSE, FALSE, "ExibicaoOtimizacaoPronta");
    if (hEventoExibicaoOtimizacaoPronta == 0) { printf("Erro na criacao do Evento <Exibicao Otimizacao Pronta>\n"); exit(-1); }
}


void FechaObjetos() {
    // 6.1. Handles de Evento
    CloseHandle(hEventoTeclaEsc);
    CloseHandle(hEventoTeclaD);
    CloseHandle(hEventoTeclaO);
    CloseHandle(hEventoTeclaP);
    CloseHandle(hEventoTeclaQ);
    CloseHandle(hEventoTeclaS);
    CloseHandle(hEventoTeclaX);

    CloseHandle(hEventoTudoPronto);
    CloseHandle(hEventoLeituraPronta);
    CloseHandle(hEventoCapturaProcessoPronta);
    CloseHandle(hEventoCapturaOtimizacaoPronta);
    CloseHandle(hEventoExibicaoProcessoPronta);
    CloseHandle(hEventoExibicaoOtimizacaoPronta);

    // 6.2. Handles de Processo
    CloseHandle(ProcessoLeitura.hProcess);
    CloseHandle(ProcessoLeitura.hThread);
    //CloseHandle(ProcessoCaptura.hProcess);
    //CloseHandle(ProcessoCaptura.hThread);
    CloseHandle(ProcessoExibicao.hProcess);
    CloseHandle(ProcessoExibicao.hThread);

}