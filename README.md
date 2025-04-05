# Checkers in C using Raylib

> [!NOTE]
> This reimplementation is currently unfinished. There is no way to play
> against the computer yet, but playing against a friend should work 
> just fine.

This was supposed to be just a small little project with a not so accurate implementation of checkers just so I could learn how to use certain Raylib functions, but I guess I got a bit ambitious...

I took some liberties while implementing certain rules (only those related to wins and draws) because I don't think I could've implemented them properly without creating a mess... but I might implement them in the future!

## Building and Running

### On Windows

If you have installed something like MinGW through MSYS2 UCRT64 or [w64devkit](https://github.com/skeeto/w64devkit) (assuming the bin folder is in your PATH), you should be able to run the following batch script to get the dependencies. Then you'll be able to compile the code with either `make` or `mingw32-make`.

```console
> .\getdeps.bat
```

If you can't or don't want to run the batch script to get the dependencies, you can try running the PowerShell script.

```console
> .\getdeps.ps1                                              # This might work just fine
or
> powershell -ExecutionPolicy Bypass -File .\getdeps.ps1     # Use this in case PowerShell says "execution of scripts is disabled on this system" to the previous command
```

### On Linux

You might want to check out [this link](https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux#install-on-gnu-linux) to see if there is a
Raylib package available for your distro. In case there isn't, you should be able to run the following shell script to get the dependencies. Then you'll be able to compile the code with either `make` or `mingw32-make`.

```console
$ chmod +x ./getdeps.sh             # give it permission to execute
$ ./getdeps.sh                      # run the script
```

Otherwise, you can always [compile from source](https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux). Then you just run `make`.