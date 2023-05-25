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
#include "bGetString.h"
//#include <pthread.h>
#include <string>
//#define _CHECKERROR	1
//#include "CheckForError.h"

#define WNT_LEAN_AND_MEAN


// *** Constantes ***
#define WIN32_LEAN_AND_MEAN 
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
DWORD WINAPI WaitEventFunc(LPVOID);	// declaração da função  coisa do código exemplo

// Tamanho dos Campos
#define TAMANHO_NSEQ 4
#define TAMANHO_TIPO 2
#define TAMANHO_T_ZONAS 6
#define TAMANHO_PRESSAO 4
#define TAMANHO_PARTE_TEMPO 2
#define TAMANHO_TEMPO 8
#define TAMANHO_DECIMAL 1
#define TAMANHO_CODIGO 2

// Tamanho das Mensagens
#define TAMANHO_MSG_PROCESSO 42+1
#define TAMANHO_MSG_ALARME 19+1
#define TAMANHO_MSG_OTIMIZACAO 37+1
#define TAMANHO_MSG_MAX 43

// Código Númerico dos Tipos
#define TIPO_PROCESSO 55
#define TIPO_ALARME 99
#define TIPO_OTIMIZACAO 01

// Set de Mínimo e Máximo de Leitura
#define NSEQ_MAX 9999
#define TEMP_MIN_PRE 700
#define TEMP_MAX_PRE 900
#define TEMP_MIN_AQUECIMENTO 901
#define TEMP_MAX_AQUECIMENTO 1200
#define TEMP_MIN_ENCHARQUE 1201
#define TEMP_MAX_ENCHARQUE 1400
#define PRESSAO_MAX 12
#define PRESSAO_MIN 10
#define CODIGO_ALARME_MIN 0
#define CODIGO_ALARME_MAX 99

char Mensagem[TAMANHO_MSG_MAX];

// Funções
void GerarMensagem(const char tipoMensagem);
void gerarMensagemProcesso();
void gerarMensagemAlarme();
void gerarMensagemOtimizacao();
void traduzirIntParaChar(char* mensagem, int valor, int tamanho, int offset);
void traduzirDoubleParaChar(char* mensagem, double valor, int tamanho, int offset);
int randFaixaInt(int min, int max);
double randFaixaDouble(double min, double max);

// Estruturas de Mensagens
struct mensagemProcesso {
    int nSeq{}, tipo = 55;
    double tZonaP{}, tZonaA{}, tZonaE{}, pressao{};
    int hora{}, minuto{}, segundo{};
};

struct mensagemAlarme {
    int nSeq{}, tipo = 99, codigo{};
    int hora{}, minuto{}, segundo{};
};

struct mensagemOtimizacao {
    int nSeq{}, tipo = 01;
    double spZonaP{}, spZonaA{}, spZonaE{};
    int hora{}, minuto{}, segundo{};
};

// Casting para terceiro e sexto parâmetros da função _beginthreadex
typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);
typedef unsigned* CAST_LPDWORD;

// Variáveis Globais
HANDLE hEventoTeclaEsc, hEventoTeclaD, hMutexNSeq;
HANDLE hMensagemDisponivel, hLerMensagem;

// Contadores NSEQ das mensagens
int nSeqProcesso = 0, nSeqAlarme = 0, nSeqOtimizacao = 0, nSeqGeral = 0;

