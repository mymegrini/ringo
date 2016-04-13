CC		= gcc
CFLAGS 		= -Wall -O1 -std=gnu99 #--pedantic -std=c11
CSRC		= $(wildcard *.c)
OBJS		= $(CSRC:%.c=../bin/%.o)
CEXE		= ../bin/Cring
LIBLINKS	= -pthread -lreadline




all		: $(CEXE)

debug: CFLAGS += -D DEBUG -g
debug: $(CEXE)

$(CEXE)		: $(OBJS)
	$(CC) -o $@ $^ $(LIBLINKS)

../bin/%.o 	: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean		:
	rm -f $(CEXE) $(OBJS)
