[![C/C++ CI](https://github.com/pavlik-dev/finfo/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/pavlik-dev/finfo/actions/workflows/c-cpp.yml)

# finfo -- F*ile* Info
finfo is a small program that displays information about a file. Yeah that's all!

## Usage
```
Usage:
  finfo [-d] <files>

  -d  --  Detailed view, fields from `struct stat` (see 'man stat.3')
```

## Example
### Simple view (default)
```
$ finfo .     
finfo-linux/:
  Type: directory

  Files:       14 (+2 hidden)
  Directories: 1 (+3 hidden)

  Permissions: drwxrwxr-x (775)

  Created:  2025-02-20 19:25:09
  Opened:   2025-02-28 01:11:25
  Modified: 2025-02-28 01:11:25
```
### Detailed view
```
$ finfo -d .  
finfo-linux/:
  Type: directory

  Files:       14 (+2 hidden)
  Directories: 1 (+3 hidden)

  Parent device ID: 2052
  Inode: 47200487
  Hard links: 6
  Owner: 1000, group 1000
  Special device ID: 0
  Size in bytes: 4096
  FS block size: 4096
  Total blocks: 8

  Permissions: drwxrwxr-x (775)

  Created:  2025-02-20 19:25:09.544126326
  Opened:   2025-02-28 01:11:25.371282257
  Modified: 2025-02-28 01:11:25.167286410
```
*Note: some fields aren't available on Windows.*

## Compiling
### Compiling
```shell
make
```

### Installing (root access required)
```shell
make install
```

## Supported OSs
**Linux** &mdash; Full support\
**Mac OS X/OS X/macOS** &mdash; Untested, full support expected\
**iPhone OS/iOS/iPadOS** &mdash; Compile with `NO_DIRENT`, `NO_MNTENT` and `NO_MIME`.\
**Windows** &mdash; Some fields are not supported in detailed view.
