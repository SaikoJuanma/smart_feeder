# Smart Feeder

[![CI](https://github.com/SaikoJuanma/smart_feeder/actions/workflows/ci.yml/badge.svg)](https://github.com/SaikoJuanma/smart_feeder/actions/workflows/ci.yml)
[![Docs](https://github.com/SaikoJuanma/smart_feeder/actions/workflows/pages.yml/badge.svg)](https://github.com/SaikoJuanma/smart_feeder/actions/workflows/pages.yml)
[![clang-format](https://github.com/SaikoJuanma/smart_feeder/actions/workflows/clang-format.yml/badge.svg?branch=main)](https://github.com/SaikoJuanma/smart_feeder/actions/workflows/clang-format.yml)
![SCA](https://github.com/SaikoJuanma/smart_feeder/actions/workflows/sca.yml/badge.svg)

An **Zephyr RTOS** application that runs on the **`ESP32C6 DevKitC - 1`** board and controls a fish feeder.

## Workspace setup

This repository includes a `west.yml` manifest pinned to Zephyr v4.3.0. A clean setup looks like:

```bash
mkdir -p ~/git/smart_feeder

git clone https://github.com/SaikoJuanma/smart_feeder.git

cd ~/git/smart_feeder/

python -m venv .venv
source .venv/bin/activate
pip install west

west init -l app
west update
west zephyr-export
west packages pip --install
```

## Quickstart (build & run)

### Run the simulation (Terminal A)

From a Zephyr workspace where this repo is the application root:

```bash
cd smart_feeder/app/
west build -b esp32c6_devkitc/esp32c6/hpcore --pristine
west flash
```

### Connect to the shell

Minicom

```
    sudo minicom -b 115200 -D /dev/tty<> 
```

## Project structure

Key folders:

```
smart_feeder/
├── src/        # Application code (this is what we target for coverage)
├── include/    # Application headers      
├── tests/
│   ├── unit/            # Unit tests per module (ztest)
│   └── integration/     # System-level tests that exercise threads/work
├── docs/                # Doxygen markdown pages
├── west.yml             # Zephyr manifest (pins Zephyr version)
├── Doxyfile             # Doxygen configuration
└── prj.conf             # App config for native_sim
```

## Tests

### Run all tests

```bash
west twister -T tests -p native_sim -v
```

### Run only unit tests

```bash
west twister -T tests/unit -p native_sim -v
```

### Run only integration tests

```bash
west twister -T tests/integration -p native_sim -v
```

Twister will also emit JUnit-style reports under `twister-out/`.

---

## Coverage (100% lines for `src/`)

### Generate coverage data + HTML report

```bash
west twister -T tests -p native_sim -v   --outdir twister-out-coverage   --coverage --coverage-tool gcovr --coverage-formats html
```

HTML report:

- `twister-out-coverage/coverage/index.html`

### Terminal summary + CI gate (this repo's code only)

This command filters to **only** the repository's `src/` and excludes `src/main.c` and all tests:

```bash
gcovr twister-out-coverage   --root .   --filter '^src/'   --exclude '^src/main\.c$'   --exclude '^tests/'   --print-summary   --fail-under-line 100
```

### What does “branch coverage” mean?

- **Line coverage**: was each line executed at least once?
- **Branch coverage**: were all outcomes of decisions executed? (e.g., both sides of `if/else`,
  all `switch` cases, short-circuit paths in `&&`/`||`, etc.)

In embedded-style code there are often defensive error branches that are hard to trigger deterministically
in CI without fault-injection. For this demo we enforce **100% line coverage** for `src/`.

---

## Documentation

- 📘 Doxygen (API docs): https://SaikoJuanma.github.io/smart_feeder/
- 🛠️ Generate locally: `doxygen Doxyfile` → open `doxygen-out/html/index.html`
