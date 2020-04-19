# Compiles mongoose
cd ./mongoose   || echo "No such directory as Mongoose, please clone properly"
gcc -c mongoose.c -o mongoose.o

# Builds FSMTP itself
mkdir ./build   || echo "Build directory already exists, proceeding"
cd ./build      || echo "Could not cd into build directory"
cmake ..
make