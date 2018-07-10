#CC = arm-hisiv200-linux-gcc
CC = gcc
TARGE = MrUvc
OBJS = main.o uvc.o minrry.o
FLAG = -Wall -lpthread -I ./ -DLINUX

$(TARGE) : $(OBJS)
	$(CC) -o $@ $^ $(LIB) $(FLAG)  

.c.o:
	$(CC) -c $< $(LIB) $(FLAG)

clean:
	rm $(TARGE) $(OBJS)  
