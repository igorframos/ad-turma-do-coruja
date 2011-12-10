OBJS = obj/main.o obj/evento.o obj/eventoChegadaPeer.o obj/eventoSaidaSeed.o obj/eventoTransmissao.o obj/pessoa.o obj/filaEventos.o obj/geradorAleatorio.o
CC = g++
DEBUG =
CFLAGS = -Wall -funroll-loops -c
LFLAGS = -Wall -lm

bin/sim: $(OBJS)
	$(CC) -o bin/sim $(OBJS) $(LFLAGS) $(DEBUG)

src/eventoChegadaPeer.h: src/evento.h

src/eventoSaidaSeed.h: src/evento.h src/pessoa.h

src/eventoTransmissao.h: src/evento.h src/pessoa.h

src/filaEventos.h: src/evento.h src/eventoChegadaPeer.h src/eventoSaidaSeed.h src/eventoTransmissao.h src/pessoa.h src/geradorAleatorio.h

src/include.h: src/evento.h src/pessoa.h src/geradorAleatorio.h src/filaEventos.h src/eventoChegadaPeer.h src/eventoSaidaSeed.h src/eventoTransmissao.h

obj/main.o: src/main.cpp src/include.h
	cd obj && $(CC) $(CFLAGS) $(DEBUG) ../src/main.cpp && cd ..

obj/evento.o: src/evento.cpp src/evento.h
	cd obj && $(CC) $(CFLAGS) $(DEBUG) ../src/evento.cpp && cd ..

obj/eventoChegadaPeer.o: src/eventoChegadaPeer.cpp src/eventoChegadaPeer.h
	cd obj && $(CC) $(CFLAGS) $(DEBUG) ../src/eventoChegadaPeer.cpp && cd ..

obj/eventoSaidaSeed.o: src/eventoSaidaSeed.cpp src/eventoSaidaSeed.h
	cd obj && $(CC) $(CFLAGS) $(DEBUG) ../src/eventoSaidaSeed.cpp && cd ..

obj/eventoTransmissao.o: src/eventoTransmissao.cpp src/eventoTransmissao.h
	cd obj && $(CC) $(CFLAGS) $(DEBUG) ../src/eventoTransmissao.cpp && cd ..

obj/pessoa.o: src/pessoa.cpp src/pessoa.h
	cd obj && $(CC) $(CFLAGS) $(DEBUG) ../src/pessoa.cpp && cd ..

obj/filaEventos.o: src/filaEventos.cpp src/filaEventos.h
	cd obj && $(CC) $(CFLAGS) $(DEBUG) ../src/filaEventos.cpp && cd ..

obj/geradorAleatorio.o: src/geradorAleatorio.cpp src/geradorAleatorio.h
	cd obj && $(CC) $(CFLAGS) $(DEBUG) ../src/geradorAleatorio.cpp && cd ..

run: bin/sim
	time ./bin/sim cenarios.txt

vrun: bin/sim
	time ./bin/sim cenarios.txt -v

debug: bin/sim
	gdb bin/sim

graph: run
	gnuplot <log/plotInfo.txt
	rm log/*Vazao.txt

vgraph: vrun
	gnuplot <log/plotInfo.txt
	rm log/*Vazao.txt

clean:
	rm obj/*.o bin/sim log/*

