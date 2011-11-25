#ifndef __FILA_EVENTOS__
#define __FILA_EVENTOS__

#include <list>
#include <vector>
#include <cstdio>
#include <cstdlib>

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
    double pRec;
    double tAtual;
    geradorAleatorio g;
    std::list<evento*> fila;
    pessoa publisher;
    std::list<pessoa> peers;
    std::list<pessoa> seeds;
    int pPeer, pBloco;
    unsigned int maxBlocos;
    unsigned int saidas, chegadas;
    std::vector<unsigned int> possuem;

    void agendaChegadaPeer(double t);
    void agendaSaidaSeed(double t, const pessoa& p);
    void agendaTransmissao(double t, const pessoa& p);

    void insereEvento(evento* e);

    void trataChegadaPeer(const eventoChegadaPeer& e);
    void trataSaidaSeed(const eventoSaidaSeed& e);
    void trataTransmissao(const eventoTransmissao& e);

    std::list<pessoa>::iterator escolhePeer(const pessoa& origem);
    unsigned int escolheBloco(const pessoa& origem, const pessoa& destino);

    public:
        filaEventos(double lambda, double mu, double gamma, double U, double pRec, int pPeer, int pBloco);
        // pRec = probabilidade de recomendarem
        // pPeer = seleção de peer
        // pBloco = seleção de bloco

        bool haEvento();
        void trataProximoEvento();
        unsigned int pessoasNoSistema();
        unsigned int chegadasTotais();
        unsigned int saidasTotais();

        enum politicas{RANDOM_PEER, RANDOM_PIECE, RAREST_FIRST};
};

std::string binario(unsigned int x);

#endif

