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
#include <process.h>
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

// *** Threads Secundárias ***
DWORD WINAPI TarefaCapturaDadosProcesso();
DWORD WINAPI TarefaCapturaDadosOtimizacao();
DWORD WINAPI TemporizacaoTarefas();

// Funções
void GerarMensagem(const char tipoMensagem);
void GerarMensagemProcesso(char* mensagem);
void GerarMensagemAlarme(char* mensagem);
void GerarMensagemOtimizacao(char* mensagem);
void DepositarMensagem(char* mensagem);
void TraduzirIntParaChar(char* mensagem, int valor, int tamanho, int offset);
void TraduzirDoubleParaChar(char* mensagem, double valor, int tamanho, int offset);
int RandFaixaInt(int min, int max);
double RandFaixaDouble(double min, double max);

void EnviarMensagemProcesso(char* mensagem);
void EnviarMensagemOtimizacao(char* mensagem);
void EnviarMensagemAlarme(char* mensagem);


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

// Funcoes de CALLBACK de Temporizacao
void CALLBACK MensagensProcessos(PVOID, BOOLEAN);
void CALLBACK MensagensAlarme(PVOID, BOOLEAN);
void CALLBACK MensagensOtimizacao(PVOID, BOOLEAN);

// Casting para terceiro e sexto parâmetros da função _beginthreadex
typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);
typedef unsigned* CAST_LPDWORD;

// Variáveis Globais Gerais
HANDLE hEventoTeclaEsc, hEventoTeclaD, hEventoTeclaO, hEventoTeclaP;
HANDLE hEventoTudoPronto, hEventoLeituraPronta, hEventoCapturaProcessoPronta, hEventoCapturaOtimizacaoPronta, hTemporizacaoPronta;
HANDLE hMsgAlarmeDisponivel, hMsgOtimizacaoDisponivel, hMsgProcessoDisponivel;
HANDLE hMailslotCaptura, hMailslotLeitura;
HANDLE hFilaTemporizadores, hTemporizadorAlarme, hTemporizadorProcesso, hTemporizadorOtimizacao;
HANDLE hEventoTimerAlarme, hEventoTimerProcesso, hEventoTimerOtimizacao;
HANDLE hArquivoOtimizacao; 



// Variáveis Globais da Lista Circular
int posicaoLivre = 0, posicaoOcupada = 0;
char listaCircular[TAMANHO_LISTA_CIRCULAR][TAMANHO_MSG_MAX];
DWORD offsetListaCircular = 0;
HANDLE hMutexListaCircular, hSemaforoLivres, hSemaforoOcupadas;
int temporizadorAleatorio;

// Variáveis Globais para os NSEQ das mensagens
int nSeqProcesso = 0, nSeqAlarme = 0, nSeqOtimizacao = 0, nSeqGeral = 0;
HANDLE hMutexNSeq;

// Tarefas de Captura
HANDLE hThreadDadosProcesso, hThreadDadosOtimizacao, hThreadTemporizacao;


