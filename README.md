# ncurses Darklands save-game editor

I found a [site from vvendigo](https://wendigo.online-siesta.com/darklands/)
where he collected resources, including data offsets,
which I used to make this editor.

He also has a [github](https://github.com/vvendigo).

## Installation
Just type make to use the Makefile. You can move the binary where you like.

The only dependency, besides a C compiler, is ncurses.

## Usage
This is a menu- and keyboard-driven console application.

Set the path to your `DARKLANDS.LST` file,
this is required to get the names of items.

Choose a `DKSAVEx.SAV` file, there are also `.BST` files, use the `.SAV` file.

The file is opened for reading and then immediately closed.
You can then view/modify the values.
If you want to save changes,
the save option will overwrite the selected name that was opened.

## Features
This program will open a saved game and allows you to view and edit values.

There are more values which could be shown or edited,
e.g.,
it should be possible to add new items (rather than modifying existing items).

## Note
I was unable to unequip armor after modification while it was equipped.
It is probably best not to modify equipped items, i.e.,
unequip any item you want to modify the quality of before saving.

The program uses the actual datatypes, some values are 8 bits, others are 16.
After modifying a value, the value you see is the value that would be stored.

There are limits in the display of the game for some values, e.g., 99.
Using a 3-digit number where it wouldn't normally fit may have side-effects.

It might be a good idea to make a backup of a save file before modification.
