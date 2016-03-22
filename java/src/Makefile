JC		= javac
JSRC		= $(wildcard *.java)
CLSS		= $(JSRC:%.java=../bin/%.class)
JEXE		= ../bin/Jring.class




all		: $(JEXE)

../bin/%.class 	: %.java
	$(JC) -o $@ $<

clean		:
	rm -f $(JEXE) $(CLSS)
