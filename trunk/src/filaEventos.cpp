#include "filaEventos.h"

filaEventos::filaEventos(double lambda, double mu, double gamma, double U, double pRec, int pPeer, int pBloco, int peersIniciais, unsigned int arqInicial) : 
    lambda(lambda), mu(mu), gamma(gamma), U(U), pRec(pRec), tAtual(0), g(geradorAleatorio(2065)), publisher(pessoa(pessoa::PUBLISHER, -1, 0)), pPeer(pPeer), pBloco(pBloco), T(0), T0(0), T1(0), D(0), D0(0), D1(0), A(0), A0(0), A1(0), V(0), V0(0), V1(0), P(0), P0(0), P1(0), t(0), tTotal(0), tRodada(0), f(TRANSIENTE), fimDeRodada(false)
{
    if (peersIniciais == 0)
        agendaChegadaPeer(tAtual);
    agendaTransmissao(tAtual, publisher);

    maxBlocos = __builtin_popcount(pessoa::arqCompleto);
    saidas = 0;
    chegadas = 0;
    saidasComputadas = 0;
    downloadsConcluidos = 0;

    out = fopen("saidas.txt", "w");

    for (unsigned int i = 0; i < maxBlocos; ++i)
    {
        possuem.push_back(0);
    }

    for (int i = 0; i < peersIniciais; ++i)
    {
        peers.push_back(pessoa(pessoa::PEER, 0, 0));
        setPeers.insert(peers.back().id());
        peers.back().blocos() = arqInicial;
        agendaTransmissao(tAtual, peers.back());
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
    fila.insert(std::make_pair(e->tempo(), e));
}

void filaEventos::trataProximoEvento()
{
    evento *e = fila.begin()->second;
    fila.erase(fila.begin());

    tAtual = e->tempo();

    A += (tAtual - t) * pessoasNoSistema(); 
    P += (tAtual - t) * peersNoSistema();
    if (tempoN.size() <= pessoasNoSistema()) tempoN.push_back(tAtual - t);
    else tempoN[pessoasNoSistema()] += tAtual - t;
    tTotal += tAtual - t;
    t = tAtual;

    if (e->tipo() == evento::CHEGADA_PEER)
    {
        ++chegadas;
        trataChegadaPeer(*dynamic_cast<eventoChegadaPeer*>(e));
    }
    else if (e->tipo() == evento::SAIDA_PEER)
    {
        ++saidas;
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

    delete e;
}

void filaEventos::trataChegadaPeer(const eventoChegadaPeer& e)
{
    peers.push_back(pessoa(pessoa::PEER, f, tAtual));
    agendaChegadaPeer(tAtual);
    agendaTransmissao(tAtual, peers.back());
    setPeers.insert(peers.back().id());

    testaFimRodada();
}

void filaEventos::trataSaidaSeed(const eventoSaidaSeed& e)
{
    ++V;

    pessoa seed = e.seed();

    for (std::list<pessoa>::iterator i = seeds.begin(); i != seeds.end(); ++i)
    {
        if (i->id() == seed.id())
        {
            if (i->cor() == f)
            {
                //fprintf (out, "%u s %.12f\n", i->cor(), tAtual - i->chegada());
                T += tAtual - i->chegada();
                ++saidasComputadas;
            }

            setSeeds.erase(setSeeds.find(i->id()));

            seeds.erase(i);
            break;
        }
    }

    for (unsigned int i = 0; i < maxBlocos; ++i)
    {
        --possuem[i];
    }
    if (g.randUniforme() / (double) geradorAleatorio::RANDMAX <= pRec)
    {
        ++chegadas;

        peers.push_back(pessoa(pessoa::PEER, f, tAtual));
        setPeers.insert(peers.back().id());

        testaFimRodada();
    }
}

void filaEventos::trataTransmissao(const eventoTransmissao& e)
{    
    const pessoa *ptr = e.ptr();
    pessoa origem = e.origem(); 

    if (e.id() != publisher.id() && setSeeds.find(e.id()) == setSeeds.end() && setPeers.find(e.id()) == setPeers.end())
    {
        return;
    }

    agendaTransmissao(tAtual, *ptr);

    if (peers.size() == 0 || (peers.size() == 1 && origem.tipo() == pessoa::PEER))
    {
        return;
    }

    std::list<pessoa>::iterator it = escolhePeer(origem);
    pessoa& destino = *it;
    unsigned int blocoEscolhido = escolheBloco(origem, destino);

    if (blocoEscolhido > maxBlocos)
    {
        return;
    }

    destino.blocos() |= (1 << blocoEscolhido);
    ++possuem[blocoEscolhido];

    if (destino.blocosFaltantes() == 0)
    {
        if (it->cor() == f)
        {
            //fprintf (out, "%u d %.12f\n", it->cor(), tAtual - it->chegada());

            D += tAtual - destino.chegada();
            tDownloads.push_back(tAtual - destino.chegada());
            ++downloadsConcluidos;
        }

        destino.viraSeed();
        setPeers.erase(setPeers.find(it->id()));
        setSeeds.insert(it->id());
        peers.erase(it);
        seeds.push_back(destino);
        agendaSaidaSeed(tAtual, destino);
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

    if (peers.size() - sub == 0)
    {
        return peers.begin();
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
        blocoEscolhido = g.randUniforme() % maxBlocos;
        while (!(blocosPossiveis & (1 << blocoEscolhido)))
        {
            ++blocoEscolhido;
            blocoEscolhido %= maxBlocos;
        }
    }
    else
    {
        unsigned int minBlocos = 0x3f3f3f3f; // Valor "infinito"

        for (int bloco = 0; pessoa::arqCompleto & (1 << bloco); ++bloco)
        {
            if (!(blocosPossiveis & (1 << bloco))) continue;

            if (possuem[bloco] < minBlocos)
            {
                minBlocos = possuem[bloco];
                blocoEscolhido = bloco;
            }
        }
    }

    return blocoEscolhido;
}

void filaEventos::testaFimRodada()
{
    if ((f == TRANSIENTE && chegadas == DELTA) || (f != TRANSIENTE && chegadas == TAMRODADA))
    {
        for (int i = 0; i < (int) tempoN.size(); ++i)
        {
            saidaTempoN.push_back(tempoN[i] / tTotal);
        }
        tempoN.clear();

        double tmp1 = T / saidasComputadas - T0,
               tmp2 = D / downloadsConcluidos - D0,
               tmp3 = A / tTotal - A0,
               tmp4 = V / (tAtual - tRodada) - V0,
               tmp5 = P / tTotal - P0;
        if (tmp1 < 0) tmp1 *= -1;
        if (tmp2 < 0) tmp2 *= -1;
        if (tmp3 < 0) tmp3 *= -1;
        if (tmp4 < 0) tmp4 *= -1;
        if (tmp5 < 0) tmp5 *= -1;

        double tmp;
        tmp = std::max(tmp1, tmp2);
        tmp = std::max(tmp, tmp3);
        tmp = std::max(tmp, tmp4);
        tmp = std::max(tmp, tmp5);
        tmp = std::max(tmp, A1);
        tmp = std::max(tmp, D1);
        tmp = std::max(tmp, T1);
        tmp = std::max(tmp, V1);
        tmp = std::max(tmp, P1);
        T1 = tmp1;
        D1 = tmp2;
        A1 = tmp3;
        V1 = tmp4;
        P1 = tmp5;

        if (f != TRANSIENTE) printf ("Encerrada fase %u. Total de %u saidas (%u computadas). Tempo da rodada: %.12f\n", f, saidas, saidasComputadas, tTotal);
        //else fprintf (out, "Encerrada a fase transiente.\n");

        fprintf (out, "Debug: %.12f\n", tmp);
        T0 = T / saidasComputadas;
        D0 = D / downloadsConcluidos;
        A0 = A / tTotal;
        V0 = V / (tAtual - tRodada);
        P0 = P / tTotal;
        chegadas = 0;
        saidas = 0;
        saidasComputadas = 0;
        downloadsConcluidos = 0;
        tRodada = tAtual;
        tTotal = 0;
        A = 0; 
        T = 0;
        D = 0;
        V = 0;
        P = 0;

        if (f == TRANSIENTE && tmp > EPS)
        {
            return;
        }

        if (f != TRANSIENTE) fimDeRodada = true;
        ++f;
    }
}

unsigned int filaEventos::fase()
{
    return f;
}

unsigned int filaEventos::pessoasNoSistema()
{
    return peers.size() + seeds.size();
}

unsigned int filaEventos::peersNoSistema()
{
    return peers.size();
}

unsigned int filaEventos::chegadasTotais()
{
    return chegadas;
}

unsigned int filaEventos::saidasTotais()
{
    return saidas;
}

bool filaEventos::fimRodada()
{
    if (fimDeRodada)
    {
        fimDeRodada = false;

        return true;
    }

    return false;
}

std::vector<double> filaEventos::tempoPorN()
{
    std::vector<double> t = saidaTempoN;
    saidaTempoN.clear();
    return t;
}

std::vector<double> filaEventos::temposDeDownload()
{
    std::vector<double> t = tDownloads;
    tDownloads.clear();
    return t;
}

double filaEventos::mediaPermanencia()
{
    return T0;
}

double filaEventos::mediaDownload()
{
    return D0;
}

double filaEventos::mediaPessoas()
{
    return A0;
}

double filaEventos::mediaPeers()
{
    return P0;
}

double filaEventos::mediaVazao()
{
    return V0;
}

std::string binario(unsigned int x, unsigned int alg)
{
    std::string ansr;

    while (alg--)
    {
        ansr.push_back((char) (x % 2) + '0');
        x /= 2;
    }

    return ansr;
}

