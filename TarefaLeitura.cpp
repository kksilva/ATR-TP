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
//#include "bGetString.h"
//#include <pthread.h>
#include <string>
#define _CHECKERROR	1
#include "CheckForError.h"

#define WNT_LEAN_AND_MEAN


// *** Constantes ***
#define WIN32_LEAN_AND_MEAN 
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
DWORD WINAPI WaitEventFunc(LPVOID);
#define TAMANHO_LISTA_CIRCULAR 200

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
#define TAMANHO_BYTES_LISTA (TAMANHO_MSG_MAX * TAMANHO_LISTA_CIRCULAR)

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


// Funções
void GerarMensagem(const char tipoMensagem);
void GerarMensagemProcesso(char *mensagem);
void GerarMensagemAlarme(char *mensagem);
void GerarMensagemOtimizacao(char* mensagem);
void DepositarMensagem(char* mensagem);
void TraduzirIntParaChar(char* mensagem, int valor, int tamanho, int offset);
void TraduzirDoubleParaChar(char* mensagem, double valor, int tamanho, int offset);
int RandFaixaInt(int min, int max);
double RandFaixaDouble(double min, double max);

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

// Variáveis Globais Gerais
HANDLE hEventoTeclaEsc, hEventoTeclaD;
HANDLE hMensagemDisponivel, hLerMensagem; 

// Variáveis Globais da Lista Circular
char* listaCircular[TAMANHO_LISTA_CIRCULAR];
int posicaoLivre = 0;
HANDLE hMutexListaCircular, hSemaforoLivres, hSemaforoOcupadas;

// Variáveis Globais para os NSEQ das mensagens
int nSeqProcesso = 0, nSeqAlarme = 0, nSeqOtimizacao = 0, nSeqGeral = 0;
HANDLE hMutexNSeq;


