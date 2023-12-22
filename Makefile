all: build

build: init
	@ninja -C build


init:
	@cmake -B build -GNinja -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE


run: build
	@./build/wzq


clean:
	@rm -rf build
	@echo "Cleaned"
