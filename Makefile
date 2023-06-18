NAME = editor
CC = gcc
C_FILES = $(wildcard *.c)
# TODO: do not output .o files
OBJS = $(patsubst %.c, %.o, $(C_FILES))
INCLUDE = -Iinclude/ # You should add your include directory here.
CFLAGS = -g3 `pkg-config --cflags gtk4`
LFLAGS = `pkg-config --libs gtk4`


all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(OBJS) -o $(NAME) $(CFLAGS) $(LFLAGS)

%.o: %.c %.h
	$(CC) $(CFLAGS) $(LFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS); rm -f $(NAME)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: clean fclean re all
.SILENT: $(OBJ)
