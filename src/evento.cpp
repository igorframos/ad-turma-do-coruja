#include "evento.h"

evento::evento (int tp, double t) : tp(tp), t(t) {}

bool evento::operator< (const evento& b) const
{
    if (t != b.tempo()) return t < b.tempo();
    return k < b.id();
}

int evento::tipo() const
{
    return tp;
}

inline unsigned int evento::id() const
{
    return k;
}

inline double evento::tempo() const
{
    return t;
}

std::string evento::strTipo() const
{
    switch (tp)
    {
        case CHEGADA_PEER:
            return "Chegada de peer";
        case SAIDA_PEER:
            return "Saída de peer";
        case TRANSMISSAO:
            return "Transmissão";
        default:
            return "Não reconheci o tipo";
    }
}

