#include <cstdio>
#include <cstring>
#include <cmath>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

int main (int argc, char *argv[])
{
    char name[2048];
    FILE* arq;

    int n;
    double soma, soma2, l, u, p, mu, sigma;
    soma = 0;
    soma2 = 0;

    for (n = 1; n <= 100; ++n)
    {
        sprintf (name, "%s%02d.txt", argv[1], n);
        arq = fopen(name, "r");

        double last, plast, at, pat;

        plast = 0;
        while (fscanf (arq, "%lf %lf", &at, &pat) == 2)
        {
            if (pat >= 0.5)
            {
                break;
            }

            last = at;
            plast = pat;
        }

        fclose(arq);

        if (pat == 0.5)
        {
            soma += at;
            soma2 += pow(at, 2);
        }
        else
        {
            double d1 = 0.5 - plast,
                   d2 = pat - 0.5;

            d1 = 1 / d1;
            d2 = 1 / d2;

            double x = (d1 * last + d2 * at) / (d1 + d2);
            
            soma += x;
            soma2 += pow(x, 2);
        }
    }

    mu = soma / n;
    sigma = sqrt((soma2 - n * mu * mu) / (n - 1));
    l = mu - 1.96 * sigma / sqrt(n);
    u = mu + 1.96 * sigma / sqrt(n);
    p = 100 * 1.96 * sigma / (mu * sqrt(n));

    printf ("Mediana: %.12f\n", mu);
    printf ("Intervalo: %.12f %.12f\n", l, u);
    printf ("Tamanho: %.12f\n", p);

    return 0;
}

