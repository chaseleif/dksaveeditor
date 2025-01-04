# Program to fix the color bug in Darklands save games

## The Problem
At some point in the game, the characters colors will wildly change,
players can get neon hair, etc.

## Possible Cause
___From the 2 save files demonstrating the issue I have seen,___
this seems to be a problem in the bytes representing the players' colors
being shifted rightward by 2 bytes (16 bits).

## The Fix
If we shift all of the colors back by 2 bytes, the colors are corrected.

## Potential problems
The colors were pushed into the region of the first character.
So, the first the first 2 of these bytes will have been overwritten.
The purpose of these 2 bytes is unknown,
side-effects caused by overwriting these bytes is unknown.

## Not necessarily a problem
The original 4 characters begin the character section.
If these 4 characters were not killed
by selecting kill character in character creation screen,
and are not being used,
then the 2 bytes that were overwritten belong to the first of those characters.

## Usage
The save file will be modified in-place, make backups desired before using.

Compile `colorfix.c` and run, providing the save file as the argument:
```bash
gcc colorfix.c -o colorfix
./colorfix DKSAVE0.SAV
```
