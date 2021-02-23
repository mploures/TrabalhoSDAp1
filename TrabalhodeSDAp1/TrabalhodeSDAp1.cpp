// Trabalho de SDA
// 
//	Autores: Maria Luiza de Andrade e Matheus Paiva 
//
//	Cliente Socket
//
//

#define _CRT_SECURE_NO_WARNINGS 
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

#define	ESC 	0x1B	//Tecla para encerrar o programa
#define p       0x70    // DEfine tecla do evento p

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
int statusSocket,port = 3445;
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

// Elementos de sincronização
HANDLE hMutexNSEQ; //  handle do mutex que protege a variavel nseq
HANDLE hSemLISTAcheia, hSemLISTAvazia; //handle do semaforo que verifica se a lista está cheia ou vazia; O lista cheia começa com 0 e vai até o tamanho maximo e o lista vazia vai do tamanho maximo até 0 
HANDLE hMutexPRODUTOR, hMutexCOSNSUMIDOR; // handle do mutex que bloqueia o produtor e o consumidor
HANDLE hMutex11, hMutex33, hMutex99; // Mutex que protegem as quantidades de mensagem
HANDLE hMutexINDICE; 
HANDLE hEventoESC,hEventoACK, hEventoP;
HANDLE hTimer;
// ----------------------------------------------------------------------------------------------------- //

//	
DWORD WINAPI EnviaMensagem(LPVOID);
DWORD WINAPI RecebeMensagem(LPVOID);
int CheckSocketError(int status, HANDLE hOut);
void ConexaoServidor();
void EnviaSocket(char* m);

int main(int argc, int argv[])
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

	

	//Mutex e semaforos
	hMutexNSEQ = CreateMutex(NULL, FALSE, L"ProtegeNSEQ");
	hMutexPRODUTOR = CreateMutex(NULL, FALSE, L"ProtegePRODUTOR");
	hMutexCOSNSUMIDOR = CreateMutex(NULL, FALSE, L"ProtegeCOSNSUMIDOR");
	hMutex11 = CreateMutex(NULL, FALSE, L"Protege11");
	hMutex33 = CreateMutex(NULL, FALSE, L"Protege33");
	hMutex99 = CreateMutex(NULL, FALSE, L"Protege99");
	hSemLISTAcheia = CreateSemaphore(NULL, 0, TAM_LIST, L"SemLISTAcheia");
	hSemLISTAvazia = CreateSemaphore(NULL, TAM_LIST, TAM_LIST, L"SemLISTAvazia");

	// Eventos
	hEventoESC = CreateEvent(NULL, TRUE, FALSE, L"EventoESC"); // reset manual
	hEventoACK= CreateEvent(NULL, FALSE, FALSE, L"EventoACK"); // reset automatico
	hEventoP = CreateEvent(NULL, FALSE, FALSE, L"EventoP"); // reset automatico

    //Timer
	hTimer = CreateWaitableTimer(NULL, FALSE, L"Timer");

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


	// ajuste do timer
	Preset.QuadPart = -(20000 * 500);
	status = SetWaitableTimer(hTimer, &Preset, 500, NULL, NULL, FALSE);

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

	do {
		cout << "\n Tecle <p> para simular o evento de solitacao de mensagem \n <ESC> para sair \n";
		Tecla = _getch();

		if (Tecla == p) {
			status = SetEvent(hEventoP);
			cout << "\n Evento P ocorreu \n";
			Tecla = 0;
		}
		else if (Tecla == ESC) {
			status = SetEvent(hEventoESC);
			cout << "\n Evento ESC ocorreu \n";
		}
		else {
			cout << "\n Evento nao cadastrado \n";
			Tecla = 0;
		}

	
	} while (Tecla != ESC);


	dwRet = WaitForMultipleObjects(5, hTarefas, TRUE, INFINITE);
	
	for (j = 0; j < 5; j++) {
		status = GetExitCodeThread(&hTarefas[j], &dwExitCode);
		cout << "thread " << j << " terminou: codigo " << dwExitCode << "\n";
		CloseHandle(hTarefas[j]);	// apaga referência ao objeto
	}  
	ResetEvent(hEventoESC);


	CloseHandle(hMutexNSEQ);
	CloseHandle(hMutexPRODUTOR);
	CloseHandle(hMutexCOSNSUMIDOR);
	CloseHandle(hMutex11);
	CloseHandle(hMutex33);
	CloseHandle(hMutex99);
	CloseHandle(hSemLISTAcheia);
	CloseHandle(hSemLISTAvazia);


	CloseHandle(hTimer);
	CloseHandle(hEventoACK);
	CloseHandle(hEventoESC);

	cout << "\nAcione uma tecla para terminar\n";
	Tecla = _getch(); // // Pare aqui, caso não esteja executando no ambiente MDS

	return EXIT_SUCCESS;

}

