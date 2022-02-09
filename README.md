# Heritage SRB2 (Demo 4.1 branch)

## Compiling

### Makefiles

#### Building in Windows 10

1. To compile on windows, make sure you have MSYS2 installed. See [MSYS2 Installation](https://www.msys2.org/#installation) **You must do all of the steps with the `MSYS2 MinGW 32-bit` terminal, or else all the steps won't work.** 
	- The easiest way to get it up and running is to search for it in the program files, click on it (it should be named `MSYS2 MinGW 32-bit`), copy the full directory from your explorer (make sure to change the `/`s into `\`, e.g. `C:\Users\user\Desktop` will have to be changed into `C:/Users/user/Desktop`) and use `cd` in conjunction with it. So for this example, you should now have pasted in your MSYS2 terminal `cd C:/Users/user/Desktop`. Now press enter. *For this example I'll be using the desktop.*
	- You should also install the required dependencies. These should be:
		- (TODO: there's other dependencies which I forgot about.. *You might also run into some errors, try googling them.*)
		- `git`
		- `nasm`
		- `make`
		- So overall, run `pacman -S git nasm make`, and accept everything. You will be set after they're all installed.
2. Once you're all set and done, clone this repo. `git clone https://github.com/gm3k4g/HeritageSRB2.git`. When it's all done, change directory inside. `cd HeritageSRB2`
3. Now, change directory into the source files. `cd src`
4. Similar to the way you would normally compile SRB2 (See [SRB2 Wiki/Source code compiling/Makefiles](https://wiki.srb2.org/wiki/Source_code_compiling/Makefiles)), you must now type  `CFLAGS="-fcommon" make -j4` inside the MSYS2 terminal. The building process will now begin, and you have to wait for a bit.
5. Once it's all done, you should see the last line saying something like `Build is done, please look for srb2win.exe in ../bin/Mingw/Release`. Typically, the build will be put inside `/bin/Mingw/Release` once it's done. So, change directory there. `cd ../bin/Mingw/Release`
6. In order to get the actual game up and running though, you will need the `demo41` assets to get the game running. [Hop over here](https://files.srb2.org/srb2.org/history/srb2dm41.exe) to grab the archive. Extract it inside the same directory as the executable file (**make sure not to replace `srb2win.exe` !!**).
7. The final step is to get the dynamic libraries in the same directory as the exectuable. You will have to go back to the root directory of the project and find the `libs` folder. All the dynamic libraries should be in there as follows:
	- `libs\dll-binaries\i686\libgme.dll`

	- `libs\libopenmpt\bin\x86\libopenmpt.dll`
	- `libs\libopenmpt\bin\x86\openmpt-mpg123.dll`
	- `libs\libopenmpt\bin\x86\openmpt-ogg.dll`
	- `libs\libopenmpt\bin\x86\openmpt-vorbis.dll`
	- `libs\libopenmpt\bin\x86\openmpt-zlib.dll`

	- `libs\SDL2\i686-w64-mingw32\bin\SDL2.dll`

	- `libs\SDLMixerX\i686-w64-mingw32\bin\libfluidsynth-2.dll`
	- `libs\SDLMixerX\i686-w64-mingw32\bin\libgcc_s_sjlj-1.dll`
	- `libs\SDLMixerX\i686-w64-mingw32\bin\libstdc++-6.dll`
	- `libs\SDLMixerX\i686-w64-mingw32\bin\SDL2_mixer_ext.dll`

**Copy all of those and paste them into the same directory as the executable.**

#### Building in other operating systems has not been tested.

Now the game should work. If something still doesn't work, then you have either done a step incorrectly, or these steps don't specifically work for you. (*In which case you'll have to unfortunately figure it out on your own.*)


### CMake

There is no CMake script yet.

### MSVC (Visual Studio)

![Trolled!!!](https://cdn.discordapp.com/emojis/438780424047558657.png?v=1)
