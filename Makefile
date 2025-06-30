.PHONY:all test clean

all: test

test:
	$(MAKE) -C test test

clean:
	$(MAKE) -C test clean