DWORD WINAPI CriaTipo11(LPVOID index) {
	// Variaveis de controle de thread e elementos de sincronização
	int status;
	DWORD ret;
	DWORD dwRet;
	//Variaveis de construção da mensagem
	string aux = "erro";
	char Print[5];
	TIPO11 m1;
	int tipo;

	HANDLE hEventos[2];

	hEventos[0] = hEventoESC;
	hEventos[1] = hTimer;

	//Rotina Principal
	do {
		// Espera a ocorrencia de um evento
		ret = WaitForMultipleObjects(2, hEventos, FALSE, INFINITE);

		// retona qual a posição do evento que ocorreu 0 para ESC e 1 para o Temporizador
		tipo = ret - WAIT_OBJECT_0;

		if (tipo == 1) {

			// -------------recebe a mensagem do CLP-------------//
			dwRet = WaitForSingleObject(hMutexNSEQ, INFINITE); // mutex pra proteger a variavel NSEQ
			m1 = novaMensagem11();
			status = ReleaseMutex(hMutexNSEQ);

			// -------------Produz Mensagem-------------//

			status = sprintf(Print, "%05d", m1.nseq);
			aux = Print;
			aux += "$";
			aux += to_string(m1.tipo) + "$";
			aux += to_string(m1.taxa) + "$";
			aux += to_string(m1.potencia) + "$";
			aux += to_string(m1.tempTrans) + "$";
			aux += to_string(m1.tempRoda);




			//-------------Tenta guardar o dado produzido-------------//

			dwRet = WaitForSingleObject(hMutexPRODUTOR, INFINITE);  //Garante um produtor por vez 

			dwRet = WaitForSingleObject(hSemLISTAvazia, INFINITE); // Aguarda um espaço vazio

			dwRet = WaitForSingleObject(hMutexINDICE, INFINITE);//atualiza o indice


			indice = (indice + 1) % TAM_LIST;
			LISTA[indice] = aux; // Armazena a mensagem na lista

			dwRet = WaitForSingleObject(hMutex11, INFINITE);
			contP11++; //atualiza o numero de produtos tipo 11 na lista			
			status = ReleaseMutex(hMutex11);

			status = ReleaseMutex(hMutexINDICE);
			status = ReleaseSemaphore(hSemLISTAcheia, 1, NULL); // Sinaliza que existe uma mensagem
			status = ReleaseMutex(hMutexPRODUTOR); // Libera Mutex
		}
		

	} while (tipo != 0);


	//Encerramento da thread

	_endthreadex((DWORD)index);
	return(0);
}

DWORD WINAPI CriaTipo33(LPVOID index) {
	// Variaveis de controle de thread e elementos de sincronização
	int status;
	DWORD ret;
	DWORD dwRet;
	//Variaveis de construção da mensagem
	string aux = "erro";
	char Print[5];
	TIPO33 m3;
	int tipo;

	HANDLE hEventos[2];

	hEventos[0] = hEventoESC;
	hEventos[1] = hEventoP;

	//Rotina Principal
	do {
		// Espera a ocorrencia de um evento
		ret = WaitForMultipleObjects(2, hEventos, FALSE, INFINITE);

		// retona qual a posição do evento que ocorreu 0 para ESC e 1 para o evento de Solicitação
		tipo = ret - WAIT_OBJECT_0;

		if (tipo == 1) {

			// -------------recebe a mensagem do CLP-------------//
			dwRet = WaitForSingleObject(hMutexNSEQ, INFINITE); // mutex pra proteger a variavel NSEQ
			m3 = novaMensagem33();
			status = ReleaseMutex(hMutexNSEQ);

			// -------------Produz Mensagem-------------//

			status = sprintf(Print, "%05d", m3.nseq);
			aux = Print;
			aux += "$";
			aux += to_string(m3.tipo);




			//-------------Tenta guardar o dado produzido-------------//

			dwRet = WaitForSingleObject(hMutexPRODUTOR, INFINITE);  //Garante um produtor por vez 

			dwRet = WaitForSingleObject(hSemLISTAvazia, INFINITE); // Aguarda um espaço vazio

			dwRet = WaitForSingleObject(hMutexINDICE, INFINITE);//atualiza o indice


			indice = (indice + 1) % TAM_LIST;
			LISTA[indice] = aux; // Armazena a mensagem na lista

			dwRet = WaitForSingleObject(hMutex33, INFINITE);
			contP33++; //atualiza o numero de produtos tipo 33 na lista			
			status = ReleaseMutex(hMutex33);

			status = ReleaseMutex(hMutexINDICE);
			status = ReleaseSemaphore(hSemLISTAcheia, 1, NULL); // Sinaliza que existe uma mensagem
			status = ReleaseMutex(hMutexPRODUTOR); // Libera Mutex
		}


	} while (tipo != 0);


	//Encerramento da thread

	_endthreadex((DWORD)index);
	return(0);
}

