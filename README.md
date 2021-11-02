# Rubik-s-Cube
![demo](https://cdn.discordapp.com/attachments/504995142550290433/905109275976798228/2021-11-02-155148_1044x766_scrot.png)

3d rubik's cube in Opengl using quaternions

This was written for fun back when I was a student using a now deprecated version of opengl

# Instructions
Hold the middle mouse button to rotate the entire cube, use cube notation to make moves:

frulbd ; Front Right Upwards Left Back Down ; Capital letter for counter-clockwise rotation

The program also supports animations, eg. `rubik.story("urdtfl");` but this requires recompilation


# Installation
`make`; You need either nix, or a C++ compiler, sfml and mesa_glu
