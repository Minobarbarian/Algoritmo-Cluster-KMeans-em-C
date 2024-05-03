#ifndef REGISTRO_H
#define REGISTRO_H

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

typedef struct Registro {
    int cluster;
    double x;
    double y;
    struct Registro* centroideAssociado;
} Registro;

Registro* createRegistro(double x, double y);
Registro* createClusteredRegistro(int c, double x, double y);
int getCluster(Registro* registro);
double getX(Registro* registro);
double getY(Registro* registro);
Registro* getCentroideAssociado(Registro* registro);
void associarCentroideMaisProximo(Registro* registro, Registro** centroides, size_t centroidesSize);
Registro* transformaEmRegistro(char* linha);

#endif /* REGISTRO_H */