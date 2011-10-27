#include "pessoa.h"

unsigned int pessoa::nextId;
unsigned int pessoa::arqCompleto;

pessoa::pessoa(int tp) : tp(tp), k(nextId++)
{
    if (tp == PUBLISHER || tp == SEED)
    {
        b = arqCompleto;
    }
    else if (tp == PEER)
    {
        b = 0;
    }
}

unsigned int pessoa::id() const
{
    return k;
}

int pessoa::tipo() const
{
    return tp;
}

unsigned int pessoa::blocos() const
{
    return b;
}

unsigned int& pessoa::blocos()
{
    return b;
}

unsigned int pessoa::blocosPossiveis(const pessoa& p) const
{
    return (b ^ p.blocos()) & b;
}

unsigned int pessoa::blocosFaltantes() const
{
    return __builtin_popcount(b ^ arqCompleto);
}

void pessoa::viraSeed()
{
    tp = SEED;
}

std::string pessoa::strTipo() const
{
    switch(tp)
    {
        case PUBLISHER:
            return "Publisher";
        case PEER:
            return "Peer";
        case SEED:
            return "Seed";
        default:
            return "Não reconheci o tipo";
    }
}

