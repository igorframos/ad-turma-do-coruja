#include "filaEventos.h"

filaEventos::filaEventos(double lambda, double mu, double gamma, double U, double pRec, int pPeer, int pBloco) : 
    lambda(lambda), mu(mu), gamma(gamma), U(U), pRec(pRec), tAtual(0), g(geradorAleatorio(1003)), publisher(pessoa(pessoa::PUBLISHER)), pPeer(pPeer), pBloco(pBloco)
{
    agendaChegadaPeer(tAtual);
    agendaTransmissao(tAtual, publisher);

    maxBlocos = __builtin_popcount(pessoa::arqCompleto);

    for (unsigned int i = 0; i < maxBlocos; ++i)
    {
        possuem.push_back(1);
    }
}

bool filaEventos::haEvento()
{
    return fila.size() > 0;
}

void filaEventos::agendaChegadaPeer(double t)
{
    t += g.randExponencial(lambda);
    insereEvento(new eventoChegadaPeer(t));
}

void filaEventos::agendaSaidaSeed(double t, const pessoa& p)
{
    t += g.randExponencial(gamma);
    insereEvento(new eventoSaidaSeed(t, &p));
}

void filaEventos::agendaTransmissao(double t, const pessoa& p)
{
    printf ("%.10f --> %s\n", tAtual, p.strTipo().c_str());
    if (p.tipo() == pessoa::PUBLISHER)
    {
        t += g.randExponencial(U);
    }
    else
    {
        t += g.randExponencial(mu);
    }

    insereEvento(new eventoTransmissao(t, &p));
}

void filaEventos::insereEvento(evento* e)
{
    std::list<evento*>::iterator it = fila.begin();

    while (it != fila.end() && **it < *e) ++it;

    fila.insert(it, e);
}

void filaEventos::trataProximoEvento()
{
    system("Próximo evento");

    std::list<evento*>::iterator it = fila.begin();
    evento *e = *it;
    fila.erase(it);

    tAtual = e->tempo();

    if (e->tipo() == evento::CHEGADA_PEER)
    {
        trataChegadaPeer(*dynamic_cast<eventoChegadaPeer*>(e));
    }
    else if (e->tipo() == evento::SAIDA_PEER)
    {
        trataSaidaSeed(*dynamic_cast<eventoSaidaSeed*>(e));
    }
    else if (e->tipo() == evento::TRANSMISSAO)
    {
        trataTransmissao(*dynamic_cast<eventoTransmissao*>(e));
    }
    else
    {
        printf ("Evento de tipo não conhecido.\n");
    }
}

void filaEventos::trataChegadaPeer(const eventoChegadaPeer& e)
{
    system("Chegada de peer");

    peers.push_back(pessoa(pessoa::PEER));
    agendaChegadaPeer(tAtual);
    agendaTransmissao(tAtual, peers.back());

    printf ("%.10f: Chegada de peer.\n", tAtual);
}

void filaEventos::trataSaidaSeed(const eventoSaidaSeed& e)
{
    system("Saída de seed");

    pessoa seed = e.seed();

    for (std::list<evento*>::iterator i = fila.begin(); i != fila.end(); ++i)
    {
        evento *e1 = *i;
        if (e1->tipo() == evento::TRANSMISSAO)
        {
            pessoa p = dynamic_cast<eventoTransmissao*>(e1)->origem();
            if (p.id() == seed.id())
            {
                i = fila.erase(i);
                break;
            }
        }
    }

    for (unsigned int i = 0; i < maxBlocos; ++i)
    {
        --possuem[i];
    }

    printf ("%.10f: Saída do seed com id %u.\n", tAtual, seed.id());

    if (g.randUniforme() / (double) geradorAleatorio::RANDMAX < pRec)
    {
        peers.push_back(pessoa(pessoa::PEER));

        printf ("%.10f: Chegada de peer via recomendação.\n", tAtual);
    }
}

void filaEventos::trataTransmissao(const eventoTransmissao& e)
{
    system("Transmissão");

    const pessoa *ptr = e.ptr();
    pessoa origem = e.origem(); 

    agendaTransmissao(tAtual, *ptr);

    if (peers.size() == 0 || (peers.size() == 1 && origem.tipo() == pessoa::PEER))
    {
        printf ("%.10f: Tentativa de transmissão partindo do %s de id %d (%x), mas não há peers no sistema.\n",
                tAtual, origem.strTipo().c_str(), origem.id(), origem.blocos());
    }

    std::list<pessoa>::iterator it = escolhePeer(origem);
    pessoa& destino = *it;
    unsigned int blocoEscolhido = escolheBloco(origem, destino);

    printf ("%.10f: Tentativa de transmissão partindo do %s de id %d (%x) para o peer de id %d (%x): ", 
            tAtual, origem.strTipo().c_str(), origem.id(), origem.blocos(), destino.id(), destino.blocos());
    
    if (blocoEscolhido > maxBlocos)
    {
        printf ("falhou.\n");
        return;
    }

    destino.blocos() |= (1 << blocoEscolhido);
    ++possuem[blocoEscolhido];

    printf ("transmitido o bloco %u.\n", blocoEscolhido);

    if (destino.blocosFaltantes() == 0)
    {
        destino.viraSeed();
        peers.erase(it);
        seeds.push_back(destino);
        agendaSaidaSeed(tAtual, destino);
        printf ("%.10f: Peer de id %u completou o download e agora é um seed.\n", tAtual, destino.id());
    }
}

std::list<pessoa>::iterator filaEventos::escolhePeer(const pessoa& origem)
{
    unsigned int p = 0;
    unsigned int sub = 0;

    if (origem.tipo() == pessoa::PEER)
    {
        sub = 1;
    }

    if (pPeer == RANDOM_PEER)
    {
        p = g.randUniforme() % (peers.size() - sub);
    }

    std::list<pessoa>::iterator it = peers.begin();
    while (p)
    {
        ++it;
        if (it->id() == origem.id()) ++it;

        --p;
    }

    return it;
}

unsigned int filaEventos::escolheBloco(const pessoa& origem, const pessoa& destino)
{
    unsigned int blocoEscolhido = 0;
    unsigned int blocosPossiveis = origem.blocosPossiveis(destino);
    unsigned int numBlocosPossiveis = __builtin_popcount(blocosPossiveis);

    if (numBlocosPossiveis == 0)
    {
        return maxBlocos + 1;
    }

    if (pBloco == RANDOM_PIECE)
    {
        unsigned int bloco = g.randUniforme() % numBlocosPossiveis;

        blocoEscolhido = 0;
        while (bloco)
        {
            if (blocosPossiveis & (1 << blocoEscolhido))
            {
                --bloco;
            }

            ++blocoEscolhido;
        }
    }
    else
    {
        unsigned int minBlocos = 0x3f3f3f3f; // Valor "infinito"

        for (blocoEscolhido = 0; pessoa::arqCompleto & (1 << blocoEscolhido); ++blocoEscolhido)
        {
            if (!(blocosPossiveis & (1 << blocoEscolhido))) continue;

            if (possuem[blocoEscolhido] < minBlocos)
            {
                minBlocos = possuem[blocoEscolhido];
            }
        }
    }

    return blocoEscolhido;
}

unsigned int filaEventos::pessoasNoSistema()
{
    return peers.size() + seeds.size() + 1;
}

