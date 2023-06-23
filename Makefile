NAME = editor
CC = gcc
# TODO: WHY IS resources.c TWO TIMES
C_FILES = $(wildcard *.c)
OBJS = $(patsubst %.c, %.o, $(C_FILES))
RES_DIR = ./res/
RES_FILE = ./.gresource.xml
INCLUDE = -Iinclude/ # You should add your include directory here.
CFLAGS = -g3 `pkg-config --cflags gtk4` `pkg-config --cflags libadwaita-1`
LFLAGS = `pkg-config --libs gtk4` `pkg-config --libs libadwaita-1`


all: $(NAME)

$(NAME): $(OBJS) resources.o
	$(CC) $(OBJS) -o $(NAME) $(CFLAGS) $(LFLAGS) && rm resources.c ./*.o

%.o: %.c %.h resources.c
	$(CC) $(CFLAGS) $(LFLAGS) -c -o $@ $<

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
	rm -f $(OBJS) $(NAME) resources.c $(RES_DIR)/*.ui

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: clean fclean re all
.SILENT: $(OBJ)