int main() {

    printf("*** Tarefa de Leitura e Captura Iniciada ***\n");
    DWORD retornoWait;
    
    srand((unsigned int)time(NULL));

    //Abrir Eventos vindos de outros processos:
    hEventoTeclaEsc = OpenEvent(EVENT_ALL_ACCESS, FALSE, "TeclaESC");
    if (hEventoTeclaEsc == 0) { printf("[LEITURA] Erro na abertura do evento <Tecla ESC Pressionada>\n"); exit(-1); }
    hEventoTeclaD = OpenEvent(EVENT_ALL_ACCESS, FALSE, "TeclaD");
    if (hEventoTeclaD == 0) { printf("[LEITURA] Erro na abertura do evento <Tecla D Pressionada>\n"); exit(-1); }
    hEventoTeclaO = OpenEvent(EVENT_ALL_ACCESS, FALSE, "TeclaO");
    if (hEventoTeclaO == 0) { printf("[CAPTURA] Erro na abertura do evento <Tecla O Pressionada>\n"); exit(-1); }
    hEventoTeclaP = OpenEvent(EVENT_ALL_ACCESS, FALSE, "TeclaP");
    if (hEventoTeclaP == 0) { printf("[CAPTURA] Erro na abertura do evento <Tecla P Pressionada>\n"); exit(-1); }

    hEventoTudoPronto = OpenEvent(EVENT_ALL_ACCESS, FALSE, "TudoPronto");
    if (hEventoTudoPronto == 0) { printf("[LEITURA] Erro na abertura do evento <Tudo Pronto>\n"); exit(-1); }
    hEventoLeituraPronta = OpenEvent(EVENT_ALL_ACCESS, FALSE, "LeituraPronta");
    if (hEventoLeituraPronta == 0) { printf("[LEITURA] Erro na abertura do evento <Leitura Pronta>\n"); exit(-1); }
    hEventoCapturaProcessoPronta = OpenEvent(EVENT_ALL_ACCESS, FALSE, "CapturaProcessoPronta");
    if (hEventoCapturaProcessoPronta == 0) { printf("[CAPTURA] Erro na abertura do evento <Captura Processo Pronta>\n"); exit(-1); }
    hEventoCapturaOtimizacaoPronta = OpenEvent(EVENT_ALL_ACCESS, FALSE, "CapturaOtimizacaoPronta");
    if (hEventoCapturaOtimizacaoPronta == 0) { printf("[CAPTURA] Erro na abertura do evento <Captura Otimizacao Pronta>\n"); exit(-1); }
    hTemporizacaoPronta = OpenEvent(EVENT_ALL_ACCESS, FALSE, "TemporizacaoPronta");
    if (hTemporizacaoPronta == 0) { printf("[CAPTURA] Erro na abertura do evento <Temporizacao Pronta>\n"); exit(-1); }
    
    // Criação de Evento Msg Disponível
    hMsgAlarmeDisponivel = CreateEvent(NULL, FALSE, FALSE, "MsgAlarmeDisponivel");
    if (hMsgAlarmeDisponivel == 0) { printf("[CAPTURA] Erro na criacao do evento <MsgAlarmeDisponivel>\n"); exit(-1); }

    // Criação dos Eventos de temporização
    hEventoTimerAlarme = CreateEvent(NULL, FALSE, FALSE, "TemporizadorAlarme");
    if (hEventoTimerAlarme == 0) { printf("Erro na criacao do Evento <Temporizador Alarme>\n"); exit(-1); }
    hEventoTimerProcesso = CreateEvent(NULL, FALSE, FALSE, "TemporizadorProcesso");
    if (hEventoTimerProcesso == 0) { printf("Erro na criacao do Evento <Temporizador Processo>\n"); exit(-1); }
    hEventoTimerOtimizacao = CreateEvent(NULL, FALSE, FALSE, "TemporizadorOtimizacao");
    if (hEventoTimerOtimizacao == 0) { printf("Erro na criacao do Evento <Temporizador Otimizacao>\n"); exit(-1); }

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

    DWORD idProcesso, idOtimizacao, idTemporizacao;

    hThreadDadosProcesso = (HANDLE)_beginthreadex(
        NULL,
        0,
        (CAST_FUNCTION)TarefaCapturaDadosProcesso,
        NULL,
        0,
        (CAST_LPDWORD)&idProcesso
    );

    hThreadDadosOtimizacao = (HANDLE)_beginthreadex(
        NULL,
        0,
        (CAST_FUNCTION)TarefaCapturaDadosOtimizacao,
        NULL,
        0,
        (CAST_LPDWORD)&idOtimizacao
    );

    hThreadTemporizacao = (HANDLE)_beginthreadex(
        NULL,
        0,
        (CAST_FUNCTION)TemporizacaoTarefas,
        NULL,
        0,
        (CAST_LPDWORD)&idTemporizacao
    );

    // Vetor de Eventos a serem utilizados no WaitForMultipleObjects
    HANDLE hEspera[5], hEsperaTeclas[2];
    hEspera[0] = hEventoTeclaD;
    hEspera[1] = hEventoTeclaEsc;
    hEspera[2] = hEventoTimerAlarme;
    hEspera[3] = hEventoTimerProcesso;
    hEspera[4] = hEventoTimerOtimizacao;

    hEsperaTeclas[0] = hEventoTeclaD;
    hEsperaTeclas[1] = hEventoTeclaEsc;

    // Avisa que está pronta e espera as outras
    printf("*** Tarefa de Leitura Pronta para Executar! ***\n");
    SetEvent(hEventoLeituraPronta);
    WaitForSingleObject(hEventoTudoPronto, INFINITE);

    hMailslotLeitura = CreateFile(
        "\\\\.\\mailslot\\MailslotProcesso",
        GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    // Início da Rotina
    printf("*** Tarefa de Leitura Executando Rotina ***\n");

    do {
        retornoWait = WaitForMultipleObjects(5, hEspera, FALSE, INFINITE);
        printf(" >%lu< ", retornoWait);
        if (retornoWait == (WAIT_OBJECT_0 + 0)) {
            // Bloqueia-se esperando a tecla D novamente ou a tecla ESC
            printf("*** Tarefa de Leitura Bloqueada! ***\n");
            retornoWait = WaitForMultipleObjects(2, hEsperaTeclas, FALSE, INFINITE);
            printf("*** Tarefa de Leitura Desbloqueada! ***\n");
        }

        else if (retornoWait == (WAIT_OBJECT_0 + 2)) {
            GerarMensagem('A');      
        }
        else if (retornoWait == (WAIT_OBJECT_0 + 3)) {
            GerarMensagem('P');
        }
        else if (retornoWait == (WAIT_OBJECT_0 + 4)) {
            GerarMensagem('O');
        }

    } while (retornoWait != (WAIT_OBJECT_0 + 1));

    printf("*** Finalizando Tarefa de Leitura! ***\n");


    //CloseHandle(hListaCircular);
    CloseHandle(hEventoTeclaEsc);
    CloseHandle(hEventoTeclaD);
    CloseHandle(hEventoTeclaO);
    CloseHandle(hEventoTeclaP);
    CloseHandle(hMsgAlarmeDisponivel);
    CloseHandle(hMutexNSeq);
    CloseHandle(hMutexListaCircular);
    CloseHandle(hSemaforoLivres);
    CloseHandle(hSemaforoOcupadas);
    CloseHandle(hEventoTudoPronto);
    CloseHandle(hEventoLeituraPronta);
    CloseHandle(hEventoCapturaProcessoPronta);
    CloseHandle(hEventoCapturaOtimizacaoPronta);
    CloseHandle(hThreadDadosOtimizacao);
    CloseHandle(hThreadDadosProcesso);
    CloseHandle(hTemporizacaoPronta);
    CloseHandle(hThreadTemporizacao);
    CloseHandle(hTemporizadorAlarme);
    CloseHandle(hTemporizadorProcesso);
    CloseHandle(hTemporizadorOtimizacao);
    CloseHandle(hEventoTimerAlarme);
    CloseHandle(hEventoTimerOtimizacao);
    CloseHandle(hEventoTimerProcesso);
    CloseHandle(hMailslotLeitura);

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

    if (tipoMensagem == 'A') EnviarMensagemAlarme(mensagem);
    if (tipoMensagem == 'P' || tipoMensagem == 'O') DepositarMensagem(mensagem);

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

    strcpy_s(listaCircular[posicaoLivre], TAMANHO_MSG_MAX, mensagem);

    //for (int i = 0; i <= TAMANHO_MSG_MAX; i++) {
    //    
    //    listaCircular[posicaoLivre][i] = mensagem[i];
    //    if (mensagem[i] = '\0') break;  
    //}

    printf("mensagem %d depositada na lista com sucesso\n", posicaoLivre);
    posicaoLivre = (posicaoLivre + 1) % TAMANHO_LISTA_CIRCULAR;
    status = ReleaseSemaphore(hSemaforoOcupadas, 1, NULL);
    //CheckForError(status == 0);

    status = ReleaseMutex(hMutexListaCircular);
    //CheckForError(status == 0);

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

DWORD WINAPI TarefaCapturaDadosProcesso() {

    printf("*** Tarefa de Captura de Dados do Processo Iniciada ***\n");

    DWORD retornoWait;
    DWORD status;
    HANDLE hEspera[3], hEsperaTeclas[2];
    hEspera[0] = hEventoTeclaP;
    hEspera[1] = hEventoTeclaEsc;
    hEspera[2] = hSemaforoOcupadas;

    hEsperaTeclas[0] = hEventoTeclaP;
    hEsperaTeclas[1] = hEventoTeclaEsc;

    char mensagem[TAMANHO_MSG_MAX];

    //Cria Eventos para sinalizacao dos outros processos
    hMsgProcessoDisponivel = CreateEvent(NULL, FALSE, FALSE, "MsgProcessoDisponivel");
    if (hMsgProcessoDisponivel == 0) { printf("[CAPTURA] Erro na criacao do evento <MsgProcessoDisponivel>\n"); exit(-1); }

    printf("*** Tarefa de Captura de Dados do Processo Pronta para Executar! ***\n");
    SetEvent(hEventoCapturaProcessoPronta);
    WaitForSingleObject(hEventoTudoPronto, INFINITE);

    hMailslotCaptura = CreateFile(
        "\\\\.\\mailslot\\MailslotProcesso",
        GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    do {

        retornoWait = WaitForMultipleObjects(3, hEspera, FALSE, INFINITE);

        if (retornoWait == (WAIT_OBJECT_0 + 2)) {

            status = WaitForSingleObject(hMutexListaCircular, INFINITE);

            strcpy_s(mensagem, TAMANHO_MSG_MAX, listaCircular[posicaoOcupada]);

            if (mensagem[5] != '5') { // Não é dados de Processo
                status = ReleaseSemaphore(hSemaforoOcupadas, 1, NULL);
            }
            else {
                posicaoOcupada = (posicaoOcupada + 1) % TAMANHO_MSG_MAX;
                EnviarMensagemProcesso(mensagem);

                status = ReleaseSemaphore(hSemaforoLivres, 1, NULL);
            }

            status = ReleaseMutex(hMutexListaCircular);
            //CheckForError(status == 0);
        }

        if (retornoWait == (WAIT_OBJECT_0 + 0)) {
            // Bloqueia-se esperando a tecla P novamente
            printf("*** Tarefa de Captura de Dados do Proceso Bloqueada! ***\n");
            retornoWait = WaitForMultipleObjects(2, hEsperaTeclas, FALSE, INFINITE);
            //CheckForError(retornoWait == WAIT_OBJECT_0);
            printf("*** Tarefa de Captura de Dados do Proceso Desbloqueada! ***\n");
        }

    } while (retornoWait != (WAIT_OBJECT_0 + 1));

    printf("*** Tarefa de Captura de Dados do Processo Encerrando ***\n");

    CloseHandle(hMailslotCaptura);
    _endthreadex(0);
    return(0);
}

DWORD WINAPI TarefaCapturaDadosOtimizacao() {

    printf("*** Tarefa de Captura de Dados de Otimizacao Iniciada ***\n");

    //Cria Eventos para sinalizacao dos outros processos
    hMsgOtimizacaoDisponivel = CreateEvent(NULL, FALSE, FALSE, "MsgOtimizacaoDisponivel");
    if (hMsgOtimizacaoDisponivel == 0) { printf("[CAPTURA] Erro na criacao do evento <MsgOtimizacaoDisponivel>\n"); exit(-1); }


    DWORD dwBytesEscritos;

    hArquivoOtimizacao = CreateFile(
        "..\\..\\..\\..\\Arquivo Circular em Disco\\DadosDeOtimizacao.txt",
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, 
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);


    DWORD retornoWait;
    DWORD status;
    HANDLE hEspera[3], hEsperaTeclas[2];
    hEspera[0] = hEventoTeclaO;
    hEspera[1] = hEventoTeclaEsc;
    hEspera[2] = hSemaforoOcupadas;

    hEsperaTeclas[0] = hEventoTeclaO;
    hEsperaTeclas[1] = hEventoTeclaEsc;

    char mensagem[TAMANHO_MSG_MAX];

   


    printf("*** Tarefa de Captura de Dados de Otimizacao Pronta para Executar! ***\n");
    SetEvent(hEventoCapturaOtimizacaoPronta);
    WaitForSingleObject(hEventoTudoPronto, INFINITE);


    do {

        retornoWait = WaitForMultipleObjects(3, hEspera, FALSE, INFINITE);
        //CheckForError(retornoWait == WAIT_OBJECT_0);

        if (retornoWait == (WAIT_OBJECT_0 + 2)) {

            status = WaitForSingleObject(hMutexListaCircular, INFINITE);
            //CheckForError(status == 0);

            strcpy_s(mensagem, TAMANHO_MSG_MAX, listaCircular[posicaoOcupada]);
            posicaoOcupada = (posicaoOcupada + 1) % TAMANHO_MSG_MAX;
            EnviarMensagemOtimizacao(mensagem);

            status = ReleaseSemaphore(hSemaforoLivres, 1, NULL);
            //CheckForError(status == 0);

            status = ReleaseMutex(hMutexListaCircular);
            //CheckForError(status == 0);
        }

        if (retornoWait == (WAIT_OBJECT_0 + 0)) {
            // Bloqueia-se esperando a tecla P novamente
            printf("*** Tarefa de Captura de Dados de Otimizacao Bloqueada! ***\n");
            retornoWait = WaitForMultipleObjects(2, hEsperaTeclas, FALSE, INFINITE);
            //CheckForError(retornoWait == WAIT_OBJECT_0);
            printf("*** Tarefa de Captura de Dados de Otimizacao Desbloqueada! ***\n");
        }

    } while (retornoWait != (WAIT_OBJECT_0 + 1));



    printf("*** Tarefas de Captura de Dados de Otimizacao Encerrando ***\n");
    CloseHandle(hMsgOtimizacaoDisponivel);
    _endthreadex(0);
    return(0);
}

DWORD WINAPI TemporizacaoTarefas() {
    BOOL status;
    DWORD retornoWait;


    hFilaTemporizadores = CreateTimerQueue();
    if (hFilaTemporizadores == NULL) {
        printf("[LEITURA] Falha na criacao da Fila de Temporizadores! Codigo = %d)\n", GetLastError());
        return 0;
    }

    SetEvent(hTemporizacaoPronta);
    WaitForSingleObject(hEventoTudoPronto, INFINITE);
    
    // Temporizador das mensagens de dados de processo - a cada 500ms
    status = CreateTimerQueueTimer(&hTemporizadorProcesso, hFilaTemporizadores, (WAITORTIMERCALLBACK)MensagensProcessos,
        NULL, 0, 500, WT_EXECUTEDEFAULT);
    if (!status) {
        printf("[LEITURA] Erro no Temporizador de Dados de Processo! Codigo = %d)\n", GetLastError());
        return 0;
    }

    // Temporizador das mensagens de alarme - aleatório entre 1 e 5 segundos
    temporizadorAleatorio = RandFaixaInt(1000, 5000);
    status = CreateTimerQueueTimer(&hTemporizadorAlarme, hFilaTemporizadores, (WAITORTIMERCALLBACK)MensagensAlarme,
        NULL, 0, temporizadorAleatorio, WT_EXECUTEDEFAULT);
    if (!status) {
        printf("[LEITURA] Erro no Temporizador Alarme! Codigo = %d)\n", GetLastError());
        return 0;
    }

    // Temporizador das mensagens de Otimizacao - aleatório entre 1 e 5 segundos
    status = CreateTimerQueueTimer(&hTemporizadorOtimizacao, hFilaTemporizadores, (WAITORTIMERCALLBACK)MensagensOtimizacao,
        NULL, 0, temporizadorAleatorio, WT_EXECUTEDEFAULT);
    if (!status) {
        printf("[LEITURA] Erro no Temporizador de Otimizacao! Codigo = %d)\n", GetLastError());
        return 0;
    }

    WaitForSingleObject(hEventoTeclaEsc, INFINITE);

    if (!DeleteTimerQueueEx(hFilaTemporizadores, NULL))
        printf("[LEITURA] Falha em Deletar a Fila de Temporizadores! Codigo = %d\n", GetLastError());

    _endthreadex(0);
    return(0);
}

void EnviarMensagemAlarme(char* mensagem) {
    BOOL status;
    DWORD bytesEscritos;

    status = WriteFile(hMailslotLeitura, mensagem, TAMANHO_MSG_ALARME, &bytesEscritos, NULL);
    printf("alarme enviado\n");
    SetEvent(hMsgAlarmeDisponivel);

}

void EnviarMensagemProcesso(char* mensagem) {
    BOOL status;
    DWORD bytesEscritos;

    status = WriteFile(hMailslotCaptura, mensagem, TAMANHO_MSG_PROCESSO, &bytesEscritos, NULL);
    printf("mensagem de processo enviada\n");
    SetEvent(hMsgProcessoDisponivel);
}

void EnviarMensagemOtimizacao(char* mensagem) {
    BOOL status;
    DWORD bytesEscritos;
    status = WriteFile(hArquivoOtimizacao, mensagem, TAMANHO_MSG_OTIMIZACAO, &bytesEscritos, NULL);
}


void CALLBACK MensagensProcessos(PVOID, BOOLEAN) {
    SetEvent(hEventoTimerProcesso);
}

void CALLBACK MensagensAlarme(PVOID, BOOLEAN) {
    printf("teste");
    temporizadorAleatorio = RandFaixaInt(1000, 5000);
    SetEvent(hEventoTimerAlarme);
}

void CALLBACK MensagensOtimizacao(PVOID, BOOLEAN) {
    temporizadorAleatorio = RandFaixaInt(1000, 5000);
    SetEvent(hEventoTimerOtimizacao);
}
