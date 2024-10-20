# Compression Tool (pack.c)

**Compression Tool** is a command-line utility written in C that compresses and decompresses files using a ZSTD. It aims to provide fast and efficient file size reduction..

## Table of Contents

- [Compression Tool (pack.c)](#compression-tool-packc)
  - [Table of Contents](#table-of-contents)
  - [Features](#features)
  - [Requirements](#requirements)
  - [Installation](#installation)
    - [Using CMake (Cross-Platform)](#using-cmake-cross-platform)
    - [Windows](#windows)
  - [Usage](#usage)
    - [Command Syntax](#command-syntax)
    - [Options](#options)
    - [Example Commands](#example-commands)
  - [Acknowledgments](#acknowledgments)
  - [License](#license)

## Features

- **Fast Compression and Decompression**: Designed to offer high-speed compression with minimal loss of data.
- **Command-Line Interface**: Easy-to-use CLI for quick file compression and decompression.
- **Cross-Platform**: Works on major operating systems including Linux and Windows.

## Requirements

- **Windows**: Requires MinGW or similar to compile.
- **Sccache**: Sccache is a ccache-like tool.
- **Compiler**: GCC or any C99 compatible compiler.
- **CMake** (optional): For build automation, recommended for easier setup.
- **Nur**: a taskrunner based on nu shell.

## Installation

### Using CMake (Cross-Platform)

1. Clone the repository:
   ```bash
   git clone --recurse-submodules -j8 https://github.com/neo2043/pack.c.git
   cd pack.c
   nur get sqlite
   nur patch
   ```

2. Build the project:
   ```bash
   nur build pack
   ```

3. Run the tool:
   ```bash
   ./build-[architecture]-[os-name]/pack[.exe]
   ```

You can copy the project binary to a folder in path to use it globally in terminal

### Windows

1. Install MinGW or another C compiler.
2. Follow the CMake instructions above to build.

## Usage

The compression tool can be used via the command line. Below are some basic usage instructions.

### Command Syntax

```bash
./pack [OPTIONS] [PATH TO ARCHIVE] [PATH TO FFOLDER]
```

### Options

- `-c, --compress <file/folder>`: Compress the specified file.
- `-l, --compression-level`: compression level.
- `-C, --chunk-size <num>`: Chunk size in MB.
- `-f, --archive <file>`: path to .cpack archive.
- `-j, --thread-num`: number of threads.
- `-v, --verbose`: verbose.
- `-h, --help`: Show help message.

### Example Commands

1. Compress a file:
   ```bash
   ./pack[.exe] -cvf example.cpack  example.txt
   ```

2. Decompress a file:
   ```bash
   ./pack[.exe] -xvf example.cpack .
   ```

## Acknowledgments

This is based on the [original implementation by PackOrganiztion](https://github.com/PackOrganization/Pack)

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.