test: test.cpp load_obj.cpp makefile stb_image.h
	g++ test.cpp load_obj.cpp -o test -lglad -lglfw -g

stb_image.h:
	wget https://raw.githubusercontent.com/nothings/stb/refs/heads/master/stb_image.h