DWORD WINAPI CriaTipo99(LPVOID index) {
	// Variaveis de controle de thread e elementos de sincronização
	int status;
	DWORD ret;
	DWORD dwRet;
	//Variaveis de construção da mensagem
	string aux = "erro";
	char Print[5];
	TIPO99 m9;
	int tipo;

	HANDLE hEventos[2];

	hEventos[0] = hEventoESC;
	hEventos[1] = hEventoACK;

	//Rotina Principal
	do {
		// Espera a ocorrencia de um evento
		ret = WaitForMultipleObjects(2, hEventos, FALSE, INFINITE);

		// retona qual a posição do evento que ocorreu 0 para ESC e 1 para o evento de Recebimento de mensgam de dados
		tipo = ret - WAIT_OBJECT_0;

		if (tipo == 1) {

			// -------------recebe a mensagem do CLP-------------//
			dwRet = WaitForSingleObject(hMutexNSEQ, INFINITE); // mutex pra proteger a variavel NSEQ
			m9 = novaMensagem99();
			status = ReleaseMutex(hMutexNSEQ);

			// -------------Produz Mensagem-------------//

			status = sprintf(Print, "%05d", m9.nseq);
			aux = Print;
			aux += "$";
			aux += to_string(m9.tipo);




			//-------------Tenta guardar o dado produzido-------------//

			dwRet = WaitForSingleObject(hMutexPRODUTOR, INFINITE);  //Garante um produtor por vez 

			dwRet = WaitForSingleObject(hSemLISTAvazia, INFINITE); // Aguarda um espaço vazio

			dwRet = WaitForSingleObject(hMutexINDICE, INFINITE);//atualiza o indice


			indice = (indice + 1) % TAM_LIST;
			LISTA[indice] = aux; // Armazena a mensagem na lista

			dwRet = WaitForSingleObject(hMutex99, INFINITE);
			contP99++; //atualiza o numero de produtos tipo 99 na lista			
			status = ReleaseMutex(hMutex99);

			status = ReleaseMutex(hMutexINDICE);
			status = ReleaseSemaphore(hSemLISTAcheia, 1, NULL); // Sinaliza que existe uma mensagem
			status = ReleaseMutex(hMutexPRODUTOR); // Libera Mutex
		}


	} while (tipo != 0);


	//Encerramento da thread

	_endthreadex((DWORD)index);
	return(0);
}


TIPO11  novaMensagem11() {
	TIPO11 m1;
	int aux = rand() % 9999;

	// Atribui o valor de NSEQ e o atualiza
	m1.nseq = NSEQ;
	NSEQ++;
	if (NSEQ == 99999) {
		NSEQ = 1;
	}
	// A variavel NSEQ é protegida antes de chamar a função

	m1.potencia= (float)aux / 10;
	aux = rand() % 9999;
	m1.tempRoda= (float)aux / 10;
	aux = rand() % 9999;
	m1.tempTrans= (float)aux / 10;
	aux = rand() % 999999;
	m1.taxa = aux;

	return m1;

}

TIPO33  novaMensagem33() {
	TIPO33 m3;

	// Atribui o valor de NSEQ e o atualiza
	m3.nseq = NSEQ;
	NSEQ++;
	if (NSEQ == 99999) {
		NSEQ = 1;
	}
	// A variavel NSEQ é protegida antes de chamar a função

	return m3;

}

TIPO99  novaMensagem99() {
	TIPO99 m9;

	// Atribui o valor de NSEQ e o atualiza
	m9.nseq = NSEQ;
	NSEQ++;
	if (NSEQ == 99999) {
		NSEQ = 1;
	}
	// A variavel NSEQ é protegida antes de chamar a função

	return m9;
}



