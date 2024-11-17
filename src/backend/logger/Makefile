logger.o:
	$(CC) $(MY_CCFLAGS) $(ROOT)/logger/logger.c

logger.so: logger.o
	$(CC) -shared $(MY_LDFLAGS) logger.o -o $(SOURCE)/liblogger.so