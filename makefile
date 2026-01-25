exec:	main.cpp buffer.cpp buffer.h editor.cpp editor.h
	clang++ -Wall -g main.cpp buffer.cpp editor.cpp -lncurses -o exec