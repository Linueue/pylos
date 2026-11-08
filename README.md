# Pylos

## About

A networking programming language that compiles to actual
configurations of each device, such as routers, and switches.

It is primarily built to be used as a command line interface (CLI),
however, an optional WebAssembly (WASM) build with Emscripten is
provided for visualizing errors, or the actual lexical analyzer.

## Build

Please download (CMake)[https://cmake.org] from their official website,
or with winget,

```
winget install Kitware.CMake
```

### Windows (VSCode)

If you have VSCode, simply open the cloned repo in the VSCode application,
and it will prompt you to install the recommended extensions. Install that,
and it will also prompt you what type of build you want to do. You may choose
between Debug/Release. To compile, press `ctrl + shift + b`.

You can then open it using the terminal by,

```
.\out\debug\Debug\Pylos.exe
```

### Windows (MSVC)

If you have Visual Studio (*not to be confused with VSCode*), you can directly
do the following,

```
cmake -B out
```

It will generate solution files, which you can then double click to open.

### Windows (Clang + Ninja)

This is personally what I use. You can do the following,

```
cmake -B out -G "Ninja"
cd out
ninja
```

### Linux

Make sure you have a compiler installed such as gcc/g++, or LLVM (clang/clang++).
You can directly execute these commands:

```
cmake -B out
cd out
make -j8
```

The `make -j8` command, the flag `-j` specifies how many threads will be used
to compile it.