int main() { // Receber parametros
    DWORD retornoWait;
    HANDLE hListaCircular, hEvent[2];
    BOOL bStatus;
    char* lpImage;;
    srand((unsigned int)time(NULL));

    // Implementação lista circular:
    hListaCircular = CreateFileMapping(
        (HANDLE)0xFFFFFFFF,
        NULL,                       // Estrutura Security_Attributes
        PAGE_READWRITE,             // Tipo de acesso
        0,                          // 32 bits mais significativos do tamanho
        TAMANHO_MSG_MAX,                    // 32 bits menos significativos do tamanho
        "LISTA_CIRCULAR"            // Apontador para o nome do objeto
    );
    //CheckForError(hListaCircular);

    lpImage = (char *)MapViewOfFile(
        hListaCircular,             //Handle para o arquivo mapeado
        FILE_MAP_WRITE,		        // Direitos de acesso: leitura e escrita
        0,                          // Bits mais significativos da "visão"
        0,                          // Bits menos significativos da "visão"
        TAMANHO_MSG_MAX
    );
    //CheckForError(lpImage);

    //Abrir Eventos vindos de outros processos:
    hEventoTeclaEsc = OpenEvent(EVENT_ALL_ACCESS, FALSE, "TeclaESC");
    if (hEventoTeclaEsc == 0) { printf("[LEITURA] Erro na abertura do evento <Tecla ESC Pressionada>\n"); exit(-1); }
    hEventoTeclaD = OpenEvent(EVENT_ALL_ACCESS, FALSE, "TeclaD");
    if (hEventoTeclaD == 0) { printf("[LEITURA] Erro na abertura do evento <Tecla D Pressionada>\n"); exit(-1); }

    //Cria Eventos para sinalizacao dos outros processos
    hMensagemDisponivel = CreateEvent(NULL, FALSE, FALSE, "hMensagemDisponivel");
    if (hEventoTeclaEsc == 0) { printf("[LEITURA] Erro na criacao do evento <Mensagem Disponivel>\n"); exit(-1); }
    hLerMensagem = CreateEvent(NULL, FALSE, FALSE, "hLerMensagem");
    if (hEventoTeclaEsc == 0) { printf("[LEITURA] Erro na criacao do evento <Ler Mensagem>\n"); exit(-1); }



    do {
        printf("Entre com nova Mensagem a ser enviada:\n");
        bStatus = bGetString(Mensagem, TAMANHO_MSG_MAX);
        printf("\n");

        // Escreve na memória compartilhada
        if (bStatus) strcpy(lpImage, Mensagem);
        else strcpy(lpImage, ""); // Aborta Processo Servidor
        SetEvent(hMensagemDisponivel);	// Avisa ao outro Processo que foi escrito

        if (!bStatus) break;

        // Espera que o outro processo leia a mensagem 
        WaitForSingleObject(hLerMensagem, INFINITE);
        ResetEvent(hLerMensagem);
        printf("Buffer apos leitura/limpeza [deve ser NULL]= %s\n\n", lpImage);
    } while (TRUE);


    hMutexNSeq = CreateMutex(NULL, FALSE, "MutexNSeq");
      
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
    CloseHandle(hMensagemDisponivel);
    CloseHandle(hLerMensagem);

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
    char mensagem[TAMANHO_MSG_PROCESSO];
    DWORD status;
    SYSTEMTIME relogio;
    int offset = 0;

    // 1. Obter o NSeq
    status = WaitForSingleObject(hMutexNSeq, INFINITE);
    //CheckForError(status == WAIT_OBJECT_0);
    if (nSeqProcesso == NSEQ_MAX) nSeqProcesso = 0;
    novaMensagem.nSeq = nSeqProcesso;
    nSeqProcesso++;
    nSeqGeral++;
    status = ReleaseMutex(hMutexNSeq);
    //CheckForError(status == WAIT_OBJECT_0);

    traduzirIntParaChar(mensagem, novaMensagem.nSeq, TAMANHO_NSEQ, offset);
    offset += TAMANHO_NSEQ;
    mensagem[offset] = '$';
    offset++;

    // 2. Obter Tipo
    traduzirIntParaChar(mensagem, TIPO_PROCESSO, TAMANHO_TIPO, offset);
    offset += TAMANHO_TIPO;
    mensagem[offset] = '$';
    offset++;

    // 3. Obter a Temperatura da Zona de Pré-Aquecimento
    novaMensagem.tZonaP = randFaixaDouble(TEMP_MIN_PRE, TEMP_MAX_PRE);
    traduzirDoubleParaChar(mensagem, novaMensagem.tZonaP, TAMANHO_T_ZONAS, offset);
    offset += TAMANHO_T_ZONAS;
    mensagem[offset] = '$';
    offset++;

    // 4. Obter a Temperatura da Zona de Aquecimento
    novaMensagem.tZonaA = randFaixaDouble(TEMP_MIN_AQUECIMENTO, TEMP_MAX_AQUECIMENTO);
    traduzirDoubleParaChar(mensagem, novaMensagem.tZonaA, TAMANHO_T_ZONAS, offset);
    offset += TAMANHO_T_ZONAS;
    mensagem[offset] = '$';
    offset++;

    // 5. Obter a Temperatura da Zona de Encharque
    novaMensagem.tZonaE = randFaixaDouble(TEMP_MIN_ENCHARQUE, TEMP_MAX_ENCHARQUE);
    traduzirDoubleParaChar(mensagem, novaMensagem.tZonaE, TAMANHO_T_ZONAS, offset);
    offset += TAMANHO_T_ZONAS;
    mensagem[offset] = '$';
    offset++;

    // 6. Obter a Pressão
    novaMensagem.pressao = randFaixaDouble(PRESSAO_MIN, PRESSAO_MAX);
    traduzirDoubleParaChar(mensagem, novaMensagem.pressao, TAMANHO_PRESSAO, offset);
    offset += TAMANHO_PRESSAO;
    mensagem[offset] = '$';
    offset++;

    // 7. Obter o Horário
    GetLocalTime(&relogio);
    novaMensagem.hora = relogio.wHour;
    novaMensagem.minuto = relogio.wMinute;
    novaMensagem.segundo = relogio.wSecond;

    traduzirIntParaChar(mensagem, novaMensagem.hora, TAMANHO_PARTE_TEMPO, offset);
    offset += TAMANHO_PARTE_TEMPO;
    mensagem[offset] = ':';
    offset++;
    traduzirIntParaChar(mensagem, novaMensagem.minuto, TAMANHO_PARTE_TEMPO, offset);
    offset += TAMANHO_PARTE_TEMPO;
    mensagem[offset] = ':';
    offset++;
    traduzirIntParaChar(mensagem, novaMensagem.segundo, TAMANHO_PARTE_TEMPO, offset);
    offset += TAMANHO_PARTE_TEMPO;
    mensagem[offset] = '\0';
}

