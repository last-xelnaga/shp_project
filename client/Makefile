
.PHONY: all clean

all : pws swc

clean: pws_clean swc_clean

.PHONY:  pws pws_deploy pws_clean

pws:
	make -f pws/.rules all

pws_deploy: pws
	scp shp_client alarm@192.168.1.45:/home/alarm/shp_client

pws_clean:
	make -f pws/.rules clean


.PHONY: swc swc_deploy swc_clean

swc:
	make -f swc/.rules all

swc_clean:
	make -f swc/.rules clean
