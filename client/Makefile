
.PHONY: all clean pws swc

all : pws swc

clean:
	make -f pws/.rules clean
	make -f swc/.rules clean

pws: 
	make -f pws/.rules all

swc:
	make -f swc/.rules all
