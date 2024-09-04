CC = gcc
CFLAGS = -Wall -Wextra -O2 -pthread -Iinclude

SRC_DIR = src
INCLUDE_DIR = include
LOG_DIR = logs

TARGET = server

SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/server.c $(SRC_DIR)/client.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(INCLUDE_DIR)/%.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

clean-logs:
	rm -f $(LOG_DIR)/*.log

run: $(TARGET)
	./$(TARGET) 8080
