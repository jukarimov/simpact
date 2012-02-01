
OBJS = game.o

LIBS = -lncurses 
CFLAGS = -Wall -g

simpact: $(OBJS)
	cc $(LIBS) $< -o $@

clean:
	rm *.o simpact
