# active-window-tracker
Tracks time spent in the active window

## Compatibility

It uses `xdotool` to get the active window, porting to Windows is possible when using `GetActiveWindow` inside a C file instead of using bash.

## What does it do?

It creates a folder inside the home directory called `.tracks` where it stores `.track` files, these are named in the format `dd.mm.yyyy_n.track` and contain two lines each for one record of an active window, it uses the window title to differentiate windows. One track record might look like:
```
Mozilla Firefox
1690535898.732222026 1690535912.929135767 1690535918.006312150 1690535919.064696755 1690536650.452245204 1690536653.535365392
```
The first line is the window title and the second line contains white space separated numbers, the first number shows the time the window has become active and the second number, when it became inactive.

For Linux, this file is generated by a bash script but for Windows, it should be included in the .c file.

## Code progress

Only focusing on Linux support right now.<br>
`tracker.sh` is done now.<br>
`main.c` has the first stage completed of collecting the track records, now they have to be sorted and put into a time line and neatly printed for viewing pleasure. It should also write `.inter` files which stores intermediary data. This is so we don't have to read through too many files after long periods of running the tracker program.