void gerarMensagemAlarme() {
    mensagemAlarme novaMensagem;
    char mensagem[TAMANHO_MSG_ALARME];
    DWORD status;
    SYSTEMTIME relogio;
    int offset = 0;

    // 1. Obter o NSeq
    status = WaitForSingleObject(hMutexNSeq, INFINITE);
    //CheckForError(status);
    if (nSeqAlarme == NSEQ_MAX) nSeqAlarme = 0;
    novaMensagem.nSeq = nSeqAlarme;
    nSeqAlarme++;
    nSeqGeral++;
    status = ReleaseMutex(hMutexNSeq);
    //CheckForError(status);

    traduzirIntParaChar(mensagem, novaMensagem.nSeq, TAMANHO_NSEQ, offset);
    offset += TAMANHO_NSEQ;
    mensagem[offset] = '$';
    offset++;

    // 2. Obter Tipo
    traduzirIntParaChar(mensagem, TIPO_ALARME, TAMANHO_TIPO, offset);
    offset += TAMANHO_TIPO;
    mensagem[offset] = '$';
    offset++;

    // 3. Obter o Código do Alarme
    novaMensagem.codigo = randFaixaInt(CODIGO_ALARME_MIN, CODIGO_ALARME_MAX);
    traduzirIntParaChar(mensagem, novaMensagem.codigo, TAMANHO_CODIGO, offset);
    offset += TAMANHO_CODIGO;
    mensagem[offset] = '$';
    offset++;

    // 4. Obter o Horário
    GetLocalTime(&relogio);
    novaMensagem.hora = relogio.wHour;
    novaMensagem.minuto = relogio.wMinute;
    novaMensagem.segundo = relogio.wSecond;

    traduzirIntParaChar(mensagem, novaMensagem.hora, TAMANHO_PARTE_TEMPO, offset);
    offset += TAMANHO_PARTE_TEMPO;
    mensagem[offset] = ':';
    offset++;
    traduzirIntParaChar(mensagem, novaMensagem.minuto, TAMANHO_PARTE_TEMPO, offset);
    offset += TAMANHO_PARTE_TEMPO;
    mensagem[offset] = ':';
    offset++;
    traduzirIntParaChar(mensagem, novaMensagem.segundo, TAMANHO_PARTE_TEMPO, offset);
    offset += TAMANHO_PARTE_TEMPO;
    mensagem[offset] = '\0';
}

