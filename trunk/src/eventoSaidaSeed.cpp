#include "eventoSaidaSeed.h"

eventoSaidaSeed::eventoSaidaSeed(double t, pessoa *p) : evento(evento::SAIDA_PEER, t), p(p) {}

pessoa eventoSaidaSeed::seed() const
{
    return *p;
}

