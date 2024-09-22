#Delete some un-needed files
#kill -SIGTERM $(ps -l | grep -v Z | grep -v defunct | awk '{print $1}')
rm *.o
rm *.out
rm *.bin

echo "Compile PIDpipe.cpp"
g++ -Wall -o manager.o PIDpipe.cpp

echo "Run the progam"
./manager.o