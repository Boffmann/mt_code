
all: hello_world_types hello_world_publisher hello_world_subscriber

clean depend generated realclean check-syntax $(CUSTOM_TARGETS):
	@$(MAKE) -f Makefile.hw_types $(@)
	@$(MAKE) -f Makefile.hw_publisher $(@)
	@$(MAKE) -f Makefile.hw_subscriber $(@)

.PHONY: hello_world_types
hello_world_types:
	@$(MAKE) -f Makefile.hw_types all

.PHONY: hello_world_publisher
hello_world_publisher: hello_world_types
	@$(MAKE) -f Makefile.hw_publisher all

.PHONY: hello_world_subscriber
hello_world_subscriber: hello_world_types
	@$(MAKE) -f Makefile.hw_subscriber all

project_name_list:
	@echo hello_world_publisher
	@echo hello_world_subscriber
