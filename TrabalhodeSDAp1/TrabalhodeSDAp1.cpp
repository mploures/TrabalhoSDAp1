// Trabalho de SDA
// 
//	Autores: Maria Luiza de Andrade e Matheus Paiva 
//
//	Cliente Sokcet
//
//


#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT  0x0400	// Necessário para ativar novas funções da versão 4


#include <windows.h>
#include <process.h>	
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <iostream>
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
string  LISTA[TAM_LIST];

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

// Elemenstos de sincronização
HANDLE hMutexNseq; //  handle do mutex que protege a variavel nseq
HANDLE hSemLISTAcheia, hSemLISTAvazia; //handle do semaforo que verifica se a lista está cheia ou vazia; O lista cheia começa com 0 e vai até o tamanho maximo e o lista vazia vai do tamanho maximo até 0 
HANDLE hMutexPRODUTOR, hMutexCOSNSUMIDOR; // handle do mutex que bloqueia o produtor e o consumidor

// ----------------------------------------------------------------------------------------------------- //

int main()
{
    std::cout << "Mudanca para teste!\n";
}

// Executar programa: Ctrl + F5 ou Menu Depurar > Iniciar Sem Depuração
// Depurar programa: F5 ou menu Depurar > Iniciar Depuração

// Dicas para Começar: 
//   1. Use a janela do Gerenciador de Soluções para adicionar/gerenciar arquivos
//   2. Use a janela do Team Explorer para conectar-se ao controle do código-fonte
//   3. Use a janela de Saída para ver mensagens de saída do build e outras mensagens
//   4. Use a janela Lista de Erros para exibir erros
//   5. Ir Para o Projeto > Adicionar Novo Item para criar novos arquivos de código, ou Projeto > Adicionar Item Existente para adicionar arquivos de código existentes ao projeto
//   6. No futuro, para abrir este projeto novamente, vá para Arquivo > Abrir > Projeto e selecione o arquivo. sln
