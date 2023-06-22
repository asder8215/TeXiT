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
resources = $(shell blueprint-compiler batch-compile $(RES_DIR) $(RES_DIR) $(RES_DIR)/*.blp\
					&& glib-compile-resources --sourcedir=$(RES_DIR) --generate-dependencies $(RES_FILE) | xargs)
# blueprints = $(shell $(resources) | sed 's/.ui/.blp/g' | grep .blp)
resources.c: $(RES_FILE) $(resources)
	glib-compile-resources $(RES_FILE) --target=resources.c --sourcedir=$(RES_DIR) --generate-source\
	&& rm $(RES_DIR)/*.ui

clean:
	rm -f $(OBJS) $(NAME) resources.c $(RES_DIR)/*.ui

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: clean fclean re all
.SILENT: $(OBJ)
