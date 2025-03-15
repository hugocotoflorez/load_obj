SRC = test.cpp load_obj.cpp load_mtl.cpp display_obj.cpp
LIBS = load_mtl.h load_obj.h display_obj.h

all:
	sudo systemctl stop keyd
	make test
	./test ; sudo systemctl start keyd ; rm test

test: $(SRC) $(LIBS) makefile stb_image.h
	g++ $(SRC) -o test -lglad -lglfw -ggdb

stb_image.h:
	wget https://raw.githubusercontent.com/nothings/stb/refs/heads/master/stb_image.h



