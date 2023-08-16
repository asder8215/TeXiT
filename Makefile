NAME = texit
CC = gcc
BUILD_DIR = ./build/
SRC_DIR = ./src/
RES_DIR = ./res/
RES_FILE = $(BUILD_DIR)/.gresources.xml
BLPS = $(wildcard $(RES_DIR)/*.blp)
GUIS = $(addprefix $(BUILD_DIR)/, $(patsubst %.blp, %.ui, $(BLPS)))
C_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(addprefix $(BUILD_DIR)/, $(patsubst $(SRC_DIR)/%.c, %.o, $(C_FILES))) $(BUILD_DIR)/resources.o
LIBS = json-c # The names of directories that are subprojects (libraries)
INCLUDE = -Iinclude
CFLAGS = `pkg-config --cflags gtk4` `pkg-config --cflags libadwaita-1` "-Wl,--export-dynamic" $(addprefix -I$(BUILD_DIR)/include/, $(LIBS))
LFLAGS = `pkg-config --libs gtk4` `pkg-config --libs libadwaita-1` `pkg-config --libs gmodule-export-2.0` -L$(BUILD_DIR)/lib64 $(addprefix -l, $(LIBS))


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

# Any other resource file that is not compiled into something else
rest_res = $(shell ls -1 $(RES_DIR) | grep -v .blp | xargs)
$(BUILD_DIR)/resources.c: $(RES_FILE) $(GUIS) $(BUILD_DIR)/$(RES_DIR)
	cp $(addprefix $(RES_DIR)/, $(rest_res)) -t $(BUILD_DIR)/$(RES_DIR) &&\
	glib-compile-resources $(RES_FILE) --target="$@" --sourcedir=$(BUILD_DIR)/$(RES_DIR) --generate-source

$(RES_FILE): $(BUILD_DIR)
	touch $@ &&\
	echo '<?xml version="1.0" encoding="UTF-8"?><gresources><gresource prefix="/me/Asder8215/TeXiT">' > $@;\
	for file in $(GUIS) $(rest_res); do\
		echo "<file>$$(basename $$file)</file>" >> $@;\
	done;\
	echo "</gresource></gresources>" >> $@

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
