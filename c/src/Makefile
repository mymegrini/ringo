JC	= javac
JSRC	= $(wildcard *.java)
CLSS	= $(JSRC:%.java=../bin/%.class)
JEXE	= ../Jring.class

CC	= gcc
CFLAGS 	= -Wall --pedantic -O2 -std=c11
CSRC	= $(wildcard *.c)
OBJS	= $(CSRC:%.c=../bin%.o)
CEXE	= ../Cring

all	: $(JEXE) $(CEXE)

$(JEXE) : ../bin/$(JEXE)
	mv $< $@

../bin/%.class 	: %.java
	$(JC) $<

$(CEXE) : $(OBJS)
	$(CC) -o $@ $^

../bin/%.o 	: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean	:
	rm -f $(JEXE) $(CLSS) $(CEXE) $(OBJS)
