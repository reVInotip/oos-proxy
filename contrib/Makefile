SUBDIRS = cache_test log_healthcheck plugin

make_all_plugins:
	for plugin in $(SUBDIRS); do \
		make -f contrib/$$plugin/Makefile; \
		mkdir $(SOURCE)/contrib/$$plugin; \
		touch $(SOURCE)/contrib/$$plugin/$$plugin.so && cp $(MAIN_DIR)/contrib/$$plugin/$$plugin.so $(SOURCE)/contrib/$$plugin/$$plugin.so; \
	done