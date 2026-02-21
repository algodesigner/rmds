# rmds

A C utility to recursively remove `.DS_Store` and AppleDouble files from your system.

## Overview

`rmds` is a powerful and flexible command-line utility designed to clean up metadata clutter. While primarily focused on recursively removing `.DS_Store` files, it also supports targeting **AppleDouble** (`._*`) files and any other recurring junk files through custom naming and pattern modes. This is particularly useful for cleaning up macOS metadata files before sharing directories or committing to version control systems that aren't configured to ignore them.

## What are AppleDouble Files?

AppleDouble is a file format used by macOS to store file metadata (like extended attributes and resource forks) on file systems that don't natively support them (such as FAT32, ExFAT, or network drives).

When a file is copied to such a system, macOS creates a companion file prepended with `._`. For example, a file named `image.png` will have a corresponding metadata file named `._image.png`. While these files are vital for preserving macOS-specific features, they often appear as "junk" files on other operating systems or in version control.

## Features

- **Recursive Scanning**: Traverses subdirectories automatically.
- **Safe Deletion**: Only targets files exactly named `.DS_Store`.
- **Flexible Modes**:
  - **Clean All**: Target both `.DS_Store` and `._*` AppleDouble files.
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
| `-A` | `--clean-all` | Remove both .DS_Store and ._* (AppleDouble) files. |
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

**Clean all metadata files (including AppleDouble `._*` files):**
```bash
./rmds -A /path/to/external/drive
```

**Preview cleaning all metadata (dry run):**
```bash
./rmds -nA /path/to/external/drive
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

## Compatibility

`rmds` is written in standard C and uses POSIX-compliant headers. It is fully compatible with:
- **macOS**: Native environment; handles TCC/permission errors gracefully.
- **Linux**: Works perfectly on any distribution (Ubuntu, Debian, Fedora, etc.).
- **Windows (WSL)**: Compatible through the Windows Subsystem for Linux.

## License

This project is licensed under the **3-Clause BSD License**. See the [LICENSE](LICENSE) file for the full license text.

## Copyright

Copyright (c) 2026, Vlad Shurupov. All rights reserved.
