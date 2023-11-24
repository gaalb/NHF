@ECHO OFF

gcc -g -Wall -Werror -ISDL2\MinGW\include -c custom_renders.c -o custom_renders.o
gcc -g -Wall -Werror -ISDL2\MinGW\include -c engine.c -o engine.o
gcc -g -Wall -Werror -ISDL2\MinGW\include -c leaderboard.c -o leaderboard.o
gcc -g -Wall -Werror -ISDL2\MinGW\include -c main.c -o main.o
gcc -g -Wall -Werror -ISDL2\MinGW\include -c text.c -o text.o
gcc.exe -LSDL2\MinGW\lib -o NHF.exe *.o   -lmingw32 -lSDL2main -lSDL2 -lSDL2_gfx -lSDL2_image -lSDL2_mixer -lSDL2_ttf -luser32 -lgdi32 -lwinmm -ldxguid

PAUSE