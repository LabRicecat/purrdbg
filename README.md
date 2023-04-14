# purrdbg
The nyasm debugger \*purr\*

## Installation
This project requires the [catcaretaker](https://github.com/labricecat/catcaretaker) and ncurses.
```
$ catcare sync
$ mkdir build && cd build
$ cmake .. 
$ sudo make install
```

## Usage
```
$ purrdbg [file.nyasm]
```

## Keybindings
```
[q]uit        -> exit the program
[w]ipe        -> clear the shell
[b]reakpoint  -> add a breakpoint
[v]iew        -> view a heap address
[s]stack      -> view the stack
[r]un         -> start the program
[c]ontinue    -> continue execution
[n]ext        -> step one line
[k]/ArrowUp   -> Scroll Up
[j]/ArrowDown -> Scroll Down
```