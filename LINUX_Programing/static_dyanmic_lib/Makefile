INCLUDES = -I.
LDFLAG = -pipe -Wall -O2 -fPIC

CC = gcc $(LDFLAG) $(INCLUDES)

AR=ar
STRIP=strip

TARGET_NAME= main

ONVIF_OBJS = bill.o second.o 

all : $(ONVIF_OBJS)
	$(CC) main.c -o $(TARGET_NAME) $(ONVIF_OBJS)
	$(STRIP) $(TARGET_NAME)
clean:
	rm -f *.o $(TARGET_NAME)