CC = gcc
CFLAGS = -Wall -Wextra -O2 -pthread -Iinclude

SRC_DIR = src
INCLUDE_DIR = include
LOG_DIR = logs
WWW_DIR = www

TARGET = server

SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/server.c $(SRC_DIR)/client.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET) dirs

$(TARGET): $(OBJS)
    $(CC) $(CFLAGS) -o $(TARGET) $(OBJS)
$(SRC_DIR)/main.o: $(SRC_DIR)/main.c
    $(CC) $(CFLAGS) -c $< -o $@
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(INCLUDE_DIR)/%.h
    $(CC) $(CFLAGS) -c $< -o $@

dirs:
    @mkdir -p $(LOG_DIR) $(WWW_DIR)
    @if [ ! -f $(WWW_DIR)/index.html ]; then \
        echo "<!DOCTYPE html>" > $(WWW_DIR)/index.html; \
        echo "<html><head><title>Sputnik Server</title></head>" >> $(WWW_DIR)/index.html; \
        echo "<body><h1>Welcome to Sputnik Web Server!</h1>" >> $(WWW_DIR)/index.html; \
        echo "<p>This is the default index page.</p></body></html>" >> $(WWW_DIR)/index.html; \
    fi

clean:
    rm -f $(OBJS) $(TARGET)

clean-logs:
    rm -f $(LOG_DIR)/*.log

run: $(TARGET)
    ./$(TARGET) 8080

.PHONY: all clean clean-logs run dirs