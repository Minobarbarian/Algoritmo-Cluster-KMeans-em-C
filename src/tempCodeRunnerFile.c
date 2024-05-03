#include "Registro.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#define CAMINHO "C:/Users/Joao/Documents/UFRN/PC/inputOutput/"
#define TAMANHO_LINHA 100
#define TAMANHO_NOME_ARQUIVO 256

FILE* openAuto(const char* text, char c);
FILE* leitorAutomatico(const char* text);
FILE* escritorAutomatico(const char* text);
void inicializarCentroides(Registro** centroides, int k);
void kMeans(int it, Registro** registros, size_t registrosSize, Registro** centroides, size_t centroidesSize);
void associarAosCentroides(Registro** registros, size_t registrosSize, Registro** centroides, size_t centroidesSize);
Registro** calcularNovosCentroides(Registro** registros, size_t registrosSize, Registro** centroides, size_t centroidesSize);
void gerarSaida(Registro** registros, size_t registrosSize);
void gerarResultado();

int main() {
    FILE* arquivo = leitorAutomatico("entrada.txt");

    if(arquivo != NULL) {
        int k = 3;
        int iteracoes = 4;

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

        printf("Leu e Alocou!\n");

        kMeans(iteracoes, registros, registrosSize, centroides, k);

        free(centroides);

        printf("Associou!\n");

        gerarSaida(registros, registrosSize);

        free(registros);

        printf("Gravou!\n");

        //gerarResultado();

    } else {
        printf("Não foi possível realizar a operação por problemas de acesso a entrada.txt.\n");
    }
    return 0;
}

void inicializarCentroides(Registro** centroides, int k) {
    srand(time(NULL));
    for (int i = 0; i < k; i++) {
        centroides[i] = createClusteredRegistro((i+1), (double)rand() / RAND_MAX, (double)rand() / RAND_MAX);
    }
}

void kMeans(int it, Registro** registros, size_t registrosSize, Registro** centroides, size_t centroidesSize) {
    for (int i = 0; i < it; i++) {
        associarAosCentroides(registros, registrosSize, centroides, centroidesSize);
        Registro** novosCentroides = calcularNovosCentroides(registros, registrosSize, centroides, centroidesSize);
        if (novosCentroides != NULL) {
            free(centroides);
            centroides = novosCentroides;
        }
    }
}

void associarAosCentroides(Registro** registros, size_t registrosSize, Registro** centroides, size_t centroidesSize) {
    for (size_t i = 0; i < registrosSize; i++) {
        associarCentroideMaisProximo(registros[i], centroides, centroidesSize);
    }
}

Registro** calcularNovosCentroides(Registro** registros, size_t registrosSize, Registro** centroides, size_t centroidesSize) {
    Registro** novosCentroides = malloc(centroidesSize * sizeof(Registro*));

    for (size_t i = 0; i < centroidesSize; i++) {
        Registro* centroide = centroides[i];

        
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
        
        //Registro** registrosAssociados = malloc(registrosSize * sizeof(Registro*));
       // size_t registrosAssociadosSize = 0;

        //for (size_t j = 0; j < registrosSize; j++) {
        //    if (getCentroideAssociado(registros[j]) == centroide) {
        //        registrosAssociados[registrosAssociadosSize++] = registros[j];
        //    }
        //}

       // if (registrosAssociadosSize > 0) {
        //    double mediaX = 0, mediaY = 0;

        //    for (size_t j = 0; j < registrosAssociadosSize; j++) {
        //        mediaX += getX(registrosAssociados[j]);
        //        mediaY += getY(registrosAssociados[j]);
        //    }

        //    mediaX /= registrosAssociadosSize;
        //    mediaY /= registrosAssociadosSize;

        //    novosCentroides[i] = createClusteredRegistro(getCluster(centroide), mediaX, mediaY);
        //} else {
        //    novosCentroides[i] = createClusteredRegistro(getCluster(centroide), getX(centroide), getY(centroide));
        //}
        //free(registrosAssociados);
    }
    return novosCentroides;
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




    fclose(arquivoEntrada);

    FILE* arquivoSaida = escritorAutomatico("resultadoC.txt");




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