# Pixar OpenUSD tutorials in C++

This directory contains C++ ports of the official Pixar OpenUSD tutorials.

## Hello World

Add the C++ port of the [Hello World tutorial](https://openusd.org/release/tut_helloworld.html)
to `hello_world.cpp`, then configure and build it from the repository root:

```bash
source ~/envs/general/bin/activate
cmake -S pixar_tutorials/C++ -B pixar_tutorials/C++/build \
  -DCMAKE_PREFIX_PATH=/media/nadim/Data/OpenUSD
cmake --build pixar_tutorials/C++/build
```

The resulting executable is `pixar_tutorials/C++/build/hello_world`.
