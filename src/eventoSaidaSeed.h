#ifndef __EVENTO_SAIDA_SEED__
#define __EVENTO_SAIDA_SEED__

#include "evento.h"
#include "pessoa.h"

class eventoSaidaSeed : public evento
{
    pessoa *p;

    public:
        eventoSaidaSeed(double t, pessoa *p);

        pessoa seed() const;
};

#endif

