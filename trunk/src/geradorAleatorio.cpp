#include "geradorAleatorio.h"

geradorAleatorio::geradorAleatorio() : seed(1) {}

geradorAleatorio::geradorAleatorio(unsigned int seed) : seed(seed) {}

unsigned int geradorAleatorio::randUniforme()
{
    long long tmp = seed;
    tmp *= MUL;
    tmp %= RANDMAX + 1;
    return seed = (unsigned int) tmp;
}

double geradorAleatorio::randExponencial(double mean)
{
    double u = randUniforme() / (double) RANDMAX; 

    return - log(u) * mean;
}

