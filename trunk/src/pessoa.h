#ifndef __PESSOA__
#define __PESSOA__

#include <string>

class pessoa
{
    int tp;
    unsigned int k;
    unsigned int b;
    unsigned int c;

    double a;

    public:
        pessoa(const int tp, const unsigned int c, const double a);

        int tipo() const;
        double chegada() const;
        unsigned int id() const;
        unsigned int blocos() const;
        unsigned int& blocos();
        unsigned int cor() const;

        unsigned int blocosFaltantes() const;
        unsigned int blocosPossiveis(const pessoa& p) const;

        void viraSeed();

        std::string strTipo() const;

        bool operator<(const pessoa& p) const;

        enum tipoPessoa{PUBLISHER, PEER, SEED};

        static unsigned int nextId;
        static unsigned int arqCompleto;
};

#endif

