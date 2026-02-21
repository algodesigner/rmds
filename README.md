# rmds

A tiny C utility to recursively remove `.DS_Store` files from your system.

## Overview

`rmds` (Remove DS_Store) is a command-line tool designed to scan a directory tree and delete all `.DS_Store` files it encounters. This is particularly useful for cleaning up macOS metadata files before sharing directories or committing to version control systems that aren't configured to ignore them.

## Features

- **Recursive Scanning**: Traverses subdirectories automatically.
- **Safe Deletion**: Only targets files exactly named `.DS_Store`.
- **Status Logging**: Prints the path of every file deleted.
- **Fast and Lightweight**: Written in pure C with minimal dependencies.

## Installation

### Prerequisites

- A C compiler (like `gcc` or `clang`)
- `make` (optional, for easier building)

### Building from Source

You can build the project using the provided `Makefile`:

```bash
make
```

Alternatively, compile directly with `gcc`:

```bash
gcc -o rmds rmds.c
```

## Usage

Run `rmds` followed by the directory you want to clean:

```bash
./rmds /path/to/directory
```

If no path is provided, it defaults to your home directory (`$HOME`):

```bash
./rmds
```

> [!CAUTION]
> Deletion is permanent. Ensure you have the necessary permissions and have backed up important data if you are unsure.

## License

This project is licensed under the **3-Clause BSD License**. See the [LICENSE](LICENSE) file for the full license text.

## Copyright

Copyright (c) 2026, Vlad Shurupov. All rights reserved.
