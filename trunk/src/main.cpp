#include "include.h"

int main(int argc, char *argv[])
{
    FILE* cenarios = fopen(argv[1], "r");
    if (!cenarios)
    {
        printf ("%s\n", argv[1]);
        printf ("Não deu pra abrir os cenários.\n");
        return 1;
    }

    FILE* resultados = fopen("resultados.txt", "w");

    while (1)
    {
        unsigned int cenario;
        unsigned int arquivo;
        double lambda;
        double mu;
        double U;
        double gamma;
        double pRec;
        unsigned int populacaoInicial;
        char politicaPeer;
        int pPeer;
        char politicaBloco;
        int pBloco;
        unsigned int arqInicial;

        int ba; // Variável só para tirar o warning de valor de retorno ignorado

        ba = fscanf (cenarios, "%u", &cenario); ++ba;
        if (!cenario) break;

        ba = fscanf (cenarios, "%x %lf %lf", &arquivo, &lambda, &mu); ++ba;
        ba = fscanf (cenarios, "%lf %lf %lf %d", &U, &gamma, &pRec, &populacaoInicial); ++ba;
        ba = fscanf (cenarios, " %c %c %u", &politicaPeer, &politicaBloco, &arqInicial); ++ba;

        if (politicaPeer == 'r') pPeer = filaEventos::RANDOM_PEER;
        if (politicaBloco == 'r') pBloco = filaEventos::RANDOM_PIECE;
        else pBloco = filaEventos::RAREST_FIRST;

        evento::nextId = 0;
        pessoa::nextId = 0;
        pessoa::arqCompleto = arquivo;
        
        printf ("Começarei o cenário %d com arquivo %x, lambda %.1f, mu %.1f, U %.1f, gamma %.1f e população inicial de %d\n", cenario, arquivo, lambda, mu, U, gamma, populacaoInicial);

        filaEventos f(1/lambda, 1/mu, 1/gamma, 1/U, pRec, pPeer, pBloco, populacaoInicial, arqInicial);

        double tempoDownload = 0, tempoDownload2 = 0, Ld, Ud, pd;
        double vazao = 0, vazao2 = 0, Lv, Uv, pv;
        double pessoas = 0, pessoas2 = 0, Ln, Un, pn;
        double peers = 0, peers2 = 0, Lp, Up, pp;
        double tempo = 0, tempo2 = 0, Lt, Ut, pt;
        std::vector<double> tempoPorN, tempoPorN2, LpN, UpN, ppN;
        std::vector<std::vector<double> > temposDownload;
        int n = 0;

        while (f.haEvento())
        {
            f.trataProximoEvento();

            if (!f.fimRodada()) continue;

            ++n;
            double mediaDownload = f.mediaDownload();
            double mediaVazao = f.mediaVazao();
            double mediaN = f.mediaPessoas();
            double mediaPeers = f.mediaPeers();
            double mediaT = f.mediaPermanencia();
            if (cenario == 2) temposDownload.push_back(f.temposDeDownload());

            tempoDownload += mediaDownload;
            tempoDownload2 += mediaDownload * mediaDownload;
            vazao += mediaVazao;
            vazao2 += mediaVazao * mediaVazao;
            pessoas += mediaN;
            pessoas2 += mediaN * mediaN;
            peers += mediaPeers;
            peers2 += mediaPeers * mediaPeers;
            tempo += mediaT;
            tempo2 += mediaT * mediaT;

            if (cenario == 1)
            {
                std::vector<double> tempoN = f.tempoPorN();
                for (int i = 0; i < (int) tempoN.size(); ++i)
                {
                    if ((int) tempoPorN.size() <= i)
                    {
                        tempoPorN.push_back(tempoN[i]);
                        tempoPorN2.push_back(tempoN[i] * tempoN[i]);
                    }
                    else
                    {
                        tempoPorN[i] += tempoN[i];
                        tempoPorN2[i] += tempoN[i] * tempoN[i];
                    }
                }
            }

            if (n < 2) continue;

            bool encerra = (n >= 30);

            if (cenario == 1)
            {
                for (int i = 0; i < (int) tempoPorN.size(); ++i)
                {
                    while (LpN.size() < tempoPorN.size()) LpN.push_back(0);
                    while (UpN.size() < tempoPorN.size()) UpN.push_back(0);
                    while (ppN.size() < tempoPorN.size()) ppN.push_back(0);

                    double mu = tempoPorN[i] / n;
                    double sigma = sqrt((tempoPorN2[i] - 2 * mu * tempoPorN[i] + n * mu * mu) / (n - 1));
                    LpN[i] = mu - 1.96 * sigma / sqrt(n);
                    UpN[i] = mu + 1.96 * sigma / sqrt(n);
                    ppN[i] = 100 * 1.96 * sigma / (mu * sqrt(n));

                    printf ("%d: %.12f (%.12f, %.12f) %.12f\n", i, mu, LpN[i], UpN[i], ppN[i]);
                    if (ppN[i] > 10) 
                    {
                        encerra = false;
                    }
                }
            }

            double mu = tempoDownload / n;
            double sigma = sqrt((tempoDownload2 - 2 * mu * tempoDownload + n * mu * mu) / (n - 1));
            Ld = mu - 1.96 * sigma / sqrt(n);
            Ud = mu + 1.96 * sigma / sqrt(n);
            pd = 100 * 1.96 * sigma / (mu * sqrt(n));

            if (pd > 10) encerra = false;

            mu = vazao / n;
            sigma = sqrt((vazao2 - 2 * mu * vazao + n * mu * mu) / (n - 1));
            Lv = mu - 1.96 * sigma / sqrt(n);
            Uv = mu + 1.96 * sigma / sqrt(n);
            pv = 100 * 1.96 * sigma / (mu * sqrt(n));

            if (pv > 10) encerra = false;

            mu = pessoas / n;
            sigma = sqrt((pessoas2 - 2 * mu * pessoas + n * mu * mu) / (n - 1));
            Ln = mu - 1.96 * sigma / sqrt(n);
            Un = mu + 1.96 * sigma / sqrt(n);
            pn = 100 * 1.96 * sigma / (mu * sqrt(n));

            if (pn > 10) encerra = false;

            mu = peers / n;
            sigma = sqrt((peers2 - 2 * mu * peers + n * mu * mu) / (n - 1));
            Lp = mu - 1.96 * sigma / sqrt(n);
            Up = mu + 1.96 * sigma / sqrt(n);
            pp = 100 * 1.96 * sigma / (mu * sqrt(n));

            if (pp > 10) encerra = false;

            mu = tempo / n;
            sigma = sqrt((tempo2 - 2 * mu * tempo + n * mu * mu) / (n - 1));
            Lt = mu - 1.96 * sigma / sqrt(n);
            Ut = mu + 1.96 * sigma / sqrt(n);
            pt = 100 * 1.96 * sigma / (mu * sqrt(n));

            if (pt > 10) encerra = false;

            printf ("Rodada %d\n", n);
            printf ("\tDownload:    %.12f (%.12f, %.12f) %.12f\n", tempoDownload / n, Ld, Ud, pd);
            printf ("\tPermanencia: %.12f (%.12f, %.12f) %.12f\n", tempo / n, Lt, Ut, pt);
            printf ("\tVazão:       %.12f (%.12f, %.12f) %.12f\n", vazao / n, Lv, Uv, pv);
            printf ("\tPessoas:     %.12f (%.12f, %.12f) %.12f\n", pessoas / n, Ln, Un, pn);
            printf ("\tPeers:       %.12f (%.12f, %.12f) %.12f\n", peers / n, Lp, Up, pp);

            if (encerra == true) break;
        }

        printf ("Encerrei o cenário %d com arquivo %x, lambda %.1f, mu %.1f, U %.1f, gamma %.1f e população inicial de %d\n", cenario, arquivo, lambda, mu, U, gamma, populacaoInicial);

        double mud = tempoDownload / n;
        double muv = vazao / n;
        double mun = pessoas / n;
        double mup = peers / n;
        double mut = tempo / n;
        fprintf (resultados, "Cenário: %d - Arquivo: %x - lambda: %.1f - População Inicial: %d - Rodadas: %d\n", cenario, arquivo, lambda, populacaoInicial, n);
        fprintf (resultados, "Média (Tempo de Download): %.12f - IC: (%.12f, %.12f) - P: %.12f\n", mud, Ld, Ud, pd);
        fprintf (resultados, "Média (Tempo de Permanência): %.12f - IC: (%.12f, %.12f) - P: %.12f\n", mut, Lt, Ut, pt);
        fprintf (resultados, "Média (Vazão): %.12f - IC: (%.12f, %.12f) - P: %.12f\n", muv, Lv, Uv, pv);
        fprintf (resultados, "Média (Pessoas): %.12f - IC: (%.12f, %.12f) - P: %.12f\n", mun, Ln, Un, pn);
        fprintf (resultados, "Média (Peers): %.12f - IC: (%.12f, %.12f) - P: %.12f\n", mup, Lp, Up, pp);

        if (cenario == 1)
        {
            fprintf (resultados, "pmf do Número total de usuários no sistema:\n");
            for (int i = 0; i < (int) tempoPorN.size(); ++i) fprintf (resultados, "\t%u: %.12f - IC: (%.12f, %.12f)\n", i, tempoPorN[i] / n, LpN[i], UpN[i]);
        }

        if (cenario == 2)
        {
            fprintf (resultados, "Tempos de download por rodada:\n");
            for (int i = 0; i < n; ++i)
            {
                sort(temposDownload[i].begin(), temposDownload[i].end());
                fprintf (resultados, "\tRodada %d:", i);
                for (int j = 0; j < (int) temposDownload[i].size(); ++j) fprintf (resultados, " %.12f", temposDownload[i][j]);
            }
            fprintf (resultados, "\n");
            temposDownload.clear();
        }
        fprintf (resultados, "\n");
    }

    return 0;

    /*
    geradorAleatorio g(seed);

    unsigned int r;
    long long k = seed;
    long long mul = 1000003;

    for (int i = 0; i <= (1 << 22); ++i)
    {
        k = (k * mul) % (g.RANDMAX + 1);
        r = g.randUniforme();

        if (k != r)
            printf ("%lld %u\n", k, r);
    }

    double lambda = 0.5;

    double y = 0;
    for (int i = 0; i < 5; ++i)
    {
        double x = 0;
        for (int i = 0; i < (1 << 22); ++i)
        {
            x += g.randExponencial(lambda);
        }

        y += x / (1 << 22);

        printf ("%f\n", x / (1<<22));
    }

    printf (">> %f\n", y / 30);

    pessoa pub(pessoa::PUBLISHER);

    std::set<evento*> s;

    s.insert(new eventoTransmissao(g.randExponencial(lambda), &pub));
    for (int i = 0; i < 20; ++i)
    {
        unsigned int p = g.randUniforme() % 3;
        if (p == 0)
            s.insert(new eventoChegadaPeer(g.randExponencial(lambda)));
        else if (p == 1)
            s.insert(new eventoTransmissao(g.randExponencial(lambda), new pessoa(pessoa::PEER)));
        else
            s.insert(new eventoSaidaSeed(g.randExponencial(lambda), new pessoa(pessoa::SEED)));
    }

    for (std::set<evento*>::iterator it = s.begin(); it != s.end(); ++it)
    {
        evento *i = *it;
        printf ("%f %d %s --> %u -->\t", i->tempo(), i->tipo(), i->strTipo().c_str(), i->id());
        if (i->tipo() == evento::CHEGADA_PEER)
        {
            eventoChegadaPeer a = dynamic_cast<eventoChegadaPeer&>(*i);
            printf ("Chegou peer novo na parada! \\o/\n");
        }
        else if (i->tipo() == evento::TRANSMISSAO)
        {
            eventoTransmissao a = dynamic_cast<eventoTransmissao&>(*i);
            pessoa p = a.origem();

            printf ("Origem: %u, %s. Arquivo: %x. Faltam %u blocos.\n", p.id(), p.strTipo().c_str(), p.blocos(), p.blocosFaltantes());
        }
        else if (i->tipo() == evento::SAIDA_PEER)
        {
            eventoSaidaSeed a = dynamic_cast<eventoSaidaSeed&>(*i);
            printf ("Cara %d vai embora. =(\n", a.seed().id());
        }
    }

    return 0;*/
}

