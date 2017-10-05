# Haxe VLC/OpenFL integration

Superbasic video player using C++/VLC:

 * Early version, expect a rewrite, not the best solution
 * Generic VLC Externs in the making
 * Tested with OpenFL 6.2.0 / Lime 5.6.0

As you all know OpenFL does not have a video player for the native c++ target. This example uses the the VLC libraries in c++ and creates a "VLCBitmap" that you can add to the OpenFL displaylist along other items.

Generic playback functions is supported: play, stop, seek, time etc..


### Windows only ..for now

 * Comes with a FD/HD project setup, but you can use whatever you want
 * Linux / Mac / Android version could come at some point, should be easy to implement
 * Does not need to have anything installed (Does not need the VLC player installed)
 * Must have the dll's and the "plugin" folder present in the application dir
 * Consider this more of a test than an actual finished library
 * VLC does not handle decoding using GPU that well.. so 4k works, but can be challenging
 * In essence, has the same features as the VLC player
 * Can also open a seperate window to play the video using DirectDraw (Better performance)
 * Can play videos in fullscreen using DirectDraw (Better performance)
