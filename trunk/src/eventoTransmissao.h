#ifndef __EVENTO_TRANSMISSAO__
#define __EVENTO_TRANSMISSAO__

#include "evento.h"
#include "pessoa.h"

class eventoTransmissao : public evento
{
    pessoa* p;

    public:
        eventoTransmissao(double t, pessoa* p);

        pessoa origem() const;
};

#endif