DWORD WINAPI EnviaMensagem(LPVOID index) {
	BOOL status;
	DWORD ret;
	DWORD dwRet;

	//Variaveis de consumir da mensagem
	string m;
	string msg;
	int j;
	int indexm;
	int AJUDA;

	//Variaveis que gerem a parte de evento da thread
	int tipo;    // tipo do evento
	HANDLE hEventos;


	// Variaveis Para o envio de Mensagem atraves do Socket
	char buf[100];

	hEventos= hEventoESC;


	do {
		// Espera a ocorrencia de um evento; para não travar nessa linha o time_out deve ser  diferente de INFINITE
		ret = WaitForSingleObject(hEventos,100);

		tipo = ret - WAIT_OBJECT_0;// retona qual a posição do evento que ocorreu 0 para ESC e 1 para E

		dwRet = WaitForSingleObject(hMutex11, INFINITE);
		AJUDA = contP11;
		ReleaseMutex(hMutex11);

		dwRet = WaitForSingleObject(hMutex33, INFINITE);
		AJUDA = AJUDA + contP33;
		ReleaseMutex(hMutex33);

		dwRet = WaitForSingleObject(hMutex99, INFINITE);
		AJUDA = AJUDA + contP99;
		ReleaseMutex(hMutex99);



		if (AJUDA >=1) {

			//-------------Tenta Acessar o dado na lista-------------//

			dwRet = WaitForSingleObject(hMutexCOSNSUMIDOR, INFINITE);  //Garante um consumidor por vez 
			dwRet = WaitForSingleObject(hSemLISTAcheia, INFINITE); // Aguarda um espaço preenchido;

			WaitForSingleObject(hMutexINDICE, INFINITE);

			// Encontra a Primeira Mensagem  na lista
			for (j = 0; j < indice; j++) {
				msg = LISTA[j];
				if (msg != "") {
					indexm = j;
					break;

				}
			}
			msg = LISTA[indexm];
			LISTA[indexm]="";
			ReleaseMutex(hMutexINDICE);
			
			if (msg != "") {

				if (msg.substr(6, 2) == "11") {

					cout << msg << "\n";
					// Grava a mensagem em um vetor de caracter
					for (j = 0; j < msg.size(); j++) {
						buf[j] = msg[j];
					}

					EnviaSocket(buf);



					dwRet = WaitForSingleObject(hMutex11, INFINITE);
					contP11--; // Atualiza o numero de produtos tipo 11
					status = ReleaseMutex(hMutex11);
					status = ReleaseSemaphore(hSemLISTAvazia, 1, NULL); // Sinaliza que uma mensagem foi lida 
					status = ReleaseMutex(hMutexCOSNSUMIDOR); // Libera Mutex
				}
				else if (msg.substr(6, 2) == "33") {

					cout << msg << "\n";
					// Grava a mensagem em um vetor de caracter
					for (j = 0; j < msg.size(); j++) {
						buf[j] = msg[j];
					}

					EnviaSocket(buf);



					dwRet = WaitForSingleObject(hMutex33, INFINITE);
					contP33--; // Atualiza o numero de produtos tipo 33
					status = ReleaseMutex(hMutex33);
					status = ReleaseSemaphore(hSemLISTAvazia, 1, NULL); // Sinaliza que uma mensagem foi lida 
					status = ReleaseMutex(hMutexCOSNSUMIDOR); // Libera Mutex
				}
				else if (msg.substr(6, 2) == "99") {

					cout << msg << "\n";
					// Grava a mensagem em um vetor de caracter
					for (j = 0; j < msg.size(); j++) {
						buf[j] = msg[j];
					}

					EnviaSocket(buf);

					dwRet = WaitForSingleObject(hMutex99, INFINITE);
					contP99--; // Atualiza o numero de produtos tipo 99
					status = ReleaseMutex(hMutex99);
					status = ReleaseSemaphore(hSemLISTAvazia, 1, NULL); // Sinaliza que uma mensagem foi lida 
					status = ReleaseMutex(hMutexCOSNSUMIDOR); // Libera Mutex
				}
				else{
					status = ReleaseMutex(hMutexCOSNSUMIDOR);
					status = ReleaseSemaphore(hSemLISTAcheia, 1, NULL);

				}
			}
			else{
				status = ReleaseMutex(hMutexCOSNSUMIDOR);
				status = ReleaseSemaphore(hSemLISTAcheia, 1, NULL);
			}





		}


	} while (tipo != 0);

	

	_endthreadex((DWORD)index);
	return (0);
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

int CheckSocketError(int status, HANDLE hOut) {//modificar e testar se vale a pena utilizar essa função 
	/*
	int erro;

	if (status == SOCKET_ERROR) {
		SetConsoleTextAttribute(hOut, HLRED);
		erro = WSAGetLastError();
		if (erro == WSAEWOULDBLOCK) {
			printf("Timeout na operacao de RECV! errno = %d - reiniciando...\n\n", erro);
			return(-1); // acarreta reinício da espera de mensagens no programa principal
		}
		else if (erro == WSAECONNABORTED) {
			printf("Conexao abortada pelo cliente TCP - reiniciando...\n\n");
			return(-1); // acarreta reinício da espera de mensagens no programa principal
		}
		else {
			printf("Erro de conexao! valor = %d\n\n", erro);
			return (-2); // acarreta encerramento do programa principal
		}
	}
	else if (status == 0) {
		//Este caso indica que a conexão foi encerrada suavemente ("gracefully")
		printf("Conexao com cliente TCP encerrada prematuramente! status = %d\n\n", status);
		return(-1); // acarreta reinício da espera de mensagens no programa principal
	}
	else return(0);*/
}

