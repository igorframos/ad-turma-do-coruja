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

        int ba; // Variável só para tirar o warning de valor de retorno ignorado

        ba = fscanf (cenarios, "%u", &cenario); ++ba;
        if (!cenario) break;

        ba = fscanf (cenarios, "%x %lf %lf", &arquivo, &lambda, &mu); ++ba;
        ba = fscanf (cenarios, "%lf %lf %lf %d", &U, &gamma, &pRec, &populacaoInicial); ++ba;
        ba = fscanf (cenarios, " %c %c", &politicaPeer, &politicaBloco); ++ba;

        if (politicaPeer == 'r') pPeer = filaEventos::RANDOM_PEER;
        if (politicaBloco == 'r') pBloco = filaEventos::RANDOM_PIECE;
        else pBloco = filaEventos::RAREST_FIRST;

        evento::nextId = 0;
        pessoa::nextId = 0;
        pessoa::arqCompleto = arquivo;
        
        printf ("Começarei o cenário %d com arquivo %x, lambda %.1f, mu %.1f, U %.1f, gamma %e e população inicial de %d\n", cenario, arquivo, lambda, mu, U, gamma, populacaoInicial);

        filaEventos f(1/lambda, 1/mu, 1/gamma, 1/U, pRec, pPeer, pBloco, populacaoInicial);

        double tempoDownload = 0, tempoDownload2 = 0, Ld, Ud, pd;
        double vazao = 0, vazao2 = 0, Lv, Uv, pv;
        double pessoas = 0, pessoas2 = 0, Ln, Un, pn;
        std::vector<double> tempoPorN(100);
        std::vector<std::vector<double> > temposDownload;
        int n = 0;

        while (f.haEvento())
        {
            f.trataProximoEvento();
            printf ("------------------------> Pessoas no sistema: %u Peers no sistema: %u\n", f.pessoasNoSistema(), f.peersNoSistema());
            printf ("%u chegadas e %u saídas até o momento.\n", f.chegadasTotais(), f.saidasTotais());

            if (!f.fimRodada()) continue;

            ++n;
            std::vector<double> tempoN = f.tempoPorN();
            double mediaDownload = f.mediaDownload();
            double mediaVazao = f.mediaVazao();
            double mediaN = f.mediaPessoas();
            temposDownload.push_back(f.temposDeDownload());

            tempoDownload += mediaDownload;
            tempoDownload2 += mediaDownload * mediaDownload;
            vazao += mediaVazao;
            vazao2 += mediaVazao * mediaVazao;
            pessoas += mediaN;
            pessoas2 += mediaN * mediaN;

            for (int i = 0; i < (int) tempoN.size(); ++i)
            {
                if ((int) tempoPorN.size() <= i)
                {
                    tempoPorN.push_back(tempoN[i]);
                }
                else
                {
                    tempoPorN[i] += tempoN[i];
                }
            }

            if (n < 2) continue;

            bool encerra = true;

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

            if (encerra == true) break;

            printf ("Rodada %d\n", n);
            printf ("\tDownload: %.15f (%.15f, %.15f) %.15f\n", tempoDownload / n, Ld, Ud, pd);
            printf ("\tVazão:    %.15f (%.15f, %.15f) %.15f\n", vazao / n, Lv, Uv, pv);
            printf ("\tPessoas:  %.15f (%.15f, %.15f) %.15f\n", pessoas / n, Ln, Un, pn);
        }

        printf ("Encerrei o cenário %d com arquivo %x, lambda %.1f e população inicial de %d\n", cenario, arquivo, lambda, populacaoInicial);

        double mud = tempoDownload / n;
        double muv = vazao / n;
        double mun = pessoas / n;
        fprintf (resultados, "Cenário: %d - Arquivo: %x - lambda: %.1f - População Inicial: %d - Rodadas: %d\n", cenario, arquivo, lambda, populacaoInicial, n);
        fprintf (resultados, "Média (Tempo de Download): %.15f - IC: (%.15f, %.15f) - P: %.15f\n", mud, Ld, Ud, pd);
        fprintf (resultados, "Média (Vazão): %.15f - IC: (%.15f, %.15f) - P: %.15f\n", muv, Lv, Uv, pv);
        fprintf (resultados, "Média (Pessoas): %.15f - IC: (%.15f, %.15f)\n - P: %.15f", mun, Ln, Un, pn);

        double t = 0;
        for  (int i = 0; i < (int) tempoPorN.size(); ++i) t += tempoPorN[i];

        fprintf (resultados, "pmf do Número total de usuários no sistema:\n");
        for (int i = 0; i < (int) tempoPorN.size(); ++i) fprintf (resultados, "\t%u: %.15f\n", i, tempoPorN[i] / t);

        fprintf (resultados, "Tempos de download por rodada:\n");
        for (int i = 0; i < n; ++i)
        {
            sort(temposDownload[i].begin(), temposDownload[i].end());
            fprintf (resultados, "\tRodada %d:", i);
            for (int j = 0; j < (int) temposDownload[i].size(); ++j) fprintf (resultados, " %.15f", temposDownload[i][j]);
        }
        printf ("\n\n");
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

