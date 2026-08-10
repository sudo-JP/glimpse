VCPKG_PATH = $(HOME)/.local/share/vcpkg/scripts/buildsystems/vcpkg.cmake

.PHONY: all setup build run clean

all: build run

setup:
	cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=$(VCPKG_PATH) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

build:
	cmake --build build

run:
	./build/glimpse

clean:
	rm -rf build
