CC = gcc
CFLAGS = -Wall -Og
BUILDDIR = .build
default: rdps rdpr

rdpr: ${BUILDDIR}/rdpr.o ${BUILDDIR}/util.o
	${CC} -o rdpr ${CFLAGS} ${BUILDDIR}/rdpr.o ${BUILDDIR}/util.o

rdps: ${BUILDDIR}/rdps.o ${BUILDDIR}/util.o
	${CC} -o rdps ${CFLAGS} ${BUILDDIR}/rdps.o ${BUILDDIR}/util.o

${BUILDDIR}/rdpr.o: src/rdpr.c
	${CC} ${CFLAGS} -o ${BUILDDIR}/rdpr.o -c src/rdpr.c

${BUILDDIR}/rdps.o: src/rdps.c
	${CC} ${CFLAGS} -o ${BUILDDIR}/rdps.o -c src/rdps.c

${BUILDDIR}/util.o: util/util.c
	${CC} ${CFLAGS} -o ${BUILDDIR}/util.o -c util/util.c

clean:
	@rm -f ${BUILDDIR}/*
	@rm -f rdpr rdps
	@echo "Clean Successful!"