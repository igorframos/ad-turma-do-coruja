#include "filaEventos.h"

filaEventos::filaEventos(double lambda, double mu, double gamma, double U, double pRec, int pPeer, int pBloco, int peersIniciais) : 
    lambda(lambda), mu(mu), gamma(gamma), U(U), pRec(pRec), tAtual(0), g(geradorAleatorio(2065)), publisher(pessoa(pessoa::PUBLISHER, -1, 0)), pPeer(pPeer), pBloco(pBloco), T(0), T0(0), T1(0), D(0), D0(0), D1(0), A(0), A0(0), A1(0), V(0), V0(0), V1(0), t(0), tTotal(0), tRodada(0), f(TRANSIENTE), fimDeRodada(false)
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
        agendaTransmissao(tAtual, peers.back());
        printf ("%d\n", peersIniciais);
    }
}

filaEventos::~filaEventos()
{
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
    std::list<evento*>::iterator it = fila.begin();

    while (it != fila.end() && **it < *e) ++it;

    fila.insert(it, e);
}

void filaEventos::trataProximoEvento()
{
    // Isso aqui é uma gambiarra para que apareça algo na tela
    // do terminal mesmo quando eu redirecionar a saída para um
    // arquivo como está no Makefile.
    int ba = system("Próximo_evento");
    ++ba;

    std::list<evento*>::iterator it = fila.begin();
    evento *e = *it;
    fila.erase(it);

    tAtual = e->tempo();

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
    

    for (int i = 0; i < (int) maxBlocos; ++i)
    {
        printf ("possuem %d: %u\t", i, possuem[i]);
    }
    printf ("\n");
    for (std::list<pessoa>::iterator i = peers.begin(); i != peers.end(); ++i)
    {
        printf ("Peer %u: %s\n", i->id(), binario(i->blocos(), maxBlocos).c_str());
    }
    for (std::list<pessoa>::iterator i = seeds.begin(); i != seeds.end(); ++i)
    {
        printf ("Seed %u: %s\n", i->id(), binario(i->blocos(), maxBlocos).c_str());
    }
}

void filaEventos::trataChegadaPeer(const eventoChegadaPeer& e)
{
    // Isso aqui é uma gambiarra para que apareça algo na tela
    // do terminal mesmo quando eu redirecionar a saída para um
    // arquivo como está no Makefile.
    int ba = system("Chegada de peer");
    ++ba;

    A += (tAtual - t) * pessoasNoSistema(); 
    if (tempoN.size() <= pessoasNoSistema()) tempoN.push_back(tAtual - t);
    else tempoN[pessoasNoSistema()] += tAtual - t;
    tTotal += tAtual - t;
    t = tAtual;
    
    peers.push_back(pessoa(pessoa::PEER, f, tAtual));
    agendaChegadaPeer(tAtual);
    agendaTransmissao(tAtual, peers.back());

    printf ("%.10f: Chegada de peer.\n", tAtual);

    testaFimRodada();
}

void filaEventos::trataSaidaSeed(const eventoSaidaSeed& e)
{
    // Isso aqui é uma gambiarra para que apareça algo na tela
    // do terminal mesmo quando eu redirecionar a saída para um
    // arquivo como está no Makefile.
    int ba = system("Saída de seed");
    ++ba;

    ++V;

    A += (tAtual - t) * pessoasNoSistema(); 
    if (tempoN.size() <= pessoasNoSistema()) tempoN.push_back(tAtual - t);
    else tempoN[pessoasNoSistema()] += tAtual - t;
    tTotal += tAtual - t;
    t = tAtual;

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

    for (std::list<pessoa>::iterator i = seeds.begin(); i != seeds.end(); ++i)
    {
        if (i->id() == seed.id())
        {
            if (i->cor() == f)
            {
                fprintf (out, "%u s %.15f\n", i->cor(), tAtual - i->chegada());
                T += tAtual - i->chegada();
                ++saidasComputadas;
            }

            seeds.erase(i);
            break;
        }
    }

    for (unsigned int i = 0; i < maxBlocos; ++i)
    {
        --possuem[i];
    }
    printf ("%.10f: Saída do seed com id %u.\n", tAtual, seed.id());

    if (g.randUniforme() / (double) geradorAleatorio::RANDMAX <= pRec)
    {
        A += (tAtual - t) * pessoasNoSistema(); 
        if (tempoN.size() <= pessoasNoSistema()) tempoN.push_back(tAtual - t);
        else tempoN[pessoasNoSistema()] += tAtual - t;
        tTotal += tAtual - t;
        t = tAtual;
        ++chegadas;

        peers.push_back(pessoa(pessoa::PEER, f, tAtual));

        printf ("%.10f: Chegada de peer via recomendação.\n", tAtual);

        testaFimRodada();
    }
}

