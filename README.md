# BandMIP

BandMIP is a C++20 local-search solver for mixed-integer programming (MIP). This document only covers how to build and run it.

## Build

Requirements: CMake ≥ 3.15 and a C++20-capable GCC or Clang.

```bash
mkdir build
cd build
cmake ..
make -j8
```

The executable is written to `build/BandMIP` (`build/BandMIP.exe` on Windows).

On Windows, build with MinGW-w64 GCC, for example:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_CXX_COMPILER=C:/path/to/mingw64/bin/g++.exe
cmake --build build -j
```

When running `BandMIP.exe`, make sure the MinGW `bin` directory is on `PATH` so that `libstdc++-6.dll`, `libgcc_s_seh-1.dll`, and `libwinpthread-1.dll` can be found.

## Run

```bash
./build/BandMIP -i test-set/sct1.mps -t 60
```

PowerShell on Windows:

```powershell
.\build\BandMIP.exe -i test-set\sct1.mps -t 60
```

`-i/--model_file` is required and accepts an `.mps` or `.lp` model. `-t/--time_limit` sets the time limit in seconds (default `10`). Paths are resolved relative to the current working directory.

Run `./build/BandMIP --help` to list every command-line option.

## Common options

| Option | Meaning |
| --- | --- |
| `-i, --model_file` | Input model (`.mps` / `.lp`), required |
| `-s, --sol_path` | Write the best solution to this `.sol` file |
| `-k, --start_sol_path` | Load a warm-start solution from this `.sol` file |
| `-c, --param_set_file` | Load parameters from a `.set` file (e.g. `default.set`) |
| `-t, --time_limit` | Time limit in seconds (default `10`) |
| `-S, --random_seed` | Random seed (default `0`) |
| `-m, --start` | Initial-solution method: `zero`, `random`, `objective`, `locks` |
| `-y, --restart` | Restart strategy: `random`, `best`, `hybrid` |
| `-l, --log_obj` | `1` prints the incumbent-objective trace, `0` disables it |

## Examples

```bash
# Write the best solution found
./build/BandMIP -i test-set/sct1.mps -t 60 -s sct1.sol

# Load parameters from a file (command-line options still take precedence)
./build/BandMIP -c default.set -i test-set/sct1.mps -t 30

# Warm start from an existing solution file
./build/BandMIP -i test-set/sct1.mps -k start.sol -t 60

# Reproducible run with a fixed seed, without the objective trace
./build/BandMIP -i test-set/sct1.mps -t 60 -S 1 -l 0
```

The `.sol` file contains a header line followed by one `variable value` pair per non-zero variable.
