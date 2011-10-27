#include "include.h"

int main(int argc, char *argv[])
{
    unsigned int seed = time(NULL);
    evento::nextId = 0;
    pessoa::nextId = 0;
    pessoa::arqCompleto = 0xff;

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
    for (int i = 0; i < 30; ++i)
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

    std::set<evento> s;

    s.insert(eventoTransmissao(g.randExponencial(lambda), &pub));
    for (int i = 0; i < 20; ++i)
    {
        if (g.randUniforme() % 2)
            s.insert(eventoChegadaPeer(g.randExponencial(lambda)));
        else
            s.insert(eventoTransmissao(g.randExponencial(lambda), new pessoa(pessoa::PEER)));
    }

    for (std::set<evento>::iterator i = s.begin(); i != s.end(); ++i)
    {
        printf ("%f %d %s --> %u\n", i->tempo(), i->tipo(), i->strTipo().c_str(), i->id());
        if (i->tipo() == evento::CHEGADA_PEER)
        {
            eventoChegadaPeer a = (eventoChegadaPeer) *i;
            printf ("\tChegou peer novo na parada! \\o/\n");
        }
        else if (i->tipo() == evento::TRANSMISSAO)
        {
            eventoTransmissao a = (eventoTransmissao) *i;
            pessoa p = a.origem();

            printf ("\tOrigem: %u, %s. Arquivo: %x. Faltam %u blocos.\n", p.id(), p.strTipo().c_str(), p.blocos(), p.blocosFaltantes());
        }
    }

    printf ("%d\n", evento::CHEGADA_PEER);

    //std::list<evento>

    return 0;
}

