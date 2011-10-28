#include "evento.h"

unsigned int evento::nextId;

evento::evento (int tp, double t) : tp(tp), t(t), k(nextId++) {}

evento::~evento() {}

bool evento::operator< (const evento& b) const
{
    if (tempo() != b.tempo()) return t < b.tempo();
    return k < b.id();
}

int evento::tipo() const
{
    return tp;
}

unsigned int evento::id() const
{
    return k;
}

double evento::tempo() const
{
    return t;
}

std::string evento::strTipo() const
{
    switch (tp)
    {
        case CHEGADA_PEER:
            return "Chegada de peer";
        case TRANSMISSAO:
            return "Transmissão";
        case SAIDA_PEER:
            return "Saída de peer";
        default:
            return "Não reconheci o tipo";
    }
}

