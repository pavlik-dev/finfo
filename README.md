[![C/C++ CI](https://github.com/pavlik-dev/finfo/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/pavlik-dev/finfo/actions/workflows/c-cpp.yml)

# finfo -- F*ile* Info
finfo is a small program that displays information about a file. Yeah that's all!

## Usage
```
./finfo <files>
```

## Example
```
$ finfo .
finfo/:
  Type: directory/folder
  Contents:
    11 files (+0 hidden):
    3 folders (+3 hidden):
  Permissions: drwxrwxr-x (775)
  Last:
    opened: Sun Feb 23 05:20:33 2025
    modified: Sun Feb 23 05:20:03 2025
```

## Compiling
### Compiling
```shell
make finfo
```

### Installing (root access required)
```shell
make install
```

## Supported OSs
**Linux** &mdash; Full support\
**Mac OS X/OS X/macOS, iPhone OS/iOS/iPadOS** &mdash; Untested, full support expected\
**Windows** &mdash; **Experimental**, build with g++.