int main() {

    printf("*** Tarefa de Leitura Iniciada ***\n");
    DWORD retornoWait;
    HANDLE hListaCircular, hEspera[2], hEsperaTeclas[2];
    BOOL bStatus;
    
    srand((unsigned int)time(NULL));
    
    // Implementação lista circular:
    hListaCircular = CreateFileMapping(
        NULL,
        NULL,                       // Estrutura Security_Attributes
        PAGE_READWRITE,             // Tipo de acesso
        0,                          // 32 bits mais significativos do tamanho
        TAMANHO_BYTES_LISTA,        // 32 bits menos significativos do tamanho
        "LISTA_CIRCULAR"            // Apontador para o nome do objeto
    );
    
   //CheckForError(hListaCircular == 0);
    

    DWORD offsetListaCircular = 0;
    for (int i = 0; i < TAMANHO_LISTA_CIRCULAR; i++) {
        
        listaCircular[i] = (char*)MapViewOfFile(
            hListaCircular,             // Handle para o arquivo mapeado
            FILE_MAP_ALL_ACCESS,		// Direitos de acesso: leitura e escrita
            0,                          // Bits mais significativos da "visão"
            offsetListaCircular,        // Bits menos significativos da "visão"
            TAMANHO_MSG_MAX
        );
        offsetListaCircular += TAMANHO_MSG_MAX;
    }
    
    
    //Abrir Eventos vindos de outros processos:
    hEventoTeclaEsc = OpenEvent(EVENT_ALL_ACCESS, FALSE, "TeclaESC");
    if (hEventoTeclaEsc == 0) { printf("[LEITURA] Erro na abertura do evento <Tecla ESC Pressionada>\n"); exit(-1); }
    hEventoTeclaD = OpenEvent(EVENT_ALL_ACCESS, FALSE, "TeclaD");
    if (hEventoTeclaD == 0) { printf("[LEITURA] Erro na abertura do evento <Tecla D Pressionada>\n"); exit(-1); }

    //Cria Eventos para sinalizacao dos outros processos
    hMensagemDisponivel = CreateEvent(NULL, FALSE, FALSE, "hMensagemDisponivel");
    if (hMensagemDisponivel == 0) { printf("[LEITURA] Erro na criacao do evento <Mensagem Disponivel>\n"); exit(-1); }
    hLerMensagem = CreateEvent(NULL, FALSE, FALSE, "hLerMensagem");
    if (hLerMensagem == 0) { printf("[LEITURA] Erro na criacao do evento <Ler Mensagem>\n"); exit(-1); }

    // Mutex para alterar o NSeq das mensagens
    hMutexNSeq = CreateMutex(NULL, FALSE, "MutexNSeq");
    if (hMutexNSeq == 0) { printf("[LEITURA] Erro na criacao do Mutex de manipulação no NSeq\n"); exit(-1); }

    // Mutex para escrever na lista circular
    hMutexListaCircular = CreateMutex(NULL, FALSE, "MutexListaCircular");
    if (hMutexListaCircular == 0) { printf("[LEITURA] Erro na criacao do Mutex de Manipulacao da Lista Circular\n"); exit(-1); }

    // Semáforos da Lista Circular de acordo com o problema dos Produtores e Consumidores
    hSemaforoLivres = CreateSemaphore(NULL, TAMANHO_LISTA_CIRCULAR, TAMANHO_LISTA_CIRCULAR, "SemaforoLivres");
    if (hSemaforoLivres == 0) { printf("[LEITURA] Erro na Criacao do Semaforo de Posicoes Livres\n"); exit(-1); }
    hSemaforoOcupadas = CreateSemaphore(NULL, 0, TAMANHO_LISTA_CIRCULAR, "SemaforoOcupadas");
    if (hSemaforoOcupadas == 0) { printf("[LEITURA] Erro na Criacao do Semaforo de Posicoes Ocupadas\n"); exit(-1); }

    // Vetor de Eventos a serem utilizados no WaitForMultipleObjects
    hEspera[0] = hEventoTeclaD;
    hEspera[1] = hEventoTeclaEsc;
            // Será maior na parte 2 do trabalho

    hEsperaTeclas[0] = hEventoTeclaD;
    hEsperaTeclas[1] = hEventoTeclaEsc;

    printf("*** Tarefa de Leitura Executando Rotina ***\n");
    // Início da Rotina
    do {
        retornoWait = WaitForMultipleObjects(2, hEspera, FALSE, 1000);
        //CheckForError(retornoWait == WAIT_OBJECT_0);

        if (retornoWait == WAIT_TIMEOUT) {
            // Essa função irá mudar na parte dois do trabalho, ficando mais complexa
            GerarMensagem('A');
            GerarMensagem('P');
            
            GerarMensagem('O');
        }

        if(retornoWait == (WAIT_OBJECT_0 + 0)) {
            // Bloqueia-se esperando a tecla D novamente
            printf("*** Tarefa de Leitura Bloqueada! ***\n");
            retornoWait = WaitForMultipleObjects(2, hEsperaTeclas, FALSE, INFINITE);
            //CheckForError(retornoWait == WAIT_OBJECT_0);
            printf("*** Tarefa de Leitura Desbloqueada! ***\n");
        }

    } while (retornoWait == (WAIT_OBJECT_0 + 1));

    printf("*** Finalizando Tarefa de Leitura! ***\n");

    for (int i = 0; i < TAMANHO_LISTA_CIRCULAR; i++) {
        bStatus = UnmapViewOfFile(listaCircular[i]);
        //CheckForError(bStatus == 0);
    }


    CloseHandle(hListaCircular);
    CloseHandle(hEventoTeclaEsc);
    CloseHandle(hEventoTeclaD);
    CloseHandle(hMensagemDisponivel);
    CloseHandle(hLerMensagem);
    CloseHandle(hMutexNSeq);
    CloseHandle(hMutexListaCircular);
    CloseHandle(hSemaforoLivres);
    CloseHandle(hSemaforoOcupadas);


    return 0;
}

