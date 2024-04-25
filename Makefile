ROOT_DIR = $(shell pwd)
objects = main.o iohub.o memory.o cpu.o accel.o memoryhub.o guest_loader.o device_hub.o vm_service.o device_serial.o

.PHONY all: akvm_dm

akvm_dm: $(objects)
	g++ $(objects) -o akvm_dm -O0 -g

$(objects): %.o: %.cpp .phony_install_linux_headers
	g++ -c "$<" -o "$@"

.phony_install_linux_headers:
	make -C /home/yy/src/linux/stable INSTALL_HDR_PATH=$(ROOT_DIR)/linux_header headers_install

.PHONY clean:
	rm -f *.o
	rm -f akvm_dm
