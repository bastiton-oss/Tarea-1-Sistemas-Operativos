CC = gcc
CFLAGS = -Wall -Wextra -g

SRCS = mishell.c R2-ComandosInternos.c R3.c R4.c R5.c pmon.c
HEADERS = headers.h
TARGET = mishell

$(TARGET): $(SRCS) $(HEADERS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)
