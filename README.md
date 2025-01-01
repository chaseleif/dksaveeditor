# ncurses Darklands save-game editor

I found a [site from vvendigo](https://wendigo.online-siesta.com/darklands/)
where he collected resources,
including the data offsets I used to make this editor.

He also has a [github](https://github.com/vvendigo).

## Installation
Just type `make` to use the Makefile. You can move the binary where you like.

You can set the path to the game directory during compilation,
this will embed item, saint, and formula information in the binary.
Including the game data in the compiled binary
removes the need to set the path and load the data each application run.

To use this feature,
set the environment variable `DARKLANDS`
to the game directory before using make.

## Usage
This is a menu- and keyboard-driven console application.

If not compiled with the Darklands data,
the Darklands path must be set to enable item, saint, and formula functions.

Choose a `DKSAVEx.SAV` file.

The file is opened for reading and then immediately closed.
You can then view/modify the saved game.

If you want to save changes,
the save option will overwrite the selected name that was opened.

## Features
This program will open a saved game and allows you to:
- View and edit party and player stats
- View and edit player items' quality and quantity
- Add items
- View, add, and read descriptions of saints
- View, add, and read descriptions of formulas

## Note
I was unable to unequip armor after modification while it was equipped.
It may have made the armor immune to quality damage.
It is probably best not to modify equipped items, i.e.,
unequip any item you want to modify the quality of before saving.

The program displays the values which will be written after modification, i.e.,
after modifying a value, the value you see is the value that would be stored.

Using bad values could cause problems.
There are limits in the display of the game for some values, e.g., 99.
Using a 3-digit number where it wouldn't normally fit may have side-effects.

It is a good idea to make a backup of a save file before modification.
