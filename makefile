test: test.o
	g++ test.o load_obj.o -o test -lglad -lglfw

test.o: test.cpp load_obj.o setShaders.h
	g++ -c test.cpp

load_obj.o: load_obj.cpp load_obj.h
	g++ -c load_obj.cpp


