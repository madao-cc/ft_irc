# ft\_irc — Build & Debug Guide (Makefile usage)

This README explains **how to use the provided Makefile** to build, clean, and debug the `ircserv` executable for the 42 *ft\_irc* project. It also covers two opt‑in features:

* `make debug` — build with debug symbols and no optimizations.
* `make asan` — build with AddressSanitizer (ASan) for memory bug detection.

---

## TL;DR

```bash
# normal build
make

# verbose build (show commands)
make V=1

# clean object files / build folder
make clean

# clean everything (binary + build folder)
make fclean

# full rebuild (fclean + build)
make re

# debug build (g3 + O0), then full rebuild
make debug

# address-sanitized build (ASan), then full rebuild
make asan
```

The binary is named **`ircserv`** and object files live under **`build/`**.

---

## Prerequisites

* **Compiler:** `c++` (GCC or Clang) with C++98 support.
* **make:** GNU Make.
* **ASan runtime (for `make asan`):**

  * **Linux (GCC):** install matching libasan package (e.g., `sudo apt install libasan8` depending on your GCC version).
  * **Linux (Clang):** ASan ships with clang (`libclang-rt` packages may be required on some distros).
  * **macOS:** AddressSanitizer is available with Apple Clang. Leak detection support differs from Linux.

> If you switch compilers, you can override on the command line: `make CXX=clang++`.

---

## How the Makefile is structured (what to expect)

* **Name:** `ircserv` (final executable).
* **Sources:** recursively collected from `srcs/**.cpp`.
* **Headers:** add your include paths under `includes/` (already used via `-I includes`).
* **Objects:** compiled under `build/` mirroring source tree structure.
* **Dependencies:** generated automatically via `-MMD -MP` (header changes trigger correct rebuilds).
* **Warnings & standard:** `-Wall -Wextra -Werror -std=c++98`.
* **Quiet logs by default:** pass `V=1` to show the actual commands.

---

## Debug build: `make debug`

**What it does**

* Appends the following flags to `CXXFLAGS` and triggers a **full rebuild** via `re`:

  * `-g3`  — maximum debug info (best for stepping into STL and your code).
  * `-O0`  — disable optimizations (easier debugging and accurate stepping).

**How to use**

```bash
make debug
./ircserv <args>
```

**Recommended workflow with GDB/LLDB**

```bash
gdb ./ircserv
# or
lldb ./ircserv
```

Common commands once inside the debugger:

* `break main` — set breakpoint at program start.
* `run -- <args>` — run with your program arguments.
* `bt` — show backtrace on crash.
* `frame variable` / `print var` — inspect variables.
* `step` / `next` — step into / over.

**Tips & insights**

* If stepping appears to “jump around”, ensure you’re using the debug build (look for `-g3 -O0` in verbose logs with `V=1`).
* Mixing old objects with new flags can make debugging inconsistent; `make debug` already **forces** a clean rebuild via `re`.
* To generate core dumps for post‑mortem debugging on Linux:

  ```bash
  ulimit -c unlimited
  ./ircserv
  gdb ./ircserv core
  ```

---

## AddressSanitizer build: `make asan`

**What it does**

* Appends the following to `CXXFLAGS` and triggers a **full rebuild** via `re`:

  * `-g3` — high‑quality symbol info in reports.
  * `-O1` — light optimization (ASan works fine; keeps reasonable performance).
  * `-fno-omit-frame-pointer` — improves backtraces in ASan reports.
  * `-fsanitize=address` — enable AddressSanitizer (detects OOB, UAF, double free, etc.).

**How to use**

```bash
make asan
./ircserv <args>
```

If ASan detects a bug, the program will typically exit with a non‑zero status and print a detailed report to stderr with the **exact allocation and crash stack traces**.

**Make ASan reports even better**
Set runtime options via `ASAN_OPTIONS` (Linux/Clang/GCC):

```bash
# Strict and helpful defaults for ft_irc
export ASAN_OPTIONS="detect_leaks=1:halt_on_error=1:strict_string_checks=1:check_initialization_order=1:detect_stack_use_after_return=1"
./ircserv <args>
```

* `detect_leaks=1` (Linux): report memory leaks on process exit.
* `halt_on_error=1`: stop at the first error (useful to iterate quickly).
* `strict_string_checks=1`: catch subtle `std::string`/C‑string misuse.
* `detect_stack_use_after_return=1`: enables quaran­tine checks for stack UAF.

> **macOS note:** `detect_leaks` may be ignored or produce different output. If leak detection seems missing, try `leaks` tool or use Linux for the most complete ASan+Lsan experience.

**Exit codes & CI**

* ASan errors cause a **non‑zero exit code**. In scripts, you can assert cleanliness with:

  ```bash
  make asan && ./ircserv <args>; test $$? -eq 0
  ```

**Common ASan issues & fixes**

* **`libasan.so` not found**: install the libasan package matching your compiler version.
* **False positives from third‑party libs**: run without ASan or use suppression files (`ASAN_OPTIONS=suppressions=asan.supp`). For ft\_irc you likely won’t need this.
* **Performance**: expect 1.5–3× slowdown. Use normal debug build when ASan isn’t needed.

---

## Customizing builds

You can override variables from the command line **without editing the Makefile**:

```bash
# use clang++
make CXX=clang++

# add a local define and keep warnings
make CXXFLAGS="-Wall -Wextra -Werror -std=c++98 -DDEBUG_LOGS"

# combine with targets
make debug CXX=clang++
make asan V=1 -j8
```

> Because `debug` and `asan` call `re`, any changes to flags/toolchain will be applied consistently across all objects.

---

## Dependency tracking (why your headers rebuild correctly)

The Makefile uses `-MMD -MP` and includes generated `.d` files:

* `-MMD` writes a per‑object dependency file that lists the headers used when compiling that source.
* `-MP` adds phony targets so deleted headers don’t break incremental builds.
* `-include $(DEPS)` pulls those dep files back into Make, so **editing a header triggers recompiles of the right units**.

You don’t need to run `make clean` after changing a header—just `make`.

---

## Troubleshooting

* **I changed flags but nothing seems to rebuild.** Use `make re` (or run `make debug`/`make asan`, which already do a full rebuild).
* **ASan doesn’t show leaks on macOS.** This is expected; try Linux or Apple’s `leaks` tool.
* **Linker can’t find ASan runtime.** Install `libasan` (Linux) or ensure you’re using Clang on macOS.
* **Commands aren’t printed; I want to see them.** Use `V=1`.
* **Build is slow on big trees.** Use parallelism: `make -j`.

---

## Example sessions

```bash
# 1) Normal flow
make -j
./ircserv 6667 password

# 2) Debugging a crash
make debug V=1
lldb ./ircserv -- 6667 password
# (lldb) run
# (lldb) bt

# 3) Hunting memory errors
make asan -j
export ASAN_OPTIONS="detect_leaks=1:halt_on_error=1"
./ircserv 6667 password

# 4) Start fresh after big refactor
make re -j
```

---

## Notes for reviewers (42)

* The Makefile uses only standard flags and recursive source discovery; no forbidden tricks.
* `debug` and `asan` are **convenience targets** that do not alter the mandatory default build.
* Quiet logs keep output clean for evaluation; `V=1` reveals full commands if needed.

Happy hacking and good luck with **ft\_irc**! 🧰💬