void gerarMensagemOtimizacao() {
    mensagemOtimizacao novaMensagem;
    char mensagem[TAMANHO_MSG_OTIMIZACAO];
    DWORD status;
    SYSTEMTIME relogio;
    int offset = 0;

    // 1. Obter o NSeq
    status = WaitForSingleObject(hMutexNSeq, INFINITE);
    //CheckForError(status);
    if (nSeqOtimizacao == NSEQ_MAX) nSeqOtimizacao = 0;
    novaMensagem.nSeq = nSeqOtimizacao;
    nSeqOtimizacao++;
    nSeqGeral++;
    status = ReleaseMutex(hMutexNSeq);
    //CheckForError(status);

    traduzirIntParaChar(mensagem, novaMensagem.nSeq, TAMANHO_NSEQ, offset);
    offset += TAMANHO_NSEQ;
    mensagem[offset] = '$';
    offset++;

    // 2. Obter Tipo
    traduzirIntParaChar(mensagem, TIPO_OTIMIZACAO, TAMANHO_TIPO, offset);
    offset += TAMANHO_TIPO;
    mensagem[offset] = '$';
    offset++;

    // 3. Obter o Set-Point de Temperatura da Zona de Pré-Aquecimento
    novaMensagem.spZonaP = randFaixaDouble(TEMP_MIN_PRE, TEMP_MAX_PRE);
    traduzirDoubleParaChar(mensagem, novaMensagem.spZonaP, TAMANHO_T_ZONAS, offset);
    offset += TAMANHO_T_ZONAS;
    mensagem[offset] = '$';
    offset++;

    // 4. Obter o Set-Point de Temperatura da Zona de Aquecimento
    novaMensagem.spZonaA = randFaixaDouble(TEMP_MIN_AQUECIMENTO, TEMP_MAX_AQUECIMENTO);
    traduzirDoubleParaChar(mensagem, novaMensagem.spZonaA, TAMANHO_T_ZONAS, offset);
    offset += TAMANHO_T_ZONAS;
    mensagem[offset] = '$';
    offset++;

    // 5. Obter o Set-Point de Temperatura da Zona de Encharque
    novaMensagem.spZonaE = randFaixaDouble(TEMP_MIN_ENCHARQUE, TEMP_MAX_ENCHARQUE);
    traduzirDoubleParaChar(mensagem, novaMensagem.spZonaE, TAMANHO_T_ZONAS, offset);
    offset += TAMANHO_T_ZONAS;
    mensagem[offset] = '$';
    offset++;

    // 6. Obter o Horário
    GetLocalTime(&relogio);
    novaMensagem.hora = relogio.wHour;
    novaMensagem.minuto = relogio.wMinute;
    novaMensagem.segundo = relogio.wSecond;

    traduzirIntParaChar(mensagem, novaMensagem.hora, TAMANHO_PARTE_TEMPO, offset);
    offset += TAMANHO_PARTE_TEMPO;
    mensagem[offset] = ':';
    offset++;
    traduzirIntParaChar(mensagem, novaMensagem.minuto, TAMANHO_PARTE_TEMPO, offset);
    offset += TAMANHO_PARTE_TEMPO;
    mensagem[offset] = ':';
    offset++;
    traduzirIntParaChar(mensagem, novaMensagem.segundo, TAMANHO_PARTE_TEMPO, offset);
    offset += TAMANHO_PARTE_TEMPO;
    mensagem[offset] = '\0';
}

void traduzirIntParaChar(char* mensagem, int valor, int tamanho, int offset) {


    for (int i = offset; i < (tamanho+offset); i++) {
        mensagem[i] = '0';
    }

    if(valor == 0){
        return;
    }

    for (int i = 1; valor > 0; i++) {
        mensagem[tamanho + offset - i] = (valor % 10) + '0';
        valor /= 10;
    }
    return;
}

void traduzirDoubleParaChar(char* mensagem, double valor, int tamanho, int offset) {

    int parteInteira = (int)valor;
    int parteDecimal;
    int tamanhoParteInteira = tamanho - TAMANHO_DECIMAL - 1;


    traduzirIntParaChar(mensagem, parteInteira, tamanhoParteInteira, offset);
    offset += tamanhoParteInteira;
    mensagem[offset] = '.';
    offset++;

    valor -= parteInteira;
    parteDecimal = (int)valor * pow(10, TAMANHO_DECIMAL);
    traduzirIntParaChar(mensagem, parteDecimal, TAMANHO_DECIMAL, offset);

    return;
}

int randFaixaInt(int min, int max) {

    int range = (max - min);
    int div = RAND_MAX / range;
    return min + (rand() / div);
}

double randFaixaDouble(double min, double max) {

    double range = (max - min);
    double div = RAND_MAX / range;
    return min + (rand() / div);
}
