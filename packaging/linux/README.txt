Eden Treaty Pandemonium
=======================

To play right away:

    ./play.sh

(or double-click play.sh in your file manager, if it's set to run
executables). If it won't start, make it executable first:

    chmod +x play.sh ETP

Run ETP directly if you prefer, but launch it from this folder - the
game loads assets/ by relative path. play.sh just handles that for you.


Adding it to your application menu
----------------------------------

    ./install-shortcut.sh              app menu entry
    ./install-shortcut.sh desktop      also put an icon on your Desktop

To undo:

    ./install-shortcut.sh remove
    ./install-shortcut.sh remove desktop

The entry points back at this folder, so don't move the game afterwards -
re-run the script instead if you do.
