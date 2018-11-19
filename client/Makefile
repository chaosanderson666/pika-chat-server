CC=cc
TARGET=piki

SOURCES=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,${SOURCES})

#%.o : %.c
#	$(CC) -c ${CFLAGS} $< -o $@

${TARGET} : ${OBJS}
	${CC} -O -o ${TARGET} ${OBJS}
	rm -f *.o

.PHONY: c
c:
	rm -f *.o ${TARGET}
