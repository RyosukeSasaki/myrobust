SRCS = sender.c
SRCC = receiver.c
EXES = sender
EXEC = receiver
CFLAGS := -std=gnu99

all: server client

server: $(SRCS)
	-@gcc $(CFLAGS) $(SRCS) -o $(EXES)

client: $(SRCC)
	-@gcc $(CFLAGS) $(SRCC) -o $(EXEC)

clean: rmserv rmclient

rmserv:
	-@rm $(EXES)

rmclient:
	-@rm $(EXEC)

verify: all clean

-include $(DEPS)