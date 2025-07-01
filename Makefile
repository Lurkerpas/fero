.PHONY:all test clean examples

all: test examples

test:
	$(MAKE) -C test test

examples:
	$(MAKE) -C examples all

clean:
	$(MAKE) -C test clean
	$(MAKE) -C examples clean