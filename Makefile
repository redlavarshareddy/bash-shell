SOURCES = prompt.c time.c built_in.c ls.c execute.c user_details.c basic.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = shell
CC = gcc
CFLAGS += -g -O3 -march=native -Wno-unused-result
.PHONY: all build-shell clean

all: build-shell
 
build-shell: $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm *.o $(TARGET)

