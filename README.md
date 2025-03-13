# ui.h

A single-header UI library written in C.

## Running

Currently the only supported backend is SDL3. The run scripts (`run.sh` and
`run.ps1`) will automatically clone the SDL repository and compile it. Only
release versions are allowed and the version number can be specified in
`SDLpath.sh` or `SDLpath.ps1`.

Building SDL requires `git`, `cmake` and `ninja`.

SDL is built only the first time you run this project on a specific system. If
you have dual boot or use WSL, SDL will be built when running on each system for
the first time.

### Linux

> Using this method references the SDL version in `SDLpath.sh`.

To compile and run `main.c` execute `run.sh`. This will produce an executable
named `main` inside `build/`.

### Windows (clang.exe)

> Using this method references the SDL version in `SDLpath.ps1`.

You need to have `clang.exe` [^1] in your PATH.

To compile and run `main.c` execute `run.sh`. This will produce an executable
named `main.exe` inside `build/`.

[^1]: Can be downladed at `https://github.com/llvm/llvm-project/releases` under `clang+llvm-<version>-<architecture>-pc-windows-msvc.tar.xz`.

### Windows (Visual Studio)

> Using this method references the SDL version in `SDLpath.ps1`.

Open `C_ui.sln` inside `VisualStudio/` and run the project.
