.PHONY: build flash clean

VID := 1209
PID := FABF

build:
	@mkdir -p build
	@cd build && cmake .. && make -j8
	@echo "Build complete: build/I2Console.uf2"

flash: build
	@echo "Looking for I2Console device (VID:PID = $(VID):$(PID))..."
	@CDC_PORT=$$(system_profiler SPUSBDataType 2>/dev/null | \
		grep -B 10 "Vendor ID: 0x$(VID)" | \
		grep -A 10 "Product ID: 0x$(PID)" | \
		grep "Serial Number:" | head -1 | awk '{print $$3}' | \
		xargs -I {} sh -c 'ls /dev/tty.usbmodem* 2>/dev/null | head -2 | tail -1'); \
	if [ -n "$$CDC_PORT" ]; then \
		echo "Found I2Console at $$CDC_PORT"; \
		echo "Sending bootloader command..."; \
		echo "BOOTLOADER" > $$CDC_PORT; \
		sleep 2; \
	else \
		echo "I2Console not found. Please enter bootloader mode manually:"; \
		echo "  1. Hold BOOTSEL button"; \
		echo "  2. Connect USB"; \
		echo "  3. Release BOOTSEL"; \
		echo ""; \
		echo "Press Enter when ready..."; \
		read dummy; \
	fi; \
	echo "Flashing firmware..."; \
	picotool load build/I2Console.uf2 && \
	picotool reboot && \
	echo "Flash complete!"

clean:
	@rm -rf build
	@echo "Build directory cleaned"
