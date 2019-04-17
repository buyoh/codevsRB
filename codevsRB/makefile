CC		= g++
CFLAGS	= -O3
LIBS	= -lm
OBJS	= Main.o Input.o Game.o Exec.o CaseGenerator.o BattleAI.o
PROGRAM	= codevs

all:		$(PROGRAM)

%.o:		%.cpp
			$(CC) -c $(CFLAGS) $< -o $@

$(PROGRAM):	$(OBJS)
			$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $(PROGRAM)

clean:;		rm *.o *~ $(PROGRAM)
