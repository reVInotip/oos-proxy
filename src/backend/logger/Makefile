logger.o:
	$(CC) $(CCFLAGS) $(ROOT)/logger/logger.c

logger.so: logger.o
	$(CC) -shared $(LDFLAGS) logger.o -o $(SOURCE)/liblogger.so