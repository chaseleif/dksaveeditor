# Patch to fix color bug in Darklands save games
___

## The Problem
At some point in the game, the characters colors will wildly change.
Colors can get neon hair, etc.

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
other side-effects caused by overwriting these bytes is also unknown.

## Usage
The save file will be modified in-place, make backups desired before using.

Compile `colorfix.c` and run, providing the save file as the argument:
```bash
gcc colorfix.c -o colorfix
./colorfix DKSAVE0.SAV
```
