
mkdir -p build

gcc reloader.c -g -static -o ./build/reloader.exe -I../core -Wno-incompatible-pointer-types 

gcc test.c ../core/audio.cpp -g -static -o ./build/test.dll -shared -I../core -Wno-incompatible-pointer-types -lgdi32 -lopengl32 -lwinmm -lole32

# gcc test.c -g -static -o ./build/test.exe -I../core -Wno-incompatible-pointer-types -lgdi32 -lopengl32

