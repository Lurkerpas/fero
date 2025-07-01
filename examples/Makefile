.PHONY: all clean

BUILD_DIR=../build
FERO_DIR=../src

all: \
	$(BUILD_DIR)/ping_pong \
	$(BUILD_DIR)/ticker

clean:
	rm -r -f $(BUILD_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/ping_pong: $(BUILD_DIR) ping_pong.c
	gcc -o $(BUILD_DIR)/ping_pong -I$(FERO_DIR) ping_pong.c $(FERO_DIR)/fero.c
	$(BUILD_DIR)/ping_pong

$(BUILD_DIR)/ticker: $(BUILD_DIR) ticker.c
	gcc -o $(BUILD_DIR)/ticker -I$(FERO_DIR) ticker.c $(FERO_DIR)/fero.c
	$(BUILD_DIR)/ticker
