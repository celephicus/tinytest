EXE := basic
 
CFLAGS = -Wall -Werror -Wundef -I../../src -I.

DEPS = ../../src/tinytest.h tinytest_local.h

OBJS = tinytest.o basic.o

$(EXE): $(OBJS)
	$(CC) -o $(EXE) $^ $(LDFLAGS)
	
tinytest.o: ../../src/tinytest.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

basic.o: basic.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
 
.PHONY: clean test

clean:
	rm -f $(OBJS) $(EXE)
	
test: $(EXE)
	@echo; echo "\n#### Verbose"
	-./$(EXE) -v
	@echo; echo "#### Default"
	-./$(EXE)
	@echo; echo "#### Concise"
	-./$(EXE) -c
	@echo; echo "#### Quiet"
	-./$(EXE) -q
