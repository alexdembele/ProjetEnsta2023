# Configuration pour MSYS 2
# Peut être modifié à votre convenance...
CXXFLAGS = -std=c++17
ifdef DEBUG
CXXFLAGS += -g -fopenmp -O0 -Wall -fbounds-check -pedantic -fsanitize=address 
CXXFLAGS2 = CXXFLAGS
else
CXXFLAGS2 = ${CXXFLAGS} -fopenmpc-O2 -march=native -Wall 
CXXFLAGS += -O3 -march=native -Wall
endif
LIB=-lsfml-graphics -lsfml-window -lsfml-main -lsfml-system