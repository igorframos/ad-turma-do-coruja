#ifndef __GERADOR_ALEATORIO__
#define __GERADOR_ALEATORIO__

#include <cmath>

class geradorAleatorio
{
    private:
        unsigned int seed;
        const static unsigned int MUL = 1000003;

    public:
        const static unsigned int RANDMAX = 1073741789 - 1;

        geradorAleatorio();
        geradorAleatorio(unsigned int seed);
        unsigned int randUniforme();
        double randExponencial(double lambda);
};

#endif

