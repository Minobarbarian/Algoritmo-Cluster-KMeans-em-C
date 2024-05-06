#include "Registro.h"

#include <synchapi.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define CAMINHO "C:/Users/Joao/Documents/UFRN/PC/inputOutput/"
#define TAMANHO_LINHA 100
#define TAMANHO_NOME_ARQUIVO 256
#define QUANTIDADE_THREADS 10

struct Argumentos_da_Thread {
    Registro** registros;
    size_t registrosSize;
    Registro** centroides;
    size_t centroidesSize;
    size_t comeco;
    size_t fim;
};

HANDLE mutexRegistros;
HANDLE mutexCentroides;

FILE* openAuto(const char* text, const char* modo);
FILE* leitorAutomatico(const char* text);
FILE* escritorAutomatico(const char* text);
DWORD WINAPI associarThreads(LPVOID arg);
void calcularNovosCentroides(Registro** registros, size_t registrosSize, Registro*** centroides, size_t centroidesSize);
void inicializarCentroides(Registro** centroides, int k);
void kMeans(int it, Registro** registros, size_t registrosSize, Registro** centroides, size_t centroidesSize);
void associarAosCentroides(Registro** registros, size_t registrosSize, Registro** centroides, size_t centroidesSize);
void gerarSaida(Registro** registros, size_t registrosSize);
void gerarResultado();
void limpar_mutexes();
void limpar_threads(HANDLE* thread);

int main() {

    FILE* arquivo = leitorAutomatico("entrada.txt");

    if(arquivo != NULL) {
        int k = 3;
        int iteracoes = 4;

        mutexRegistros = CreateMutex(NULL, FALSE, NULL);
        mutexCentroides = CreateMutex(NULL, FALSE, NULL);

        Registro** centroides = malloc(k * sizeof(Registro*));
        inicializarCentroides(centroides, k);

        size_t numRegistros = 0;
        char linha[TAMANHO_LINHA];
        while (fgets(linha, sizeof(linha), arquivo) != NULL) {
            numRegistros++;
        }

        rewind(arquivo);

        Registro** registros = malloc(numRegistros * sizeof(Registro*));

        size_t registrosSize = 0;
        while (fgets(linha, sizeof(linha), arquivo) != NULL) {
            registros[registrosSize] = transformaEmRegistro(linha);
            registrosSize++;
        }

        fclose(arquivo);

        printf("Leu!\n");
        
        kMeans(iteracoes, registros, registrosSize, centroides, k);

        free(centroides);

        printf("Associou!\n");

        gerarSaida(registros, registrosSize);

        free(registros);

        printf("Gravou!\n");

        gerarResultado();

    } else {
        printf("Não foi possível realizar a operação por problemas de acesso a entrada.txt.\n");
    }

    limpar_mutexes();
    return 0;
}

void inicializarCentroides(Registro** centroides, int k) {
    srand(time(NULL));
    for (int i = 0; i < k; i++) {
        centroides[i] = createClusteredRegistro((i+1), (double)rand() / RAND_MAX, (double)rand() / RAND_MAX);
    }
}

DWORD WINAPI associarThreads(LPVOID arg) {
    struct Argumentos_da_Thread* args = (struct Argumentos_da_Thread*)arg;
    
    for (size_t i = args->comeco; i < args->fim; i++) {
        associarCentroideMaisProximo(args->registros[i], args->centroides, args->centroidesSize);
    }

    return 0;
}

void kMeans(int it, Registro** registros, size_t registrosSize, Registro** centroides, size_t centroidesSize) {
    HANDLE threads[QUANTIDADE_THREADS];
    struct Argumentos_da_Thread threadArgs[QUANTIDADE_THREADS];
    size_t chunkSize = registrosSize / QUANTIDADE_THREADS;


    for (int i = 0; i < it; i++) {
        for(int t = 0; t < QUANTIDADE_THREADS; t++) {
            threadArgs[t].registros = registros;
            threadArgs[t].registrosSize = registrosSize;
            threadArgs[t].centroides = centroides;
            threadArgs[t].centroidesSize = centroidesSize;
            threadArgs[t].comeco = t * chunkSize;
            threadArgs[t].fim = (t == QUANTIDADE_THREADS - 1) ? registrosSize : (t + 1) * chunkSize;

            threads[t] = CreateThread(NULL, 0, associarThreads, (LPVOID)&threadArgs[t], 0, NULL);
            if (threads[t] == NULL) {
                fprintf(stderr, "Erro ao criar thread\n");

                for(int j = 0; j < t; j++) {
                    limpar_threads(&threads[j]);
                }
                return;
            }
        }

        WaitForMultipleObjects(QUANTIDADE_THREADS, threads, TRUE, INFINITE);
        
        calcularNovosCentroides(registros, registrosSize, &centroides, centroidesSize);
    }

    for (int t = 0; t < QUANTIDADE_THREADS; t++) {
        limpar_threads(&threads[t]);
    }
}

