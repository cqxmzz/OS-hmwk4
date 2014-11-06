CC := arm-none-linux-gnueabi-gcc
LD := arm-none-linux-gnueabi-gcc
CFLAGS := -g -Wall -Werror -static
LDFLAGS := -static

OBJECTS := test2.o

all: test2 


: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS)

%.o: %.c 
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f test2 
	rm -f test2.o

.PHONY: clean
