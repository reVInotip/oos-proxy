$(MAIN_OBJ): $(OBJ) $(LIBS)
	$(CC) $(MY_CCFLAGS) $(ROOT)/master.c
  
master_utils.o:
	$(CC) $(MY_CCFLAGS) $(ROOT)/master_utils.c

static.a:
	$(CC) $(MY_CCFLAGS) $(ROOT)/static_lib.c && ar -rc $(SOURCE)/libstatic.a static_lib.o && ranlib $(SOURCE)/libstatic.a

dynamic.so:
	$(CC) $(MY_CCFLAGS) $(ROOT)/dynamic_lib.c && $(CC) -shared $(MY_LDFLAGS) dynamic_lib.o -o $(SOURCE)/libdynamic.so

include $(ROOT)/shlib_operations/Makefile
include $(ROOT)/utils/Makefile
include $(ROOT)/guc/Makefile
include $(ROOT)/logger/Makefile
include $(ROOT)/memory/Makefile
include $(ROOT)/boss_operations/Makefile
include $(ROOT)/bg_worker/Makefile
