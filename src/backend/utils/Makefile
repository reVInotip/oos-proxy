utils.o: 
	$(CC) $(CCFLAGS) $(ROOT)/utils/hash_map.c $(ROOT)/utils/stack.c

utils.a: utils.o
	ar -rc $(SOURCE)/libutils.a stack.o hash_map.o && ranlib $(SOURCE)/libutils.a 