void filaEventos::trataTransmissao(const eventoTransmissao& e)
{    
    // Isso aqui é uma gambiarra para que apareça algo na tela
    // do terminal mesmo quando eu redirecionar a saída para um
    // arquivo como está no Makefile.
    int ba = system("Transmissão");
    ++ba;

    const pessoa *ptr = e.ptr();
    pessoa origem = e.origem(); 

    agendaTransmissao(tAtual, *ptr);

    printf ("%d %s\n", peers.size(), origem.strTipo().c_str());

    if (peers.size() == 0 || (peers.size() == 1 && origem.tipo() == pessoa::PEER))
    {
        printf ("%.10f: Tentativa de transmissão partindo do %s de id %d (%x), mas não há peers no sistema.\n",
                tAtual, origem.strTipo().c_str(), origem.id(), origem.blocos());
        return;
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
        if (it->cor() == f)
        {
            fprintf (out, "%u d %.15f\n", it->cor(), tAtual - it->chegada());
            D += tAtual - destino.chegada();
            ++downloadsConcluidos;
        }

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
    printf ("Posso transmitir algum em %s.\n", binario(blocosPossiveis, maxBlocos).c_str());

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

void filaEventos::testaFimRodada()
{
    if ((f == TRANSIENTE && chegadas == DELTA) || (f != TRANSIENTE && chegadas == TAMRODADA))
    {
        fprintf (out, "Tempo médio de permanência no sistema: %.15f (%u saídas computadas)\n", T / saidasComputadas, saidasComputadas);
        fprintf (out, "Tempo médio de download: %.15f (%u downloads concluídos)\n", D / downloadsConcluidos, downloadsConcluidos);
        fprintf (out, "Número médio de pessoas no sistema: %.15f (%.15f tempo decorrido)\n", A / tTotal, tTotal);
        fprintf (out, "Vazão: %.15f\n", V / (tAtual - tRodada));

        double tmp = 0;
        for (int i = 0; i < (int) tempoN.size(); ++i)
        {
            tmp += tempoN[i];
        }
        for (int i = 0; i < (int) tempoN.size(); ++i)
        {
            fprintf (out, "%d: %f, ", i, tempoN[i] / tmp);
            saidaTempoN.push_back(tempoN[i]);
            tempoN[i] = 0;
        }
        fprintf (out, "\n");

        double tmp1 = T / saidasComputadas - T0,
               tmp2 = D / downloadsConcluidos - D0,
               tmp3 = A / tTotal - A0,
               tmp4 = V / (tAtual - tRodada) - V0;
        if (tmp1 < 0) tmp1 *= -1;
        if (tmp2 < 0) tmp2 *= -1;
        if (tmp3 < 0) tmp3 *= -1;
        if (tmp4 < 0) tmp4 *= -1;

        tmp = std::max(tmp1, tmp2);
        tmp = std::max(tmp, tmp3);
        tmp = std::max(tmp, tmp4);
        tmp = std::max(tmp, A1);
        tmp = std::max(tmp, D1);
        tmp = std::max(tmp, T1);
        tmp = std::max(tmp, V1);
        T1 = tmp1;
        D1 = tmp2;
        A1 = tmp3;
        V1 = tmp4;

        fprintf (out, "Debug: %.15f\n", tmp);
        T0 = T / saidasComputadas;
        D0 = D / downloadsConcluidos;
        A0 = A / tTotal;
        V0 = V / (tAtual - tRodada);
        chegadas = 0;
        saidas = 0;
        saidasComputadas = 0;
        downloadsConcluidos = 0;
        tRodada = tAtual;
        t = 0;
        tTotal = 0;
        A = 0;
        T = 0;
        D = 0;
        V = 0;

        if (f == TRANSIENTE && tmp > EPS)
        {
            return;
        }

        if (f != TRANSIENTE) printf ("Encerrada fase %u. Total de %u saidas.\n", f, saidas);
        else fprintf (out, "Encerrada a fase transiente.\n");

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
    return saidaTempoN;
}

std::vector<double> filaEventos::temposDeDownload()
{
    return tDownloads;
}

double filaEventos::mediaDownload()
{
    return D0;
}

double filaEventos::mediaPessoas()
{
    return A0;
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

