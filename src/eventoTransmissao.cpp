#include "eventoTransmissao.h"

eventoTransmissao::eventoTransmissao(double t, const pessoa* p) : evento(evento::TRANSMISSAO, t), p(p) {}

pessoa eventoTransmissao::origem() const
{
    return *p;
}

const pessoa *eventoTransmissao::ptr() const
{
    return p;
}

