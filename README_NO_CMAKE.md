# GoGame — No CMake (Makefile build)
- Build: `make`
- Run: `make run`
- Clean: `make clean`

## Install deps
### MSYS2 (Windows)
```bash
pacman -Syu
pacman -S --needed mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-pkgconf \
  mingw-w64-ucrt-x86_64-sfml mingw-w64-ucrt-x86_64-nlohmann-json
```
(Use `mingw-w64-x86_64-*` packages if you're on MinGW64 shell.)

### Ubuntu/Debian
```bash
sudo apt install g++ make pkg-config libsfml-dev nlohmann-json3-dev
```

### macOS
```bash
brew install sfml nlohmann-json pkg-config
```

## Notes
- Run from the project root so `assets/` is found.
- Scoring is area-estimate; ko is simplified.
