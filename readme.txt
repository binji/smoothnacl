SmoothLife


CHANGES since last upload:

SmoothLifeSphere: smooth time step mode works, parameters for SmoothLife L included, color schemes work again

nicer graphics for SmoothLifeSphere, the sphere is transparent now, and you can show/hide a grid


There are four versions:
- SmoothLifeFB in FreeBasic plus Windows executable
- smoothlife.m in Matlab/Octave
- SmoothLife, a C++, MSVC, OpenGL, GLSL Windows version plus executable. It should also work with CodeBlocks and gcc/MinGW. You should just have to replace _int64 with long long. GLEE is used, but not included.
- SmoothLifeSphere, which is based on an earlier version of SmoothLife without FFT.

Install by unpacking the .zip to a place where the program can write in its own directory. It writes to the logfile SmoothLifeLog.txt. This will help with debugging if the program won't start for you for some reason. You need OpenGL 3.0 for example and your OpenGL version string will appear in the logfile. It reads the shaders from the 1D,2D or 3D directories. Parameter values are read from the first line of the SmoothLifeConfig.txt file, the rest is ignored. So if you want to change default settings you have to copy the appropriate line to the top in the config file. Also, there are settings like the number of dimensions that can not be changed in the program, but only this way.

If you start the program successfully you should see the running simulation and a bunch of numbers. The first one is timing information. My screen refresh rate is 85 Hz, so I like to know how many screens a time step takes. That is the meaning of this number. You could also say it is the time measured in 1/85 seconds. Then there is the information if you are in discrete (0) or smooth (1,2) time step mode. The next numbers are: outer radius, relation outer/inner radius, relation outer radius/kernel step width and time step (ignored for discrete mode).

The second line shows the values for the sigmoid function: Birth and death intervals, step function types and step widths. You can change the birth/death values with keys and observe the effects in real-time. That's the whole point: to find interesting birth/death values. Use the keys q/a, w/s, e/d, r/f to raise or lower respectively the four values. If you have found interesting values, you can save them to the config file by pressing "m". You can start next time with these values by moving the last line to the top. The complete key settings can be found in the comment section at the top of the main.cpp file. You change from 2D to 3D or 1D by editing the first number in the config file.

further examples, descriptions and details
youtube.com/user/ionreq
vimeo.com/user10202028
