PROG= capno1
OBJS= main.o Output.o Plot.o
CC= arm-angstrom-linux-gnueabi-g++
LD= arm-angstrom-linux-gnueabi-g++
CFLAGS= -Isdl/include
LDFLAGS= -Lsdl/lib -lSDL

all: ${PROG}

${PROG}: ${OBJS}
	${LD} ${LDFLAGS} ${OBJS} -o ${PROG}

main.o: main.cc
	${CC} ${CFLAGS} -c main.cc

Output.o: Output.cpp
	${CC} ${CFLAGS} -c Output.cpp

Plot.o: Plot.cc
	${CC} ${CFLAGS} -c Plot.cc




clean:
	rm ${OBJS}

distclean:
	rm ${PROG} ${OBJS}
