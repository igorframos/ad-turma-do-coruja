#ifndef __EVENTO_TRANSMISSAO__
#define __EVENTO_TRANSMISSAO__

#include "evento.h"
#include "pessoa.h"

class eventoTransmissao : public evento
{
    const pessoa* p;

    public:
        eventoTransmissao(double t, const pessoa* p);

        pessoa origem() const;
        const pessoa *ptr() const;
};

#endif

