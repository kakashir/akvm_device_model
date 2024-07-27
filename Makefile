ROOT_DIR = $(shell pwd)
objects = main.o iohub.o memory.o cpu.o accel.o memoryhub.o guest_loader.o device_hub.o \
			vm_service.o device_serial.o device_hwcfg.o utility.o dev_test.o
h_objects = $(shell find $(ROOT_DIR) -regex ".*\.h$$")
h_objects += $(shell find $(ROOT_DIR) -regex ".*\.hpp$$")
LINUX_SRC_ROOT = /lib/modules/$(shell uname -r)/build
cc_flags = -O0 -g $(cc_flags_user)

.PHONY all: akvm_dm

dev_test:
	make -C $(ROOT_DIR) cc_flags_user="$(cc_flags_user) -D__DEV_TEST__"

akvm_dm: $(objects)
	g++ $(objects) $(cc_flags) -o akvm_dm

$(objects): %.o: %.cpp .phony_install_linux_headers $(h_objects)
	g++ $(cc_flags) -c "$<" -o "$@"

.phony_install_linux_headers:
	make -C $(LINUX_SRC_ROOT) INSTALL_HDR_PATH=$(ROOT_DIR)/linux_header headers_install

.PHONY clean:
	rm -f *.o
	rm -f akvm_dm
