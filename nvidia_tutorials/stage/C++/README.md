# C++ OpenUSD tutorial

Configure and build from the repository root:

```bash
cmake -S C++ -B C++/build \
  -DCMAKE_PREFIX_PATH=/media/nadim/Data/OpenUSD
cmake --build C++/build
```

Run the example from the repository root:

```bash
./C++/build/stage_tutorial
```

By default it creates `_assets/first_stage_cpp.usda`. You can provide a
different output path as the first argument:

```bash
./C++/build/stage_tutorial _assets/my_stage.usda
```
