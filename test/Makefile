CC = g++
CC_FLAGS = -O3 -Wall -std=c++14 -I../src
LD_FLAGS = -lpthread
NAME = thread_utils_test
SOURCE_DIR = ../
SOURCE_FILES = $(shell find $(SOURCE_DIR) -type f -iregex '.*\.\(c\|i\|ii\|cc\|cp\|cxx\|cpp\|CPP\|c++\|C\|s\|S\|sx\)' )
OUTPUT_DIR = ./

all: $(OUTPUT_DIR) $(SOURCE_FILES) 
	$(CC) $(CC_FLAGS) $(SOURCE_FILES) -o "$(OUTPUT_DIR)/$(NAME)" $(LD_FLAGS)

.PHONY: clean
clean: 
	rm $(OUTPUT_DIR)/$(NAME)

$(OUTPUT_DIR):
	mkdir $(OUTPUT_DIR)