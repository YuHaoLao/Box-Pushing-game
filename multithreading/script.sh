if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    g++ -Wall main.cpp gl_frontEnd.cpp -lm -lGL -lglut -lpthread -o robots
elif [[ "$OSTYPE" == "darwin"* ]]; then
    g++ main.cpp gl_frontEnd.cpp -lm -framework OpenGL -framework GLUT -o robots
fi
FILE="robotSimulOut";

NUM=1;
for((c=0;c<=$5-1;c++))
do
    
    ./robots $1 $2 $3 $4
    FNAME="$FILE$NUM"
    mv robotSimulOut.txt $FNAME.txt;
    NUM=$((NUM+1));
done