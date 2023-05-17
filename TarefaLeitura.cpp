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
#define WIN32_LEAN_AND_MEAN 
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
DWORD WINAPI WaitEventFunc(LPVOID);	// declaração da função  coisa do código exemplo

// Set de Mínimo e Máximo de Leitura
#define NSEQ_MAX 9999
#define TEMP_MIN_PRE 700
#define TEMP_MAX_PRE 900
#define TEMP_MIN_AQUECIMENTO 901
#define TEMP_MAX_AQUECIMENTO 1200
#define TEMP_MIN_ENCHARQUE 1201
#define TEMP_MAX_ENCHARQUE 1400
#define PRESSAO_MAX 12
#define PRESSAO_MAX 10
#define TAM_MSG 42

// Funções
void GerarMensagem(const char tipoMensagem);
void gerarMensagemProcesso();
void gerarMensagemAlarme();
void gerarMensagemOtimizacao();
int randfaixaint(int min, int max);
double randfaixadouble(double min, double max);

// Estruturas de Mensagens
struct mensagemProcesso {
    int nSeq, tipo = 55;
    float tZonaP, tZonaA, tZonaE, pressao;
    int hora, minuto, segundo;
};

struct mensagemAlarme {
    int nSeq, tipo = 99, codigo;
    int hora, minuto, segundo;
};

struct mensagemOtimizacao {
    int nSeq, tipo = 01;
    float tZonaP, tZonaA, tZonaE;
    int hora, minuto, segundo;
};

// Casting para terceiro e sexto parâmetros da função _beginthreadex
typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);
typedef unsigned* CAST_LPDWORD;

// Variáveis Globais
HANDLE hEventoTeclaEsc, hEventoTeclaD, hMutexNSeq;

// Contadores NSEQ das mensagens
int nSeqProcesso = 0, nSeqAlarme = 0, nSeqOtimizacao = 0, nSeqGeral = 0;

int main() { // Receber parametros
    DWORD retornoWait;
    HANDLE hListaCircular, hEvent[2];
    BOOL bStatus;
    char* lpImage;

    // Implementação lista circular:
    hListaCircular = CreateFileMapping(
        (HANDLE)0xFFFFFFFF,
        NULL,                       // Estrutura Security_Attributes
        PAGE_READWRITE,             // Tipo de acesso
        0,                          // 32 bits mais significativos do tamanho
        TAM_MSG,                    // 32 bits menos significativos do tamanho
        "LISTA_CIRCULAR"            // Apontador para o nome do objeto
    );
    //CheckForError(hListaCircular);

    lpImage = (char *)MapViewOfFile(
        hListaCircular,             //Handle para o arquivo mapeado
        FILE_MAP_WRITE,		        // Direitos de acesso: leitura e escrita
        0,                          // Bits mais significativos da "visão"
        0,                          // Bits menos significativos da "visão"
        TAM_MSG
    );
    //CheckForError(lpImage);

    printf("*** tarefa leitura iniciada ***\n");

    hEventoTeclaEsc = OpenEvent(EVENT_ALL_ACCESS, FALSE, "TeclaESC");
        if (hEventoTeclaEsc == 0) { printf("[LEITURA] Erro na abertura do evento <Tecla ESC Pressionada>\n"); exit(-1); }
    hEventoTeclaD = OpenEvent(EVENT_ALL_ACCESS, FALSE, "TeclaD");
        if (hEventoTeclaD == 0) { printf("[LEITURA] Erro na abertura do evento <Tecla D Pressionada>\n"); exit(-1); }
    
    HANDLE Eventos[2] = { hEventoTeclaD, hEventoTeclaEsc };
    retornoWait = WaitForMultipleObjects(2, Eventos, FALSE, INFINITE);      // Espera por um dos dois eventos do vetor Eventos
    //CheckForError(retornoWait == WAIT_OBJECT_0);
   
    int nTipoEvento = retornoWait - WAIT_OBJECT_0;
    if (nTipoEvento == 0) {
    
    //tecla D
    
    }
    hMutexNSeq = CreateMutex(NULL, FALSE, "MutexNSeq");
        if (hMutexNSeq == 0) { printf("[LEITURA] Erro na abertura do mutex <Mutex Numero de Sequencia>\n"); exit(-1); }







    //retornoWait = WaitForSingleObject(hEventoTeclaD, INFINITE);
    //retornoWait = WaitForMultipleObjects()
    //CheckForError(retornoWait == WAIT_OBJECT_0);

    // recebe o evento de bloqueio e o esc
    // recebe ids de comunicação com as tarefas de captura
    // criar objetos de sincronização
        // mutex de leitura/escrita da lista circular
        // evento de temporização
    // criar a lista circular
    // gerar mensagem
        // armazenar mensagem na lista
    // wait for multiple, bloqueio ou ESC

    bStatus = UnmapViewOfFile(lpImage);
    //CheckForError(bStatus);
    CloseHandle(hListaCircular);
    CloseHandle(hEventoTeclaEsc);
    CloseHandle(hEventoTeclaD);

    CloseHandle(hMutexNSeq);

    return 0;
}

void GerarMensagem(const char tipoMensagem) {

    
    if (tipoMensagem == 'A')
        gerarMensagemAlarme();
    else if (tipoMensagem == 'P')
        gerarMensagemProcesso();
    else if (tipoMensagem == 'O')
        gerarMensagemOtimizacao();
    else {
        printf("[Erro][Função GerarMensagem]: Tipo de Mensagem Incorreta!");
        exit(-1);
    }

    SYSTEMTIME time_stamp;
    GetLocalTime(&time_stamp);
    
    
    //intParaChar(time_stamp.wHour, hora, TAMANHO_TEMPO);
    //intParaChar(time_stamp.wMinute, minuto, TAMANHO_TEMPO);
    //intParaChar(time_stamp.wSecond, segundo, TAMANHO_TEMPO);

}

void gerarMensagemProcesso() {

    mensagemProcesso novaMensagem;
    DWORD status;

    // 1. Obter o NSeq
    status = WaitForSingleObject(hMutexNSeq, INFINITE);
        //CheckForError(status == WAIT_OBJECT_0);
    if (nSeqProcesso == NSEQ_MAX) nSeqProcesso = 0;
    novaMensagem.nSeq = nSeqProcesso;
    nSeqProcesso++;
    nSeqGeral++;
    status = ReleaseMutex(hMutexNSeq);
        //CheckForError(status == WAIT_OBJECT_0);

    // 2. Obter a Temperatura da Zona de Pré-Aquecimento
    //novaMensagem.tZonaP = randfaixadouble();




}
void gerarMensagemAlarme() {

}
void gerarMensagemOtimizacao() {

}

int randfaixaint(int min, int max) {

    int range = (max - min);
    int div = RAND_MAX / range;
    return min + (rand() / div);
}

double randfaixadouble(double min, double max) {

    double range = (max - min);
    double div = RAND_MAX / range;
    return min + (rand() / div);
}
