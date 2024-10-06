operations.o:
	$(CC) $(CCFLAGS) $(ROOT)/shlib_operations/loader.c $(ROOT)/shlib_operations/destroyer.c $(ROOT)/shlib_operations/initializer.c $(ROOT)/shlib_operations/exec_requests.c

operations.a: operations.o
	ar -rc $(SOURCE)/liboperations.a exec_requests.o initializer.o loader.o destroyer.o && ranlib $(SOURCE)/liboperations.a