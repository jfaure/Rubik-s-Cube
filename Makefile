NAME	=	cube
CXX	?=	clang++
CPPFLAGS=	-std=c++17 -g $(WNO)
LFLAGS	=	-lsfml-window -lsfml-system -lGL #      -lGLU -lglut -lpthread
WNO	=	-Wno-logical-op-parentheses 

SRC	=	Cube.cpp

OBJDIR	=	obj
OBJ	=	$(SRC:%.cpp=$(OBJDIR)/%.o)

all:
	$(CXX) -o $(NAME) $(CPPFLAGS) $(LFLAGS) $(SRC) &&  ./$(NAME)
