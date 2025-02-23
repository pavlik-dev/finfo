[![C/C++ CI](https://github.com/pavlik-dev/finfo/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/pavlik-dev/finfo/actions/workflows/c-cpp.yml)

# finfo -- F*ile* Info
finfo is a small program that displays information about a file. Yeah that's all!

## Usage
```
Usage:
  ./finfo [args] <files>
Args:
  -fd Prints out field's ID instead of it's name.
  -ev Prints out every loaded extension.
```

## Example
```
$ ./finfo .
finfo/:
  Contents:
    1 folders (+1 hidden)
    17 files (+1 hidden)
  MIME: inode/directory
  Permissions: drwxr-xr-x (755)
  Type: directory/folder
```

## Extensions
Want to see the image resolution? Make your own extension!
Take mime_extension.cpp as an example.
But please stick to these rules:
* The extension ID must be unique to prevent conflicts.<br>
  `pavliktt.is_kitty`
* An error occurred? `throw Exception()!`<br>
  ```c++
  throw new Exception("Cuteness overflow!");
  ```
* Compatibility check should be super fast, like checking magic numbers or extensions.<br>
  Don't use anything that can slow down the program.
* **PLEASE**, check if your extension is working properly before making it publicly available.

## Compiling
### Compiling finfo only
```shell
make finfo
```
### finfo + mime_extension
```shell
make all
```

## Supported OSs
**Linux** -- Full support\
**Mac OS X/OS X/macOS, iPhone OS/iOS/iPadOS** -- Partial support, no extensions\
**Windows** -- Untested.

I'm sure it'll work on any architecture.
