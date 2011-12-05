#include "filaEventos.h"

// O construtor inicializa todas as muitas variáveis da simulação.
filaEventos::filaEventos(double lambda, double mu, double gamma, double U, double pRec, int pPeer, int pBloco, int peersIniciais, unsigned int arqInicial, char arqOut[64]):
    lambda(lambda), mu(mu), gamma(gamma), U(U), pRec(pRec), tAtual(0), g(geradorAleatorio(time(NULL) % 10000 + 1)), publisher(pessoa(pessoa::PUBLISHER, -1, 0)), pPeer(pPeer), pBloco(pBloco), T(0), T0(0), T1(0), D(0), D0(0), D1(0), A(0), A0(0), A1(0), V(0), V0(0), V1(100), P(0), P0(0), P1(0), t(0), tTotal(0), tRodada(0), f(TRANSIENTE), n(0), fimDeRodada(false)
{
    if (peersIniciais == 0) // Condição bem específica desse trabalho.
        agendaChegadaPeer(tAtual);
    agendaTransmissao(tAtual, publisher);

    maxBlocos = __builtin_popcount(pessoa::arqCompleto);    // Quero saber quantos blocos tem o arquivo inicial.
    saidas = 0;
    chegadas = 0;
    saidasComputadas = 0;
    totalSaidas = 0;
    downloadsConcluidos = 0;
    downloadsTotais = 0;
    eventosFimTrans = 0;

    for (unsigned int i = 0; i < maxBlocos; ++i)
    {
        possuem.push_back(0);
    }

    for (int i = 0; i < peersIniciais; ++i)
    {
        peers.push_back(pessoa(pessoa::PEER, 0, 0));
        setPeers.insert(peers.back().id());
        peers.back().blocos() = arqInicial;
        for (unsigned int i = 0; i < maxBlocos; ++i)
        {
            if (arqInicial & (1 << i)) ++possuem[i];
        }

        agendaTransmissao(tAtual, peers.back());
    }

    out = fopen(arqOut, "w");

    fprintf (out, "0 0 0 0 0 0\n");
}

