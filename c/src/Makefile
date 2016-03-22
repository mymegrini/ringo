CC		= gcc
CFLAGS 		= -Wall --pedantic -O2 #-std=c11
CSRC		= $(wildcard *.c)
OBJS		= $(CSRC:%.c=../bin/%.o)
CEXE		= ../bin/Cring
LIBLINKS	= -pthread



all		: $(CEXE)

$(CEXE)		: $(OBJS)
	$(CC) -o $@ $^ $(LIBLINKS)

../bin/%.o 	: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean		:
	rm -f $(CEXE) $(OBJS)
