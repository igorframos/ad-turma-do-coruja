#ifndef __PESSOA__
#define __PESSOA__

#include <string>

class pessoa
{
    int tp;
    unsigned int k;
    unsigned int b;

    public:
        pessoa(int tp);

        int tipo() const;
        unsigned int id() const;
        unsigned int blocos() const;
        unsigned int& blocos();

        unsigned int blocosFaltantes() const;
        unsigned int blocosPossiveis(const pessoa& p) const;

        std::string strTipo() const;

        enum tipoPessoa{PUBLISHER, PEER, SEED};

        static unsigned int nextId;
        static unsigned int arqCompleto;
};

#endif

