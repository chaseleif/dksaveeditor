# ncurses Darklands save-game editor
<b>Darklands is a historical fantasy role-playing video game
developed and published by MicroProse in 1992 for MS-DOS.</b>
___

I found a [site from wendigo](https://wendigo.online-siesta.com/darklands/)
where he collected resources,
including the data offsets I used to make this editor.

He also has a [github](https://github.com/vvendigo).

## Installation
Just type `make` to use the Makefile.
The binary can be relocated, e.g., into the Darklands save directory.

If the path to the game directory is set during compilation,
the item, saint, and formula information will be embedded in the binary.
Including the game data in the compiled binary
removes the need to set the path and load the data each application run.

To use this feature,
set the environment variable `DARKLANDS`
to the game directory before using make.

## Usage
This is a menu- and keyboard-driven console application.

Use the up, down, home, and end keys for menu item selection.
In all menus, a 'q' (quit), 'c' (cancel), escape, etc., can be used to return.
The enter key typically selects a menu item.
In the saint and formula description screens,
the enter key closes the description (as well as q, c, etc.).
In input (number) prompts, use the backspace and number keys to modify input.
The enter key is used to confirm input.
Empty input for a field will cancel the change operation.
Certain prompts will have a hint, e.g.,
in the "add" menus, the 'a' key is used to add the highlighted selection.

If not compiled with the Darklands data,
the Darklands path must be set to enable item, saint, and formula functions.

Choose a `DKSAVEx.SAV` file.

The file is opened for reading and then immediately closed.
The saved game is then available to be viewed and modified.

When saving changes,
the save option will overwrite the selected name that was opened.

Using the color-fix or resetting players' alchemy/saint bonuses
is still done in-memory.
To make the changes permanent you must "Save all changes."

## Features
This program will open a saved game and allows to:
- View and edit party and player stats
- View and edit the city contents seed (for randomizing things in the game)
- Patch the save to fix the color bug
- View and edit player's age, attributes, skills, items, saints, formulas
- View and edit player items' quality and quantity
- Add items to a player's inventory
- Clear formula and saint bonuses to a player's fire resistance, weapons, armor

## Note
The items equipped are referenced by their type and quality.
Modification of an equipped item breaks this reference.
You will be unable to unequip items if this reference is broken.
You should unequip any items before modification,
i.e., unequip items before creation of the save file.

The program displays the values which will be written after modification, i.e.,
after modifying a value, the value shown is the value that would be stored.

Using bad values could cause problems.
There are limits in the display of the game for some values, e.g., 99.
Using a 3-digit number where it wouldn't normally fit may have side-effects.

It is a good idea to make a backup of a save file before modification.