void GerarMensagem(const char tipoMensagem) {

    char mensagem[TAMANHO_MSG_MAX];

    if (tipoMensagem == 'A')
        GerarMensagemAlarme(mensagem);
    else if (tipoMensagem == 'P')
        GerarMensagemProcesso(mensagem);
    else if (tipoMensagem == 'O')
        GerarMensagemOtimizacao(mensagem);
    else {
        printf("[Erro][Função GerarMensagem]: Tipo de Mensagem Incorreta!");
        exit(-1);
    }

    if(tipoMensagem == 'P' || tipoMensagem == 'O') DepositarMensagem(mensagem);

}

void GerarMensagemProcesso(char* mensagem) {
    mensagemProcesso novaMensagem;
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

    TraduzirIntParaChar(mensagem, novaMensagem.nSeq, TAMANHO_NSEQ, offset);
    offset += TAMANHO_NSEQ;
    mensagem[offset] = '$';
    offset++;

    // 2. Obter Tipo
    TraduzirIntParaChar(mensagem, TIPO_PROCESSO, TAMANHO_TIPO, offset);
    offset += TAMANHO_TIPO;
    mensagem[offset] = '$';
    offset++;

    // 3. Obter a Temperatura da Zona de Pré-Aquecimento
    novaMensagem.tZonaP = RandFaixaDouble(TEMP_MIN_PRE, TEMP_MAX_PRE);
    TraduzirDoubleParaChar(mensagem, novaMensagem.tZonaP, TAMANHO_T_ZONAS, offset);
    offset += TAMANHO_T_ZONAS;
    mensagem[offset] = '$';
    offset++;

    // 4. Obter a Temperatura da Zona de Aquecimento
    novaMensagem.tZonaA = RandFaixaDouble(TEMP_MIN_AQUECIMENTO, TEMP_MAX_AQUECIMENTO);
    TraduzirDoubleParaChar(mensagem, novaMensagem.tZonaA, TAMANHO_T_ZONAS, offset);
    offset += TAMANHO_T_ZONAS;
    mensagem[offset] = '$';
    offset++;

    // 5. Obter a Temperatura da Zona de Encharque
    novaMensagem.tZonaE = RandFaixaDouble(TEMP_MIN_ENCHARQUE, TEMP_MAX_ENCHARQUE);
    TraduzirDoubleParaChar(mensagem, novaMensagem.tZonaE, TAMANHO_T_ZONAS, offset);
    offset += TAMANHO_T_ZONAS;
    mensagem[offset] = '$';
    offset++;

    // 6. Obter a Pressão
    novaMensagem.pressao = RandFaixaDouble(PRESSAO_MIN, PRESSAO_MAX);
    TraduzirDoubleParaChar(mensagem, novaMensagem.pressao, TAMANHO_PRESSAO, offset);
    offset += TAMANHO_PRESSAO;
    mensagem[offset] = '$';
    offset++;

    // 7. Obter o Horário
    GetLocalTime(&relogio);
    novaMensagem.hora = relogio.wHour;
    novaMensagem.minuto = relogio.wMinute;
    novaMensagem.segundo = relogio.wSecond;

    TraduzirIntParaChar(mensagem, novaMensagem.hora, TAMANHO_PARTE_TEMPO, offset);
    offset += TAMANHO_PARTE_TEMPO;
    mensagem[offset] = ':';
    offset++;
    TraduzirIntParaChar(mensagem, novaMensagem.minuto, TAMANHO_PARTE_TEMPO, offset);
    offset += TAMANHO_PARTE_TEMPO;
    mensagem[offset] = ':';
    offset++;
    TraduzirIntParaChar(mensagem, novaMensagem.segundo, TAMANHO_PARTE_TEMPO, offset);
    offset += TAMANHO_PARTE_TEMPO;
    mensagem[offset] = '\0';


}

