EXE=tt_main.exe
 
CC = g++.exe
CFLAGS = -Wall -Werror -Wundef -I..\..\src -I.

TT_SRCS = ..\..\src\tinytest.c
# SRCS = ..\..\src\tinytest.c tt_main.c #  test-all.o test-dump.o test-fixture.o
DEPS = ..\..\src\tinytest.h tinytest_local.h

# All .o files are in this dir.
#_OBJS = $(notdir $(SRCS))
# OBJS = $(_OBJS:.c=.o)
OBJS = tinytest.o tt_main.o

$(EXE): $(OBJS)
	$(CC) -o $(EXE) $^ $(LDFLAGS)
	
tinytest.o: ..\..\src\tinytest.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

tt_main.o: tt_main.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
 
.PHONY: clean test

clean:
	del $(OBJS) $(EXE)
	
test: $(EXE)
	@echo ***************** Verbose
	-$(EXE) -v
	@echo ***************** Default
	-$(EXE)
	@echo ***************** Concise
	-$(EXE) -c
	@echo ***************** Quiet
	-$(EXE) -q
