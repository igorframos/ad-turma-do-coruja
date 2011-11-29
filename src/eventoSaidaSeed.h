#ifndef __EVENTO_SAIDA_SEED__
#define __EVENTO_SAIDA_SEED__

#include "evento.h"
#include "pessoa.h"
#include <list>

class eventoSaidaSeed : public evento
{
    const pessoa *p;

    public:
        eventoSaidaSeed(double t, const pessoa *p);

        pessoa seed() const;
        const pessoa *ptr() const;
};

#endif

