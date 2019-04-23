COPT='-Ofast -march=native -pthread'

g++ $COPT -c BattleAI.cpp -o BattleAI.o
g++ $COPT -c CaseGenerator.cpp -o CaseGenerator.o
g++ $COPT -c Exec.cpp -o Exec.o
g++ $COPT -c Game.cpp -o Game.o
g++ $COPT -c Input.cpp -o Input.o
g++ $COPT -c Main.cpp -o Main.o

g++ -O3 Main.o Input.o Game.o Exec.o CaseGenerator.o BattleAI.o -lm -lpthread -o ./codevs

./codevs
