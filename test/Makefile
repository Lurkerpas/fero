.PHONY: all test clean

BUILD_DIR = ../build

FERO_SOURCES = \
	../src/fero.c

FERO_HEADERS = \
	../src/fero.h

UNITY_SOURCES = \
	../libs/Unity/src/unity.c

C_FLAGS = -Wall

all: test

test: $(BUILD_DIR) $(BUILD_DIR)/test_fero

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/test_fero: $(FERO_SOURCES) $(FERO_HEADERS) $(UNITY_SOURCES) test_fero.c
	gcc $(C_FLAGS) test_fero.c $(UNITY_SOURCES) $(FERO_SOURCES) -o $(BUILD_DIR)/test_fero
	$(BUILD_DIR)/test_fero

clean:
	rm -r $(BUILD_DIR)
