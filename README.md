## AltTabMod Project Documentation
![grab-landing-page](overview.gif)
This project does amazing things. Here are some key features:
- Feature 1 Moves the Task Switcher (Alt+Tab) window to the monitor the mouse is on.
- Feature 2 Moves the selected program to the monitor the mouse is on.
- Feature 3 That is pretty much it. :)

For more information, downaload the [Docs](Docs.7z).

## Overview of what AltTabMod does!.
The TaskSwitcher (Alt Tab) window on MS Windows (10) will always spawn on the primary monitor.<br />
This can be very annoying if you have multiple monitors, esp if they are apart.<br /> 
So AltTabMod will move the TaskSwitcher (Alt Tab) window to the desktop the mouse is at.<br />
It will also bring the selected application to the desktop you are working on.<br />

#### !!!NOTE!!!
This programs needs to be run with admin permission.
This **program hooks into explorer.exe** and listen for a two events, when the Alt+tab window is created and destroyed.<br />
So keep that in mind. maybe you have some AV, or other policy's set that triggers something that prevents this code from running.<br />


## Known Limitations and Issues ####
- Can not detect a monitor that is off.
- Will not move fullscreen windows on purpose.<br />
  often fullscreen windows such as games, media centers etc are setup to be run on desired monitor.<br />
  in most cases moving them is not something you want to do.<br />
  This needs to be improved a bit because at the moment chrome in fullscreen (youtube). it doesnt move it.<br />
  So need a way to distinguish from programs that are fullscreen "windowed" or fullscreen exclusively. like a 3d games.
- When the mouse is not on the primary monitor, the TaskSwitcher can flicker for a short while.<br />
  This is because the TaskSwitcher is always spawned at the primary monitor before getting moved.<br />
- Monitors with different resolutions may have issues as its not fully tested.
- Will not work with the old ask style TaskSwitcher.

## How It Moves The Programs In (Alt+Tab).
Rules used for moving programs "so far.."
1. If the program is in full-screen mode. don't move it.
2. If the program is on the same screen as the monitor. don't move it.
3. If the program is maximized. maximize it on the target desktop as well.
4. If the program is bigger than the target monitor, set it to left,top pos and maximize it.
5. If the program is wider than the target monitor but the app area is smaller. set it to left,top pos of the monitor.
6. If the program is taller/higher than the target monitor but the app area is smaller. set it to left,top pos of the monitor.

## Tools used in this project.
gcc is used on windows, to be precise mingw.<br />
most of my gnu tools (gcc, make, etc) are installed by using [msys2](https://www.msys2.org/)<br />
i am not fully up todate on the latest stuff.<br /><br />

gcc --version\n
gcc (Rev8, Built by MSYS2 project) 11.2.0<br />
Copyright (C) 2021 Free Software Foundation, Inc.<br />
This is free software; see the source for copying conditions.  There is NO<br />
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.<br /><br />

windres --version<br />
GNU windres (GNU Binutils) 2.3<br />
Copyright (C) 2021 Free Software Foundation, Inc.<br />
This program is free software; you may redistribute it under the terms of<br />
the GNU General Public License version 3 or (at your option) any later version.<br />
This program has absolutely no warranty.<br /><br />
So if yours is newer than this. it should probably work fine..


## Compile the code!.
I have only built this on 64Bit Os.<br />
Build the code<br />
1. windres.exe -I ..\\AppDir -J rc -O coff -i resource.rc -o resource.res.<br />
2. gcc -o MyApp.exe main.c resource.res -lole32 -lpsapi (-ldwmapi)

To compile/run the program without the Console window, you probably need some extra flags.<br />
gcc -Wall -o MyApp.exe res.... -lole32 **-Wl, –subsystem,windows**<br />
Simple makefile example, check out the [makefile](makefile)<br /><br />

For a 32bit build you might need to do some changes.<br />
Forexample you might need to add the flag **-F pe-i386** to windres.<br />
Also for Gcc you might need to use **-m32 -lmsimg32 -mwindows**.<br />
Although this is not tested so i cant verify.

## Run the program.
Just double click on it and it will run.<br />
If you want the program to automatically start when you login to the pc, you use google. :)<br />
There are tonz of guides on howto automatically start a program.<br />


## Background why i made this.
Im just a widapi noob hacking stuff together for my own needs.<br />
So in my setup, i have two monitors at my desk/bench.<br />
But i also have a tv attached to it a couple of meters away, by hdmi cable.<br />
And my primary desktop is of course on the 1st monitor, the others are just extended desktops.<br />
If i am in the sofa watching tv and i know ie, chrome is open on one of the monitors.<br />
Normally pressing ALt+Tab brings up the Alt+Tab Window on the monitor(Primary) and since it on the 1st monitor, i cant see it when sitting in the sofa.<br />
This has annoyed me for quite a bit, so i have put together something that works for me.<br />
I hope this also can be useful for other people with same annoyance.<br />
And perhaps help improve it as there is probably a better way to approach this.<br />


## Credits.
Some of the guys especially Grégori Macário Harbs also know was Mysoft from Irc. freenode/Libera #winapi<br />
I don't know who else, perhaps MS itself for the OS we use so much, and the guys who made the Tools i have used.<br />

## License 
Gnu Gpl V2. No discussion.


Best Regards.
