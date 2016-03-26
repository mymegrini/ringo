JC		= javac
JSRC		= $(wildcard *.java)
CLSS		= $(JSRC:%.java=../bin/%.class)
BIN		= ../bin



all		: $(CLSS)

../bin/%.class 	: %.java
	$(JC) -d $(BIN) $<

clean		:
	rm -f $(JEXE) $(CLSS)
