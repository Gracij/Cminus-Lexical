#
# Makefile for TINY
# Gnu C Version
# K. Louden 2/3/98
# Modified for C-

CC = gcc

CFLAGS = 

OBJS = main.o util.o lex.yy.o cm.tab.o  

cmc: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o cmc -lfl

main.o: main.c globals.h util.h scan.h parse.h cm.tab.h 
	$(CC) $(CFLAGS) -c main.c

util.o: util.c util.h globals.h cm.tab.h
	$(CC) $(CFLAGS) -c util.c

lex.yy.o: cm.tab.h lex.yy.c scan.h util.h globals.h
	$(CC) $(CFLAGS) -c lex.yy.c

cm.tab.o: cm.tab.h
	$(CC) $(CFLAGS) -c cm.tab.c

cm.tab.h: cm.y
	bison -dv cm.y

lex.yy.c: cm.l
	flex cm.l

tags: *.c Makefile
	ctags -R

clean:
	-rm cmc
	-rm $(OBJS)
	-rm cm.tab.*
	-rm lex.yy.c
	-rm cm.output

all: cmc