filaEventos::~filaEventos()
{
    fclose(out);
    while (!fila.empty())
    {
        fila.erase(fila.begin());
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
    // Como a fila é um set e guardamos ponteiros para eventos, a ordenação é feita
    // introduzindo cada elemento como um par (tempo, evento*). Não é possível fazer
    // buscas aqui dentro, então vamos apenas ignorar eventos que deveriam ter sido
    // removidos anteriormente por algum motivo. O custo amortizado é baixo.
    fila.insert(std::make_pair(e->tempo(), e));
}

void filaEventos::trataProximoEvento()
{
    // Pega o primeiro evento da fila e depois o apaga.
    evento *e = fila.begin()->second;
    fila.erase(fila.begin());

    tAtual = e->tempo();    // O tempo da simulação salta de evento em evento.

    // Esse trecho vai atualizar as áreas de gráfico que dão o número de pessoas e de
    // peers no sistema em um dado momento. É feito aqui porque todos os eventos têm
    // potencial para alterar o número de pessoas ou peers no sistema. 
    A += (tAtual - t) * pessoasNoSistema();
    A1 += (tAtual - t) * pessoasNoSistema();
    P += (tAtual - t) * peersNoSistema();
    P1 += (tAtual - t) * peersNoSistema();
    if (tempoN.size() <= pessoasNoSistema()) tempoN.push_back(tAtual - t);
    else tempoN[pessoasNoSistema()] += tAtual - t;
    tTotal += tAtual - t;
    t = tAtual;

    // Usamos ponteiros para eventos na fila para que essa parte funcione.
    // A alternativa era ter 3 filas e escolher o evento entre elas, mas
    // preferimos os ponteiros por ser mais claro ter uma fila só.
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

    if (e->tipo() == evento::CHEGADA_PEER)
    {
        ++n;

        if (n % 50 == 0 && f <= 1)
        {
            fprintf (out, "%u %.12f %.12f %.12f %.12f %.12f\n", n, T1/totalSaidas, D1/downloadsTotais, A1/tAtual, V1/tAtual, P1/tAtual);
        }
    }

    delete e;
}

void filaEventos::trataChegadaPeer(const eventoChegadaPeer& e)
{
    peers.push_back(pessoa(pessoa::PEER, f, tAtual));
    agendaChegadaPeer(tAtual);
    agendaTransmissao(tAtual, peers.back());
    setPeers.insert(peers.back().id());

    testaFimRodada();   // Tamanho da rodada é medido em chegadas.
}

void filaEventos::trataSaidaSeed(const eventoSaidaSeed& e)
{
    ++V;
    ++V1;

    pessoa seed = e.seed();

    // Remove o seed da lista e do set.
    for (std::list<pessoa>::iterator i = seeds.begin(); i != seeds.end(); ++i)
    {
        if (i->id() == seed.id())
        {
            if (i->cor() == f)
            {
                T += tAtual - i->chegada();
                ++saidasComputadas;
            }

            T1 += tAtual - i->chegada();
            ++totalSaidas;

            setSeeds.erase(setSeeds.find(i->id()));

            seeds.erase(i);
            break;
        }
    }

    // Com a saída do seed, o número de pessoas que possuem cada um dos blocos diminui em 1.
    for (unsigned int i = 0; i < maxBlocos; ++i)
    {
        --possuem[i];
    }

    // Avalia chegadas por recomendação.
    if (g.randUniforme() / (double) geradorAleatorio::RANDMAX <= pRec)
    {
        ++chegadas;
        ++n;

        if (n % 50 == 0 && f <= 1)
        {
            fprintf (out, "%u %.12f %.12f %.12f %.12f %.12f\n", n, T1/totalSaidas, D1/downloadsTotais, A1/tAtual, V1/tAtual, P1/tAtual);
        }


        peers.push_back(pessoa(pessoa::PEER, f, tAtual));
        setPeers.insert(peers.back().id());

        testaFimRodada();   // Como é uma chegada, pode ter encerrado a rodada.
    }
}

void filaEventos::trataTransmissao(const eventoTransmissao& e)
{    
    const pessoa *ptr = e.ptr();
    pessoa origem = e.origem(); 

    // Verifica se o evento é válido. Se este evento devia ter sido apagado, será ignorado.
    if (e.id() != publisher.id() && setSeeds.find(e.id()) == setSeeds.end() && setPeers.find(e.id()) == setPeers.end())
    {
        return;
    }

    // Após transmitir, deve ser agendada a próxima transmissão da pessoa.
    agendaTransmissao(tAtual, *ptr);

    // Se não houver peers para receberem a transmissão, ela falha.
    if (peers.size() == 0 || (peers.size() == 1 && origem.tipo() == pessoa::PEER))
    {
        return;
    }


    std::list<pessoa>::iterator it = escolhePeer(origem);
    pessoa& destino = *it;
    unsigned int blocoEscolhido = escolheBloco(origem, destino);

    // O código para o caso de não poder transmitir nada é escolher um bloco inexistente.
    if (blocoEscolhido > maxBlocos)
    {
        return;
    }

    destino.blocos() |= (1 << blocoEscolhido);
    ++possuem[blocoEscolhido];

    // Trata o caso de o peer terminar o download com o bloco recebido.
    if (destino.blocosFaltantes() == 0)
    {
        // Se o peer que terminou o download for da cor certa, contamos seu tempo de download.
        if (it->cor() == f)
        {
            D += tAtual - destino.chegada();
            tDownloads.push_back(tAtual - destino.chegada());
            ++downloadsConcluidos;
        }

        D1 += tAtual - destino.chegada();
        ++downloadsTotais;

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
    unsigned int p = 0;     // "Índice" do peer escolhido.
    unsigned int sub = 0;   // Apenas para sabermos se a origem é um peer.

    if (origem.tipo() == pessoa::PEER)
    {
        sub = 1;
    }

    if (pPeer == RANDOM_PEER)
    {
        p = g.randUniforme() % (peers.size() - sub);
    }

    // Percorre a lista de peers até encontrar o escolhido.
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
        // Valor infinito para o número de pessoas que possuem o bloco mais raro.
        unsigned int minBlocos = 0x3f3f3f3f;

        // Escolhe o bloco possível mais raro.
        for (int bloco = 0; pessoa::arqCompleto & (1 << bloco); ++bloco)
        {
            if (!(blocosPossiveis & (1 << bloco))) continue;

            if (possuem[bloco] < minBlocos)
            {
                minBlocos = possuem[bloco];
                blocoEscolhido = bloco;
            }
        }

        if (!(blocosPossiveis & (1 << blocoEscolhido))) blocoEscolhido = maxBlocos + 1;
    }

    return blocoEscolhido;
}

void filaEventos::testaFimRodada()
{
    if ((f == TRANSIENTE && chegadas == DELTA) || (f != TRANSIENTE && chegadas == TAMRODADA))
    {
        // Prepara as porcentagens do tempo com k pessoas para serem repassadas e limpa os dados da rodada.
        for (int i = 0; i < (int) tempoN.size(); ++i)
        {
            saidaTempoN.push_back(tempoN[i] / tTotal);
        }
        tempoN.clear();

        // Variáveis para detectar o final da fase transiente. tmpi é a diferença da média desta rodada para
        // a média da rodada anterior, de modo que uma diferença muito grande tem grandes chances de significar
        // que ainda estamos na fase transiente.
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

        if (f != TRANSIENTE) printf ("Encerrada fase %u. Total de %u saidas (%u computadas). Tempo da rodada: %.12f\n", f, saidas, saidasComputadas, tTotal);
        else
        {
            eventosFimTrans = n;
            fimTrans = tAtual;
        }
        
        // Limpa resultados da rodada para calcular as próximas.
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

        // Escolhe a maior das diferenças como parâmetro.
        bool dif1 = tmp1 > EPS * T0,
             dif2 = tmp2 > EPS * D0,
             dif3 = tmp3 > EPS * A0,
             dif4 = tmp4 > EPS * V0,
             dif5 = tmp5 > EPS * P0;

        double tmpVal = std::max(T0, D0);
        tmpVal = std::max(tmpVal, A0);
        tmpVal = std::max(tmpVal, V0);
        tmpVal = std::max(tmpVal, P0);

        if (f == TRANSIENTE)
        {
            tDownloads.clear();
            saidaTempoN.clear();
            if (dif1 || dif2 || dif3 || dif4 || dif5)
            {
                return;
            }
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

unsigned int filaEventos::eventosFaseTransiente()
{
    return eventosFimTrans;
}

double filaEventos::fimFaseTransiente()
{
    return fimTrans;
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

