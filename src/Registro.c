#include "Registro.h"

Registro* createRegistro(double x, double y) {
    Registro* registro = (Registro*)malloc(sizeof(Registro));
    if (registro != NULL) {
        registro->cluster = -1;
        registro->x = x;
        registro->y = y;
        registro->centroideAssociado = NULL;
    }
    return registro;
}

Registro* createClusteredRegistro(int c, double x, double y) {
    Registro* registro = createRegistro(x, y);
    if (registro != NULL) {
        registro->cluster = c;
    }
    return registro;
}

int getCluster(Registro* registro) {
    return registro->cluster;
}

double getX(Registro* registro) {
    return registro->x;
}

double getY(Registro* registro) {
    return registro->y;
}

Registro* getCentroideAssociado(Registro* registro) {
    return registro->centroideAssociado;
}

void associarCentroideMaisProximo(Registro* registro, Registro** centroides, size_t centroidesSize) {
    double minDist = INFINITY;
    Registro* closestCentroide = NULL;

    for (size_t i = 0; i < centroidesSize; i++) {
        Registro* centroide = centroides[i];
        double dist = sqrt(pow(registro->x - centroide->x, 2) + pow(registro->y - centroide->y, 2));
        if (dist < minDist) {
            minDist = dist;
            closestCentroide = centroide;
        }
    }

    registro->centroideAssociado = closestCentroide;
    registro->cluster = closestCentroide->cluster;
}

Registro* transformaEmRegistro(char* linha) {
    double x, y;
    sscanf(linha, "%lf %lf", &x, &y);
    return createRegistro(x, y);
}