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
But please stick to these rules:
* The extension ID should contain your nickname, first name, last name, anything that will make it unique:<br>
  `pavliktt.is_kitty`
* An error occurred? `throw Exception(*what happened*)!`<br>
  ```
  throw new Exception("Cuteness overflow!");
  ```
* Compatibility check should be super fast, like checking magic numbers or extensions.<br>
  Don't use anything that can slow down the program.
* **PLEASE**, check if your extension is working properly before making it publicly available.

## Compiling
### Compiling finfo only
```
make finfo
```
### finfo + mime_extension
```
make all
```
