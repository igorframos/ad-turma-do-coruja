#include "include.h"

int main(int argc, char *argv[])
{
    unsigned int seed = time(NULL);

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

    return 0;
}

