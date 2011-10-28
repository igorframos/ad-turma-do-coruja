#include "include.h"

int main(int argc, char *argv[])
{
    unsigned int seed = time(NULL);
    evento::nextId = 0;
    pessoa::nextId = 0;
    pessoa::arqCompleto = 0x3;

    filaEventos f(1/0.5, 1/0.1, 1/0.1, 1/1, 0.5, filaEventos::RANDOM_PEER, filaEventos::RANDOM_PIECE);

    int ini = time(NULL);
    while (f.haEvento())
    {
        f.trataProximoEvento();
        printf ("--------------------------> Pessoas no sistema: %u\n", f.pessoasNoSistema());
        if (time(NULL) - ini > 600) break;
    }

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
    */

    return 0;
}

