# The ARM toolchain prefix (32 bit = arm-...-eabi, 64 bit = aarch64-...-gnueabi)
# TOOLCHAIN = arm-none-eabi
# TOOLCHAIN = /usr/local/gcc-arm-none-eabi-6-2017-q2-update/bin/arm-none-eabi
# TOOLCHAIN = /root/x-tools/armv8-rpi3-linux-gnueabihf/bin/armv8-rpi3-linux-gnueabihf
# TOOLCHAIN = /root/x-tools/aarch64-rpi3-linux-gnueabi/bin/aarch64-rpi3-linux-gnueabi
TOOLCHAIN = /root/x-tools/gcc-linaro-5.5.0-2017.10-x86_64_aarch64-elf/bin/aarch64-elf

AARCH = 
CCFLAGS = -Wall -nostartfiles -ffreestanding -mcpu=cortex-a53 -ggdb 
++FLAGS = $(CCFLAGS) -std=c++11 -fno-rtti -fno-exceptions -fno-use-cxa-atexit -fno-unwind-tables -ffunction-sections -fdata-sections -Wl,--gc-sections
LIBS = -L /root/x-tools/sysroot-newlib-linaro-2017.10-aarch64-elf/usr/lib/include -L /root/x-tools/sysroot-newlib-linaro-2017.10-aarch64-elf/usr/lib

TARGET = kernel8
BUILD = build
SOURCE = src

COPY = /Volumes/boot

SOBJ = bootcode64.o vectors64.o
UOBJ = cstartup.o cstubs.o peripheral.o gpio.o mailbox.o interrupts.o timer.o uart.o multicore.o perf.o
COBJ = init.o locks.o producer.o context.o kernel.o

all: $(BUILD)/$(TARGET).elf $(BUILD)/$(TARGET).img $(BUILD)/$(TARGET).list

# ELF
$(BUILD)/$(TARGET).elf: $(addprefix $(BUILD)/, $(SOBJ)) $(addprefix $(BUILD)/, $(UOBJ)) $(addprefix $(BUILD)/, $(COBJ))
	$(TOOLCHAIN)-g++ $(++FLAGS) -T $(SOURCE)/linker.ld $(addprefix $(BUILD)/, $(SOBJ)) $(addprefix $(BUILD)/, $(UOBJ)) $(addprefix $(BUILD)/, $(COBJ)) -o $(BUILD)/$(TARGET).elf

# ELF to LIST
$(BUILD)/$(TARGET).list: $(BUILD)/$(TARGET).elf
	$(TOOLCHAIN)-objdump -D $(BUILD)/$(TARGET).elf > $(BUILD)/$(TARGET).list

# ELF to IMG
$(BUILD)/$(TARGET).img: $(BUILD)/$(TARGET).elf
	$(TOOLCHAIN)-objcopy -O binary $(BUILD)/$(TARGET).elf $(BUILD)/$(TARGET).img

$(addprefix $(BUILD)/, $(SOBJ)): $(BUILD)/%.o: $(SOURCE)/asm/%.s
	$(TOOLCHAIN)-as $(SOURCE)/asm/$(basename $(notdir $@)).s -o $@

$(addprefix $(BUILD)/, $(UOBJ)): $(BUILD)/%.o: $(SOURCE)/c/%.c
	$(TOOLCHAIN)-gcc $(CCFLAGS) -c $(SOURCE)/c/$(basename $(notdir $@)).c -o $@

$(addprefix $(BUILD)/, $(COBJ)): $(BUILD)/%.o: $(SOURCE)/cpp/%.cpp
	$(TOOLCHAIN)-g++ $(++FLAGS) -c $(SOURCE)/cpp/$(basename $(notdir $@)).cpp -o $@

copy: all
	cp $(BUILD)/$(TARGET).img $(COPY)/$(TARGET).img

clean:
	rm -f $(BUILD)/*

# Cross compile the binary using a container with the toolchain already built, when running in this environment, $(TOOLCHAIN) must point to the path within the cointainer
DOCKER_IMAGE = toolchain
DOCKER_BUILD = /root/build
start-toolchain:
	docker run --rm -it -v $(CURDIR):$(DOCKER_BUILD) -w $(DOCKER_BUILD) $(DOCKER_IMAGE)

SERIAL_DEVICE = /dev/tty.usbserial-AH069DMB
SERIAL_BAUD_RATE = 115200
serial-screen:
	screen $(SERIAL_DEVICE) $(SERIAL_BAUD_RATE)

# GDB_HOST = localhost
GDB_HOST = docker.for.mac.host.internal
GDB_ARGS = -ex "tui enable" -ex "layout split" -ex "set confirm off" 

gdb-core-0:
	$(TOOLCHAIN)-gdb $(BUILD)/$(TARGET).elf  -ex "target remote $(GDB_HOST):3333" $(GDB_ARGS) -ex "load $(BUILD)/$(TARGET).elf"

gdb-core-1:
	$(TOOLCHAIN)-gdb $(BUILD)/$(TARGET).elf -ex "target remote $(GDB_HOST):3334" $(GDB_ARGS) -ex "file $(BUILD)/$(TARGET).elf"
	
gdb-core-2:
	$(TOOLCHAIN)-gdb $(BUILD)/$(TARGET).elf -ex "target remote $(GDB_HOST):3335" $(GDB_ARGS) -ex "file $(BUILD)/$(TARGET).elf"
	
gdb-core-3:
	$(TOOLCHAIN)-gdb $(BUILD)/$(TARGET).elf -ex "target remote $(GDB_HOST):3336" $(GDB_ARGS) -ex "file $(BUILD)/$(TARGET).elf"
