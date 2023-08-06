NAME = texit
CC = gcc
# TODO: WHY IS resources.c TWO TIMES
C_FILES = $(wildcard *.c)
OBJS = $(patsubst %.c, %.o, $(C_FILES))
RES_DIR = ./res/
RES_FILE = ./.gresource.xml
INCLUDE = -Iinclude
CFLAGS = `pkg-config --cflags gtk4` `pkg-config --cflags libadwaita-1` -Ibuild/include/json-c
LFLAGS = `pkg-config --libs gtk4` `pkg-config --libs libadwaita-1` -Lbuild/lib64 -ljson-c


all: $(NAME)

$(NAME): json-c-install $(OBJS) resources.o
	$(CC) $(INCLUDE) $(OBJS) -o $(NAME) $(CFLAGS) $(LFLAGS) && rm resources.c ./*.o

json-c-install:
	if [ ! -d "build/include/json-c" ]; then (\
		mkdir -p build/json-c-build && cd build/json-c-build &&\
		cmake -DBUILD_SHARED_LIBS=OFF -DBUILD_STATIC_LIBS=ON -DCMAKE_BUILD_TYPE="debug" -DCMAKE_INSTALL_PREFIX="../" ../../json-c &&\
		make && make install &&\
		cd .. && rm -rf json-c-build &&\
		echo "Json-C build completed!"\
	); else (\
		echo "Skipping json-c build"\
	); fi

%.o: %.c resources.c
	$(CC) $(INCLUDE) $(CFLAGS) $(LFLAGS) -c -o $@ $<

# Adapted from https://stackoverflow.com/questions/28855850/gtk-c-and-gtkbuilder-to-make-a-single-executable
resources = $(shell glib-compile-resources --sourcedir=$(RES_DIR) --generate-dependencies $(RES_FILE))
blueprints = $(shell echo $(resources) | tr ' ' '\n' | sed 's/.ui/.blp/g' | grep .blp)
rest_res = $(shell echo $(resources) | tr ' ' '\n' | grep -v .ui | xargs)
resources.c: $(RES_FILE) $(blueprints) $(rest_res)
	glib-compile-resources $(RES_FILE) --target=resources.c --sourcedir=$(RES_DIR) --generate-source\
	&& rm $(RES_DIR)/*.ui

%.blp:
	blueprint-compiler compile $(RES_DIR)/$@ --output $(RES_DIR)/$(shell basename -s .blp "$@").ui
# resources will not have the sourcedir in the path for .blp files, so must use `$(RES_DIR)/$@`

clean:
	rm -rf $(OBJS) $(NAME) compile_commands.json resources.c $(RES_DIR)/*.ui build 

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: clean fclean re all
.SILENT: $(OBJ)
