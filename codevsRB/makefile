CC		= g++
CFLAGS	= -O3
LDFLAGS	= -L/usr/local/lib
LIBS	= -lm
OBJS	= Main.o Input.o Game.o Exec.o CaseGenerator.o BattleAI.o
PROGRAM	= codevs

all:		$(PROGRAM)

%.o:		%.cpp
			$(CC) -c $(CFLAGS) $< -o $@

$(PROGRAM):	$(OBJS)
			$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) $(LIBS) -o $(PROGRAM)

clean:;		rm *.o *~ $(PROGRAM)
