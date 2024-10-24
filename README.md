# finfo -- F*ile* Info
finfo is a small program that displays information about a file. Yeah that's all!

## Usage
```
./finfo [args] <files>
```

## Example
```
$ ./finfo .
<b>finfo/</b>:
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

## Compiling
### Compiling finfo only
```
make finfo
```
### finfo + mime_extension
```
make all
```
