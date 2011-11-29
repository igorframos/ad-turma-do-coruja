#ifndef __FILA_EVENTOS__
#define __FILA_EVENTOS__

#include <list>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cmath>

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
    double T, T0, T1, D, D0, D1, A, A0, A1, V, V0, V1;
    double t, tTotal, tRodada;
    unsigned int f;
    unsigned int maxBlocos;
    unsigned int saidas, chegadas, saidasComputadas, downloadsConcluidos;
    std::vector<unsigned int> possuem;
    std::vector<double> tempoN;
    FILE* out;

    bool fimDeRodada;
    std::vector<double> tDownloads;
    std::vector<double> saidaTempoN;

    void agendaChegadaPeer(double t);
    void agendaSaidaSeed(double t, const pessoa& p);
    void agendaTransmissao(double t, const pessoa& p);

    void insereEvento(evento* e);

    void trataChegadaPeer(const eventoChegadaPeer& e);
    void trataSaidaSeed(const eventoSaidaSeed& e);
    void trataTransmissao(const eventoTransmissao& e);

    void testaFimRodada();

    std::list<pessoa>::iterator escolhePeer(const pessoa& origem);
    unsigned int escolheBloco(const pessoa& origem, const pessoa& destino);

    public:
        filaEventos(double lambda, double mu, double gamma, double U, double pRec, int pPeer, int pBloco, int peersIniciais);
        // pRec = probabilidade de recomendarem
        // pPeer = seleção de peer
        // pBloco = seleção de bloco
        ~filaEventos();

        bool haEvento();
        void trataProximoEvento();
        unsigned int fase();
        unsigned int pessoasNoSistema();
        unsigned int usuarios();
        unsigned int peersNoSistema();
        unsigned int chegadasTotais();
        unsigned int saidasTotais();

        std::vector<double> tempoPorN();
        std::vector<double> temposDeDownload();
        bool fimRodada();
        double mediaVazao();
        double mediaPessoas();
        double mediaDownload();

        const static unsigned int TRANSIENTE = 0;
        const static unsigned int DELTA = 100;
        const static unsigned int TAMRODADA = 2500;
        const static double EPS = 1.5;
        enum politicas{RANDOM_PEER, RANDOM_PIECE, RAREST_FIRST};
};

std::string binario(unsigned int x, unsigned int alg);

#endif

