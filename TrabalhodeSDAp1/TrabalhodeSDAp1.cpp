// Trabalho de SDA
// 
//	Autores: Maria Luiza de Andrade e Matheus Paiva 
//
//	Cliente Socket
//
//


#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT  0x0400	// Necessário para ativar novas funções da versão 4
#define _WINSOCK_DEPRECATED_NO_WARNINGS //Uso necessário devido a função inet_addr

#include <windows.h>
#include <process.h>	
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <iostream>
#include <winsock2.h>
#pragma comment (lib, "ws2_32.lib") //Inclui a biblioteca winsocket 

using namespace std;
#include<string>
using std::string;
#include <ctype.h>
#include <iostream>

// Casting para terceiro e sexto parâmetros da função _beginthreadex
typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);
typedef unsigned* CAST_LPDWORD;

//--------------- Declarações relacionadas a Criação das mensagens do tipo 11, 33 e 99 --------------- //

int NSEQ = 1;
//lista na memomiara ram
#define TAM_LIST 200
int indice = 0;
string  LISTA[TAM_LIST]; //Lista final de envio 

//Variaveis Socket
WSADATA     wsaData;
SOCKET      s;
SOCKADDR_IN ServerAddr;
int statusSocket,port;
char *ipaddr;// ipaddr é a comunicação IP e o port é o número da porta - Um deles é o 4045

typedef struct TIPO11 {
	int nseq = 1;
	int tipo = 11;
	int taxa = 0;
	float potencia = 0.0;
	float tempTrans = 0.0;
	float tempRoda = 0.0;

}TIPO11; // definição do tipo 11

typedef struct TIPO33 {
	int nseq = 1;
	int tipo = 33;
}TIPO33; // definição do tipo 33

typedef struct TIPO99 {
	int nseq = 1;
	int tipo = 99;
}TIPO99; // definição do tipo 99


// Threads de Gerenciamento 
DWORD WINAPI CriaTipo11(LPVOID);	// declaração da thread  que  gerencia a Criancão de mensagens do tipo 11
DWORD WINAPI CriaTipo33(LPVOID);	// declaração da thread  que  gerencia a Criancão de mensagens do tipo 33
DWORD WINAPI CriaTipo99(LPVOID);	// declaração da thread  que  gerencia a Criancão de mensagens do tipo 99

// Funções de Criação das Mensagens
TIPO11  novaMensagem11();
TIPO33  novaMensagem33();
TIPO99  novaMensagem99();

//variaveis que informa se exite mensagens do tipo 11, do tipo 33 ou do tipo 99 na lista
int contP11 = 0;
int contP33 = 0;
int contP99 = 0;

// Elemenstos de sincronização
HANDLE hMutexNSEQ; //  handle do mutex que protege a variavel nseq
HANDLE hSemLISTAcheia, hSemLISTAvazia; //handle do semaforo que verifica se a lista está cheia ou vazia; O lista cheia começa com 0 e vai até o tamanho maximo e o lista vazia vai do tamanho maximo até 0 
HANDLE hMutexPRODUTOR, hMutexCOSNSUMIDOR; // handle do mutex que bloqueia o produtor e o consumidor
HANDLE hMutex11, hMutex33, hMutex99; // Mutex que protegem as quantidades de mensagem

// ----------------------------------------------------------------------------------------------------- //

//	
DWORD WINAPI EnviaMensagem(LPVOID);
DWORD WINAPI RecebeMensagem(LPVOID);
void ConexaoServidor();