void GerarMensagemAlarme(char* mensagem) {
    mensagemAlarme novaMensagem;
    DWORD status;
    SYSTEMTIME relogio;
    int offset = 0;

    // 1. Obter o NSeq
    status = WaitForSingleObject(hMutexNSeq, INFINITE);
    //CheckForError(status == 0);
    if (nSeqAlarme == NSEQ_MAX) nSeqAlarme = 0;
    novaMensagem.nSeq = nSeqAlarme;
    nSeqAlarme++;
    nSeqGeral++;
    status = ReleaseMutex(hMutexNSeq);
    //CheckForError(status == 0);

    TraduzirIntParaChar(mensagem, novaMensagem.nSeq, TAMANHO_NSEQ, offset);
    offset += TAMANHO_NSEQ;
    mensagem[offset] = '$';
    offset++;

    // 2. Obter Tipo
    TraduzirIntParaChar(mensagem, TIPO_ALARME, TAMANHO_TIPO, offset);
    offset += TAMANHO_TIPO;
    mensagem[offset] = '$';
    offset++;

    // 3. Obter o Código do Alarme
    novaMensagem.codigo = RandFaixaInt(CODIGO_ALARME_MIN, CODIGO_ALARME_MAX);
    TraduzirIntParaChar(mensagem, novaMensagem.codigo, TAMANHO_CODIGO, offset);
    offset += TAMANHO_CODIGO;
    mensagem[offset] = '$';
    offset++;

    // 4. Obter o Horário
    GetLocalTime(&relogio);
    novaMensagem.hora = relogio.wHour;
    novaMensagem.minuto = relogio.wMinute;
    novaMensagem.segundo = relogio.wSecond;

    TraduzirIntParaChar(mensagem, novaMensagem.hora, TAMANHO_PARTE_TEMPO, offset);
    offset += TAMANHO_PARTE_TEMPO;
    mensagem[offset] = ':';
    offset++;
    TraduzirIntParaChar(mensagem, novaMensagem.minuto, TAMANHO_PARTE_TEMPO, offset);
    offset += TAMANHO_PARTE_TEMPO;
    mensagem[offset] = ':';
    offset++;
    TraduzirIntParaChar(mensagem, novaMensagem.segundo, TAMANHO_PARTE_TEMPO, offset);
    offset += TAMANHO_PARTE_TEMPO;
    mensagem[offset] = '\0';

}

void GerarMensagemOtimizacao(char* mensagem) {

    
    mensagemOtimizacao novaMensagem;
    DWORD status;
    SYSTEMTIME relogio;
    int offset = 0;
    
    // 1. Obter o NSeq
    status = WaitForSingleObject(hMutexNSeq, INFINITE);
    //CheckForError(status == 0);
    if (nSeqOtimizacao == NSEQ_MAX) nSeqOtimizacao = 0;
    novaMensagem.nSeq = nSeqOtimizacao;
    nSeqOtimizacao++;
    nSeqGeral++;
    status = ReleaseMutex(hMutexNSeq);
    //CheckForError(status == 0);
    
    TraduzirIntParaChar(mensagem, novaMensagem.nSeq, TAMANHO_NSEQ, offset);
    offset += TAMANHO_NSEQ;
    mensagem[offset] = '$';
    offset++;

    // 2. Obter Tipo
    TraduzirIntParaChar(mensagem, TIPO_OTIMIZACAO, TAMANHO_TIPO, offset);
    offset += TAMANHO_TIPO;
    mensagem[offset] = '$';
    offset++;

    // 3. Obter o Set-Point de Temperatura da Zona de Pré-Aquecimento
    novaMensagem.spZonaP = RandFaixaDouble(TEMP_MIN_PRE, TEMP_MAX_PRE);
    TraduzirDoubleParaChar(mensagem, novaMensagem.spZonaP, TAMANHO_T_ZONAS, offset);
    offset += TAMANHO_T_ZONAS;
    mensagem[offset] = '$';
    offset++;

    // 4. Obter o Set-Point de Temperatura da Zona de Aquecimento
    novaMensagem.spZonaA = RandFaixaDouble(TEMP_MIN_AQUECIMENTO, TEMP_MAX_AQUECIMENTO);
    TraduzirDoubleParaChar(mensagem, novaMensagem.spZonaA, TAMANHO_T_ZONAS, offset);
    offset += TAMANHO_T_ZONAS;
    mensagem[offset] = '$';
    offset++;

    // 5. Obter o Set-Point de Temperatura da Zona de Encharque
    novaMensagem.spZonaE = RandFaixaDouble(TEMP_MIN_ENCHARQUE, TEMP_MAX_ENCHARQUE);
    TraduzirDoubleParaChar(mensagem, novaMensagem.spZonaE, TAMANHO_T_ZONAS, offset);
    offset += TAMANHO_T_ZONAS;
    mensagem[offset] = '$';
    offset++;

    // 6. Obter o Horário
    GetLocalTime(&relogio);
    novaMensagem.hora = relogio.wHour;
    novaMensagem.minuto = relogio.wMinute;
    novaMensagem.segundo = relogio.wSecond;

    TraduzirIntParaChar(mensagem, novaMensagem.hora, TAMANHO_PARTE_TEMPO, offset);
    offset += TAMANHO_PARTE_TEMPO;
    mensagem[offset] = ':';
    offset++;
    TraduzirIntParaChar(mensagem, novaMensagem.minuto, TAMANHO_PARTE_TEMPO, offset);
    offset += TAMANHO_PARTE_TEMPO;
    mensagem[offset] = ':';
    offset++;
    TraduzirIntParaChar(mensagem, novaMensagem.segundo, TAMANHO_PARTE_TEMPO, offset);
    offset += TAMANHO_PARTE_TEMPO;
    mensagem[offset] = '\0';
   
}

