#ifndef __EVENTO_CHEGADA_PEER__
#define __EVENTO_CHEGADA_PEER__

class eventoChegadaPeer : public evento
{
    int tpChegada;

    public:
        eventoChegadaPeer(int tp, double t, int tpChegada);

        int tipoChegada();
};

#endif

