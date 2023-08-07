NAME = texit
CC = gcc
# TODO: WHY IS resources.c TWO TIMES
C_FILES = $(wildcard *.c)
OBJS = $(patsubst %.c, %.o, $(C_FILES))
BUILD_DIR = ./build/
RES_DIR = ./res/
RES_FILE = ./.gresource.xml
LIBS = json-c # The names of directories that are subprojects (libraries)
INCLUDE = -Iinclude
CFLAGS = `pkg-config --cflags gtk4` `pkg-config --cflags libadwaita-1` $(addprefix -I$(BUILD_DIR)/include/, $(LIBS))
LFLAGS = `pkg-config --libs gtk4` `pkg-config --libs libadwaita-1` -L$(BUILD_DIR)/lib64 $(addprefix -l, $(LIBS))


all: $(NAME)

$(NAME): $(BUILD_DIR)/lib64/libjson-c.a $(OBJS) resources.o
	$(CC) $(INCLUDE) $(OBJS) -o $(NAME) $(CFLAGS) $(LFLAGS) && rm resources.c ./*.o

$(BUILD_DIR)/lib64/libjson-c.a:
	LIB="json-c";\
	mkdir -p $(BUILD_DIR)/$$LIB && cd $(BUILD_DIR)/$$LIB &&\
	cmake -DBUILD_SHARED_LIBS=OFF -DBUILD_STATIC_LIBS=ON -DCMAKE_BUILD_TYPE="debug" -DCMAKE_INSTALL_PREFIX="../" ../../$$LIB &&\
	make && make install &&\
	cd .. && rm -rf $$LIB &&\
	echo "$$LIB build completed!"

%.o: %.c resources.c
	$(CC) $(INCLUDE) $(CFLAGS) $(LFLAGS) -c -o $@ $<

# Adapted from https://stackoverflow.com/questions/28855850/gtk-c-and-gtkbuilder-to-make-a-single-executable
resources = $(shell glib-compile-resources --sourcedir=$(RES_DIR) --generate-dependencies $(RES_FILE))
# .blp files will not be prefixed with the directory they're in, so must add prefix.
guis = $(addprefix $(BUILD_DIR)/$(RES_DIR), $(shell echo "$(resources)" | tr ' ' '\n' | grep ".ui"))
rest_res = $(shell echo $(resources) | tr ' ' '\n' | grep -v .ui | xargs)
resources.c: $(RES_FILE) $(guis) $(rest_res)
	mkdir -p $(BUILD_DIR)/$(RES_DIR) && cp -t $(BUILD_DIR)/$(RES_DIR) $(rest_res) &&\
	glib-compile-resources $(RES_FILE) --target=resources.c --sourcedir=$(BUILD_DIR)/$(RES_DIR) --generate-source

%.ui:
	mkdir -p $(dir $@) &&\
	blueprint-compiler compile $(RES_DIR)/$(notdir $(patsubst %.ui, %.blp, $@)) --output $@

clean:
	rm -rf $(OBJS) $(NAME) resources.c $(RES_DIR)/*.ui build 

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: clean fclean re all
.SILENT: $(OBJ)
