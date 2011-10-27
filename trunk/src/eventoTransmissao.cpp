#include "eventoTransmissao.h"

eventoTransmissao::eventoTransmissao(double t, pessoa* p) : evento(evento::TRANSMISSAO, t), p(p) {}

pessoa eventoTransmissao::origem() const
{
    return *p;
}

