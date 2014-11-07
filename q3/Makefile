CC := arm-none-linux-gnueabi-gcc
LD := arm-none-linux-gnueabi-gcc
# CC := gcc
# LD := gcc
CFLAGS := -g -Wall -Werror -static
LDFLAGS := -static

OBJECTS := cpu-alloc.o

all: cpu-alloc


: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f cpu-alloc
	rm -f cpu-alloc.o

.PHONY: clean
