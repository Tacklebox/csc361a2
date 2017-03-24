CC = gcc
CFLAGS = -Wall -Og
BUILDDIR = .build
MKDIR_P = mkdir -p

.PHONY: directories

default: rdps rdpr

rdpr: directories ${BUILDDIR}/rdpr.o ${BUILDDIR}/util.o ${BUILDDIR}/pqueue.o
	${CC} -o rdpr ${CFLAGS} ${BUILDDIR}/rdpr.o ${BUILDDIR}/util.o ${BUILDDIR}/pqueue.o

rdps: directories ${BUILDDIR}/rdps.o ${BUILDDIR}/util.o ${BUILDDIR}/pqueue.o
	${CC} -o rdps ${CFLAGS} ${BUILDDIR}/rdps.o ${BUILDDIR}/util.o ${BUILDDIR}/pqueue.o

${BUILDDIR}/rdpr.o: src/rdpr.c
	${CC} ${CFLAGS} -o ${BUILDDIR}/rdpr.o -c src/rdpr.c

${BUILDDIR}/rdps.o: src/rdps.c
	${CC} ${CFLAGS} -o ${BUILDDIR}/rdps.o -c src/rdps.c

${BUILDDIR}/util.o: util/util.c
	${CC} ${CFLAGS} -o ${BUILDDIR}/util.o -c util/util.c 

${BUILDDIR}/pqueue.o: util/pqueue.c
	${CC} ${CFLAGS} -o ${BUILDDIR}/pqueue.o -c util/pqueue.c 

directories:
	${MKDIR_P} ${BUILDDIR}

clean:
	@rm -rf ${BUILDDIR}
	@rm -f rdpr rdps
	@echo "Clean Successful!"
