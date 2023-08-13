NAME = texit
CC = gcc
BUILD_DIR = ./build/
SRC_DIR = ./src/
RES_DIR = ./res/
RES_FILE = ./.gresource.xml
BLPS = $(wildcard $(RES_DIR)/*.blp)
GUIS = $(addprefix $(BUILD_DIR)/, $(patsubst %.blp, %.ui, $(BLPS)))
C_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(addprefix $(BUILD_DIR)/, $(patsubst $(SRC_DIR)/%.c, %.o, $(C_FILES))) $(BUILD_DIR)/resources.o
LIBS = json-c # The names of directories that are subprojects (libraries)
INCLUDE = -Iinclude
CFLAGS = `pkg-config --cflags gtk4` `pkg-config --cflags libadwaita-1` $(addprefix -I$(BUILD_DIR)/include/, $(LIBS))
LFLAGS = `pkg-config --libs gtk4` `pkg-config --libs libadwaita-1` -L$(BUILD_DIR)/lib64 $(addprefix -l, $(LIBS))


all: $(NAME)

$(NAME): $(BUILD_DIR)/lib64/libjson-c.a $(OBJS)
	$(CC) $(INCLUDE) $(OBJS) -o $(NAME) $(CFLAGS) $(LFLAGS)

$(BUILD_DIR)/lib64/libjson-c.a: json-c
	LIB="json-c";\
	mkdir -p $(BUILD_DIR)/$$LIB-build && cd $(BUILD_DIR)/$$LIB-build &&\
	cmake -DBUILD_SHARED_LIBS=OFF -DBUILD_STATIC_LIBS=ON -DCMAKE_BUILD_TYPE="debug" -DCMAKE_INSTALL_PREFIX="../" ../../$$LIB &&\
	make && make install &&\
	cd .. && rm -rf $$LIB-build

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(BUILD_DIR)/$(RES_DIR)
	$(CC) $(INCLUDE) $(CFLAGS) $(LFLAGS) -c -o $@ $<
$(BUILD_DIR)/resources.o: $(BUILD_DIR)/resources.c $(BUILD_DIR)/$(RES_DIR)
	$(CC) $(INCLUDE) $(CFLAGS) $(LFLAGS) -c -o $@ $<

# Adapted from https://stackoverflow.com/questions/28855850/gtk-c-and-gtkbuilder-to-make-a-single-executable
resources = $(shell glib-compile-resources --sourcedir=$(RES_DIR) --generate-dependencies $(RES_FILE))
rest_res = $(shell echo $(resources) | tr ' ' '\n' | grep -v .ui | xargs)
$(BUILD_DIR)/resources.c: $(RES_FILE) $(GUIS) $(rest_res) $(BUILD_DIR)/$(RES_DIR)
	cp $(rest_res) -t $(BUILD_DIR)/$(RES_DIR) &&\
	glib-compile-resources $(RES_FILE) --target="$@" --sourcedir=$(BUILD_DIR)/$(RES_DIR) --generate-source

$(BUILD_DIR)/$(RES_DIR)/%.ui: $(RES_DIR)/%.blp $(BUILD_DIR)/$(RES_DIR)
	blueprint-compiler compile $< --output $@

$(BUILD_DIR)/$(RES_DIR):
	mkdir -p $@
$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(NAME) build

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: clean fclean re all
.SILENT: $(OBJ)
