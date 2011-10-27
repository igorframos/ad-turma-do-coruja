#include "filaEventos.h"

filaEventos::filaEventos(double lambda, double mu) : lambda(lambda), mu(mu), tAtual(0)
{
    agendaChegadaPeer(tAtual);
    agendaTransmissao(tAtual, publisher);
}

void filaEventos::agendaChegadaPeer(double t)
{
    t += g.randExponencial(lambda);
    insereEvento(new eventoChegadaPeer(t));
}

void filaEventos::agendaSaidaSeed(double t, const pessoa& p)
{
    t += g.randExponencial(gamma);
    insereEvento(new eventoSaidaSeed(t, p));
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

    insereEvento(new eventoTransmissao(t, p));
}

void filaEventos::insereEvento(const evento* e)
{
    std::list<evento*>::iterator it = fila.begin();

    while (**it < *e) ++it;

    fila.insert(it, e);
}

void filaEventos::trataProximoEvento()
{
    std::list<evento*>::iterator it = fila.begin();

    // Terminar aqui. Chamar uma função de tratamento específica para cada cara.
}