void DepositarMensagem(char* mensagem) {
    DWORD status;

    status = WaitForSingleObject(hMutexListaCircular, INFINITE);
    //CheckForError(status == 0);

    status = WaitForSingleObject(hSemaforoLivres, INFINITE);
    //CheckForError(status == 0);

    


    for (int i = 0; i <= TAMANHO_MSG_MAX; i++) {
        listaCircular[posicaoLivre][i] = mensagem[i];
        if (mensagem[i] = '\0') break;  
    }
    printf("mensagem %d armazenada com sucesso\n", posicaoLivre);
    posicaoLivre = (posicaoLivre + 1) % TAMANHO_LISTA_CIRCULAR;
    status = ReleaseSemaphore(hSemaforoOcupadas, 1, NULL);
    //CheckForError(status == 0);

    status = ReleaseMutex(hMutexListaCircular);
    //CheckForError(status == 0);

    printf("erro tarefa de leitura\n");
    exit(-1);
}

void TraduzirIntParaChar(char* mensagem, int valor, int tamanho, int offset) {


    for (int i = offset; i < (tamanho + offset); i++) {
        mensagem[i] = '0';
    }

    if (valor == 0) {
        return;
    }

    for (int i = 1; valor > 0; i++) {
        mensagem[tamanho + offset - i] = (valor % 10) + '0';
        valor /= 10;
    }
    return;
}

void TraduzirDoubleParaChar(char* mensagem, double valor, int tamanho, int offset) {

    int parteInteira = (int)valor;
    int parteDecimal;
    int tamanhoParteInteira = tamanho - TAMANHO_DECIMAL - 1;


    TraduzirIntParaChar(mensagem, parteInteira, tamanhoParteInteira, offset);
    offset += tamanhoParteInteira;
    mensagem[offset] = '.';
    offset++;

    valor -= parteInteira;
    parteDecimal = (int)(valor * pow(10, TAMANHO_DECIMAL));
    TraduzirIntParaChar(mensagem, parteDecimal, TAMANHO_DECIMAL, offset);

    return;
}

int RandFaixaInt(int min, int max) {

    int range = (max - min);
    int div = RAND_MAX / range;
    return min + (rand() / div);
}

double RandFaixaDouble(double min, double max) {

    double range = (max - min);
    double div = RAND_MAX / range;
    return min + (rand() / div);
}
