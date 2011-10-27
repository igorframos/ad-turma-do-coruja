#ifndef __FILA_EVENTOS__
#define __FILA_EVENTOS__

#include <list>

#include "pessoa.h"
#include "evento.h"
#include "eventoChegadaPeer.h"
#include "eventoSaidaSeed.h"
#include "eventoTransmissao.h"

#include "geradorAleatorio.h"

class filaEventos
{
    // Atenção: letras correspondentes a uma taxa são, na verdade, a média (1 / taxa).
    // Isso facilita tratar casos como gamma = infinito.
    // Pode dar problema com o caso em que não chegam peers, mas esse é mais fácil de tratar.
    double lambda, mu, gamma, U;
    double tAtual;
    geradorAleatorio g(1003);
    std::list<evento*> fila;
    pessoa publisher(pessoa::PUBLISHER);
    std::set<pessoa> peers;
    std::set<pessoa> seeds;

    void agendaChegadaPeer(double t);
    void agendaSaidaSeed(double t, const pessoa& p);
    void agendaTransmissao(double t, const pessoa& p);

    void trataProximoEvento();
    void insereEvento(const evento* e);

    public:
        filaEventos(double taxa);
};

#endif