int main()
{
	SetConsoleTitle(L"Trabalho de SDA - Principal");
    std::cout << "Em obras \n";
	
	//variaveis e Handles

	HANDLE hTarefas[5]; // handle para todas as tarefas
	DWORD dwCriacao11, dwCriacao33, dwCriacao99,dwEnvio,dwRecebe;
	DWORD dwExitCode = 0;
	DWORD dwRet;
	int aux, j,status,Tecla = 0;
	LARGE_INTEGER Preset;

	

	//Mutex
	hMutexNSEQ = CreateMutex(NULL, FALSE, L"ProtegeNSEQ");
	hMutexPRODUTOR = CreateMutex(NULL, FALSE, L"ProtegePRODUTOR");
	hMutexCOSNSUMIDOR = CreateMutex(NULL, FALSE, L"ProtegeCOSNSUMIDOR");
	hMutex11 = CreateMutex(NULL, FALSE, L"Protege11");
	hMutex33 = CreateMutex(NULL, FALSE, L"Protege33");
	hMutex99 = CreateMutex(NULL, FALSE, L"Protege99");
	hSemLISTAcheia = CreateSemaphore(NULL, 0, TAM_LIST, L"SemLISTAcheia");
	hSemLISTAvazia = CreateSemaphore(NULL, TAM_LIST, TAM_LIST, L"SemLISTAvazia");

	// Threads de criação de mensagens 
	hTarefas[0] = (HANDLE)_beginthreadex(NULL, 0, (CAST_FUNCTION)CriaTipo11, NULL, 0, (CAST_LPDWORD)&dwCriacao11);
	if (hTarefas[0]) 	cout << "Thread de criacao de mensagens do tipo 11 criada com Id=" << dwCriacao11 << "\n";
	hTarefas[1] = (HANDLE)_beginthreadex(NULL, 0, (CAST_FUNCTION)CriaTipo33, NULL, 0, (CAST_LPDWORD)&dwCriacao33);
	if (hTarefas[1]) 	cout << "Thread de criacao de mensagens do tipo 33 criada com Id =" << dwCriacao33 << "\n";
	hTarefas[2] = (HANDLE)_beginthreadex(NULL, 0, (CAST_FUNCTION)CriaTipo99, NULL, 0, (CAST_LPDWORD)&dwCriacao99);
	if (hTarefas[2]) 	cout << "Thread de criacao de mensagens do tipo 99 criada com Id=" << dwCriacao99 << "\n";

	// Threads de Envio de mensagens 
	hTarefas[3] = (HANDLE)_beginthreadex(NULL, 0, (CAST_FUNCTION)EnviaMensagem, NULL, 0, (CAST_LPDWORD)&dwEnvio);
	if (hTarefas[3]) 	cout << "Thread de envio de mensagens criada com Id=" << dwEnvio << "\n";

	// Threads de Recebimento de mensagens 
	hTarefas[4] = (HANDLE)_beginthreadex(NULL, 0, (CAST_FUNCTION)RecebeMensagem, NULL, 0, (CAST_LPDWORD)&dwRecebe);
	if (hTarefas[4]) 	cout << "Thread de recebimento de mensagens criada com Id=" << dwRecebe << "\n";

	// Inicializa Winsock versão 2.2
	statusSocket = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (statusSocket != 0) {
		printf("Falha na inicializacao do Winsock 2! Erro  = %d\n", WSAGetLastError());
		WSACleanup();
		exit(0);
	}

	// Cria um novo socket para estabelecer a conexão.
	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) {
		statusSocket = WSAGetLastError();
		if (statusSocket == WSAENETDOWN)
			printf("Rede ou servidor de sockets inacessíveis!\n");
		else
			printf("Falha na rede: codigo de erro = %d\n", statusSocket);
		WSACleanup();
		exit(0);
	}
	// A conexão com o servidor acho q tem q estar em um while ou alg assim
	// Inicializa a estrutura SOCKADDR_IN que será utilizada para a conexão ao servidor.
	
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(port);//port é a porta de comunicação
	ServerAddr.sin_addr.s_addr = inet_addr(ipaddr);//IPaddr é o endereço IP que seria passado por linha de comando

	// Estabelece a conexão com o servidor
	statusSocket = connect(s, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr));
	if (statusSocket == SOCKET_ERROR) {
		printf("Falha na conexao ao servidor ! Erro  = %d\n", WSAGetLastError());
		WSACleanup();
		exit(0);
	}



	dwRet = WaitForMultipleObjects(5, hTarefas, TRUE, INFINITE);
	
	for (j = 0; j < 5; j++) {
		status = GetExitCodeThread(&hTarefas[j], &dwExitCode);
		cout << "thread " << j << " terminou: codigo " << dwExitCode << "\n";
		CloseHandle(hTarefas[j]);	// apaga referência ao objeto
	}  



	CloseHandle(hMutexNSEQ);
	CloseHandle(hMutexPRODUTOR);
	CloseHandle(hMutexCOSNSUMIDOR);
	CloseHandle(hMutex11);
	CloseHandle(hMutex33);
	CloseHandle(hMutex99);
	CloseHandle(hSemLISTAcheia);
	CloseHandle(hSemLISTAvazia);

	cout << "\nAcione uma tecla para terminar\n";
	Tecla = _getch(); // // Pare aqui, caso não esteja executando no ambiente MDS

	return EXIT_SUCCESS;

}

DWORD WINAPI CriaTipo11(LPVOID index) {
	_endthreadex((DWORD)index);
	return(0);
}

DWORD WINAPI CriaTipo33(LPVOID index) {
	_endthreadex((DWORD)index);
	return(0);
}

DWORD WINAPI CriaTipo99(LPVOID index) {
	_endthreadex((DWORD)index);
	return(0);
}

DWORD WINAPI EnviaMensagem(LPVOID index) {
	_endthreadex((DWORD)index);
	return(0);
}

DWORD WINAPI RecebeMensagem(LPVOID index) {
	do {
		//statusSocket = recv(s, buf, TAMBUF, 0);
		if (statusSocket > 0) {
			//strncpy_s(msg, TAMBUF + 1, buf, statusSocket);
			//printf("Hora corrente no servidor %s = %s\n", msg);
			//memset(msg, TAMBUF + 1, 0);
		}
		else if (statusSocket == 0)
			printf("Fim da mensagem recebida do servidor.\n");
		else
			printf("Falha na recepcao de dados do servidor ! Erro  = %d\n", WSAGetLastError());
	} while (statusSocket > 0); //Ainda não deve sair, precisamos tentar conectar com o servidor novamente 
	//Precisamos ver como vai ser pra identificar uqal mensagem ta chegando -> apos o número sequencial (tamanho de um inteiro) temos um S
	//Em seguida o tipo da mensagem, ai podemos direcionar de acordo com o tratamento . 

	_endthreadex((DWORD)index);
	return(0);
}

void ConexaoServidor() {

}


