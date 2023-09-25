
mkdir build

gcc reloader.c -g -o ./build/reloader.exe -I../core -Wno-incompatible-pointer-types 

gcc test.c -g -o ./build/test.dll -shared -I../core -Wno-incompatible-pointer-types -lgdi32 -lopengl32
