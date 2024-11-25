CC		:= gcc
CFLAGS 	:= -g -O0 -fPIC
LDFLAGS := -shared -lX11
#CFLAGS 	:= -O3
TESTLDFLAGS := -g -O0 -ldl

OBJS	:= x11_gpio.o
TESTOBJS	:= main.o
LIB		:= libx11_gpio.so
EXE		:= a.out


all:$(EXE)

$(EXE): $(LIB) $(TESTOBJS)
	$(CC) $(TESTLDFLAGS) $(TESTOBJS) -o $@

$(LIB): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@ 

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $<

$(TESTOBJS): %.o: %.c
	$(CC) $(TESTCFLAGS) -c $<


clean:
	rm -f $(OBJS) $(TESTOBJS) $(LIB) $(EXE)
