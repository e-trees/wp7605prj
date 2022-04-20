
ifndef CC
	CC = gcc
 endif
CFLAGS = -Wall -Wextra -O2 `pkg-config --cflags glib-2.0`
LDFLAGS = -lm  -lbluetooth -lcurl -lpthread

TARGET = logger
OBJ = logger.o blescan.o at.o bledev_mgr.o curl_module.o

all: $(TARGET)

$(TARGET):  $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

clean:
	$(RM) *~ *.o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<