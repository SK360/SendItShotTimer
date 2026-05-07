sketch      :=code.ino
CORE        :=m5stack:esp32
boardconfig :="${CORE}:m5stack_sticks3"

ARDUINO_CLI ?= arduino-cli

# Native USB — update this to match your device path (e.g. /dev/cu.usbmodem*)
DEVICE :=/dev/cu.usbmodem01

ESPTOOL := ~/Library/Arduino15/packages/m5stack/tools/esptool_py/5.1.0/esptool
BUILD_DIR := build/m5stack.esp32.m5stack_sticks3

.PHONY: build
build:
	$(ARDUINO_CLI) compile -e --fqbn $(boardconfig) $(sketch) \
	  --additional-urls "https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/arduino/package_m5stack_index.json"

.PHONY: flash
flash:
	$(ARDUINO_CLI) upload -p ${DEVICE} --fqbn ${boardconfig} ${sketch} \
	  --additional-urls "https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/arduino/package_m5stack_index.json"

.PHONY: merge
merge:
	$(ESPTOOL) --chip esp32s3 merge_bin --target-offset 0x0 -o merged.bin \
	  0x0    $(BUILD_DIR)/code.ino.bootloader.bin \
	  0x8000 $(BUILD_DIR)/code.ino.partitions.bin \
	  0xe000 ~/Library/Arduino15/packages/m5stack/hardware/esp32/3.3.7/tools/partitions/boot_app0.bin \
	  0x10000 $(BUILD_DIR)/code.ino.bin

.PHONY: clean
clean:
	rm -rf build
	rm -f merged.bin

.PHONY: monitor
monitor:
	arduino-cli monitor --fqbn ${boardconfig} -p ${DEVICE} --config 115200

.PHONY: all
all: build flash clean monitor

.PHONY: release
release: clean build merge
