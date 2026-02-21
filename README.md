# rmds

A tiny C utility to recursively remove `.DS_Store` files from your system.

## Overview

`rmds` (Remove DS_Store) is a command-line tool designed to scan a directory tree and delete all `.DS_Store` files it encounters. This is particularly useful for cleaning up macOS metadata files before sharing directories or committing to version control systems that aren't configured to ignore them.

## Features

- **Recursive Scanning**: Traverses subdirectories automatically.
- **Safe Deletion**: Only targets files exactly named `.DS_Store`.
- **Flexible Modes**:
  - **Dry Run**: Preview deletions without making changes.
  - **Interactive**: Confirm each deletion manually.
  - **Quiet**: Suppress non-essential output.
  - **Verbose**: See which directories are being scanned.
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

```bash
./rmds [options] [start_path]
```

### Options

| Flag | Long Flag | Description |
| :--- | :--- | :--- |
| `-n` | `--dry-run` | Show what would be deleted without actually deleting. |
| `-q` | `--quiet` | Suppress all output except errors. |
| `-v` | `--verbose` | Display directories as they are scanned. |
| `-i` | `--interactive` | Prompt for confirmation before deleting each file. |
| `-d` | `--max-depth <N>` | Only scan directories at most N levels deep. |
| `-x` | `--one-file-system` | Do not traverse directories on different filesystems. |
| `-e` | `--exclude <DIR>` | Exclude directory name from scan (can be used multiple times). |
| `-m` | `--name <NAME>` | Target filename to delete (defaults to .DS_Store). |
| `-h` | `--help` | Display the help menu. |

### Examples

**Clean a specific directory (dry run):**
```bash
./rmds -n /path/to/directory
```

**Clean multiple directories quietly:**
```bash
./rmds -q dir1 dir2 dir3
```

**Ignore specific directories (e.g., .git and node_modules):**
```bash
./rmds -e .git -e node_modules /path/to/project
```

**Target a different file name:**
```bash
./rmds -m "Thumbs.db" /path/to/directory
```

**Interactive clean with verbose output:**
```bash
./rmds -iv /path/to/project
```

> [!CAUTION]
> Deletion is permanent. Ensure you have the necessary permissions and have backed up important data if you are unsure.

## License

This project is licensed under the **3-Clause BSD License**. See the [LICENSE](LICENSE) file for the full license text.

## Copyright

Copyright (c) 2026, Vlad Shurupov. All rights reserved.
