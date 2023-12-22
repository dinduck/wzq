# RelWithDebInfo 优化 MinSizeRel 最小二进制 Release 发行 Debug 调试
CONFIG := Release

all: build

build: init
	@ninja -C build


init:
	@cmake -B build -GNinja -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_BUILD_TYPE:STRING=${CONFIG}


run: build
	@./build/wzq


clean:
	@rm -rf build
	@echo "Cleaned"
