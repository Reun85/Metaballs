.PHONY: default run setup clean test debug_run release_run

default: run

executable = "Graf"

# Fordítja a programot optimalizációkkal
release_build: setup
	cd build && cmake --build . --config Release

# Futtatja a programot optimalizációk nélkül
run: setup
	cd build && cmake --build . 
	cd build && ./${executable}

# Fordítja a programot DEBUG flagekkel (és optimalizációk nélkül)
debug_build: setup
	cd build && cmake --build . --config Debug

# Konfigurálja a projektet
setup:
	cmake -B build -S .

# (Amennyiben véletlenül hibásan módosítottuk a build mappát)
clean:
	-rm -r ./build
	-rm -r ./.cache

# Konfigurálja a programot és ellenőrzi, hogy a kezdő projekt működik-e
test: clean
	$(MAKE) setup
	$(MAKE) run
