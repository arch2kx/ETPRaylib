Eden Treaty Pandemonium
=======================

Drag ETP.app onto the Applications shortcut in this folder to install it,
or just double-click ETP.app to play in place.


"ETP.app is damaged" / "cannot be opened"
-----------------------------------------

The app isn't code-signed or notarized, so macOS quarantines it after a
browser download and may claim it's damaged. It isn't.

Easiest fix - right-click (or Control-click) ETP.app and choose Open,
then confirm. You only need to do this once.

If macOS still refuses, clear the quarantine flag from Terminal:

    xattr -dr com.apple.quarantine /Applications/ETP.app

(adjust the path if you kept the app somewhere else).

Installing through the itch desktop app avoids this entirely.
