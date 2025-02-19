test: test.cpp load_obj.cpp
	g++ test.cpp load_obj.cpp -o test -lglad -lglfw
# -fsanitize=address,null -g



