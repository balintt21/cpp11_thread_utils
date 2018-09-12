CROSS_COMPILE = arm-intellio-linux-gnueabihf-
CC = g++
CC_FLAGS = -O3 -Wall
LD_FLAGS = -lpthread
NAME = thread_utils
SOURCE_DIR = ./
SOURCE_FILES = $(shell find $(SOURCE_DIR) -type f -iregex '.*\.\(c\|i\|ii\|cc\|cp\|cxx\|cpp\|CPP\|c++\|C\|s\|S\|sx\)' )
OUTPUT_DIR = ./test

all: $(OUTPUT_DIR) $(SOURCE_FILES) 
	$(CC) $(CC_FLAGS) $(SOURCE_FILES) -o "$(OUTPUT_DIR)/$(NAME)" $(LD_FLAGS)

arm: $(OUTPUT_DIR) $(SOURCE_FILES)
	$(CROSS_COMPILE)$(CC) $(CC_FLAGS) $(SOURCE_FILES) -o "$(OUTPUT_DIR)/$(NAME)_arm" $(LD_FLAGS)


.PHONY: clean
clean: 
	rm -r $(OUTPUT_DIR)

$(OUTPUT_DIR):
	mkdir $(OUTPUT_DIR)