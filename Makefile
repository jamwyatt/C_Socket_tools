
LIB=libNetTools.a

all: $(LIB)

OBJS=connectionTools.o error.o misc.o 

$(LIB): $(OBJS)
		ar -rcs $(LIB) $(OBJS)

clean:
	rm -f *.o $(LIB)