void associarAosCentroides(Registro** registros, size_t registrosSize, Registro** centroides, size_t centroidesSize) {
    WaitForSingleObject(mutexRegistros, INFINITE);
    for (size_t i = 0; i < registrosSize; i++) {
        associarCentroideMaisProximo(registros[i], centroides, centroidesSize);
    }
    ReleaseMutex(mutexRegistros);
}

void calcularNovosCentroides(Registro** registros, size_t registrosSize, Registro*** centroides, size_t centroidesSize) {
    WaitForSingleObject(mutexRegistros, INFINITE);
    WaitForSingleObject(mutexCentroides, INFINITE);

    Registro** novosCentroides = malloc(centroidesSize * sizeof(Registro*));
    if (novosCentroides == NULL) {
        fprintf(stderr, "Erro ao alocar memoria a novos centroides\n");
        ReleaseMutex(mutexRegistros);
        ReleaseMutex(mutexCentroides);
        return;
    } else {
        
    }

    for (size_t i = 0; i < centroidesSize; i++) {
        Registro* centroide = (*centroides)[i];
        double mediaX = 0, mediaY = 0;
        size_t registrosAssociadosSize = 0;

        for (size_t j = 0; j < registrosSize; j++) {
            if(getCentroideAssociado(registros[j]) == centroide) {
                mediaX += getX(registros[j]);
                mediaY += getY(registros[j]);
                registrosAssociadosSize++;
            }
        }
        
        if (registrosAssociadosSize > 0) {
            mediaX /= registrosAssociadosSize;
            mediaY /= registrosAssociadosSize;
            novosCentroides[i] = createClusteredRegistro(getCluster(centroide), mediaX, mediaY);
        } else {
            novosCentroides[i] = createClusteredRegistro(getCluster(centroide), getX(centroide), getY(centroide));
        }
    }

    ReleaseMutex(mutexRegistros);
    ReleaseMutex(mutexCentroides);

    free(*centroides);
    *centroides = novosCentroides;
}

void gerarSaida(Registro** registros, size_t registrosSize) {
    FILE* arquivoSaida = escritorAutomatico("saidaC.txt");

    if(arquivoSaida != NULL) {
        for (size_t i = 0; i < registrosSize; i++) {
            fprintf(arquivoSaida, "%lf %lf %d\n", getX(registros[i]), getY(registros[i]), getCluster(registros[i]));
        }

        fclose(arquivoSaida);
        printf("Registros associados gravados.\n");
    } else {
        printf("Não foi possível realizar a operação por problemas de acesso a saidaC.txt.\n");
    }
}

void gerarResultado() {
    FILE* arquivoEntrada = leitorAutomatico("saidaC.txt");

    char linha[TAMANHO_LINHA];

    int contadorMap[100] = {0};

    while (fgets(linha, sizeof(linha), arquivoEntrada) != NULL) {
        char* token = strtok(linha, " \t\n");
        int count = 0;
        while (token != NULL) {
            count++;
            if (count >= 3) {
                int numero;
                if (sscanf(token, "%d", &numero) == 1) {
                    contadorMap[numero]++;
                } else {
                    fprintf(stderr, "Erro decompondo linha: %s\n", linha);
                }
            }
            token = strtok(NULL, " \t\n");
        }
    }

    fclose(arquivoEntrada);

    FILE* arquivoSaida = escritorAutomatico("resultadoC.txt");

    for (int i = 0; i < 100; i++) {
        if (contadorMap[i] > 0) {
            fprintf(arquivoSaida, "Grupo [%dº]: %d ocorrências.\n", (i), contadorMap[i]);
        }
    }

    fclose(arquivoSaida);

}

FILE* openAuto(const char* text, const char* modo) {
    FILE* arquivo;
    char caminhoArquivo[TAMANHO_NOME_ARQUIVO];

    sprintf(caminhoArquivo,"%s%s", CAMINHO, text);

    arquivo = fopen(caminhoArquivo, modo);
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo");
        return NULL;
    }

    return arquivo;
}

FILE* leitorAutomatico(const char* text) {
    return openAuto(text, "r");
}

FILE* escritorAutomatico(const char* text) {
    return openAuto(text, "w");
}

void limpar_mutexes() {
    if (mutexRegistros != NULL) {
        CloseHandle(mutexRegistros);
        mutexRegistros = NULL;
    }
    if (mutexCentroides != NULL) {
        CloseHandle(mutexCentroides);
        mutexCentroides = NULL;
    }
}

void limpar_threads(HANDLE* thread) {
    if(*thread != NULL) {
        CloseHandle(*thread);
        *thread = NULL;
    }
}

