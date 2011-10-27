#ifndef __EVENTO__
#define __EVENTO__

#include <string>

class evento
{
    protected:
        int tp;
        double t;
        unsigned int k;

    public:
        evento(int tp, double t);
        virtual ~evento();

        bool operator<(const evento& b) const;
 
        int tipo() const;
        double tempo() const;
        unsigned int id() const;
        std::string strTipo() const;

        enum tipoEvento{CHEGADA_PEER, SAIDA_PEER, TRANSMISSAO};

        static unsigned int nextId;
};

#endif

