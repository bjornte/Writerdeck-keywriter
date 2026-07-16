# Writerdeck fork of remarkable-keywriter

Owned by the [Writerdeck](https://github.com/bjornte/Writerdeck-for-reMarkable) project. Upstream: [dps/remarkable-keywriter](https://github.com/dps/remarkable-keywriter). Default branch: `master`.

## `edit_mac_helpers.qml.inc`

Edit-mode caret, shift-selection, backspace/delete, wrap/visual-line, undo, and combo helpers. CI (`build-keywriter.sh`) inserts this file into `main.qml` before `showLobby()` after other Writerdeck QML patches.

Source of truth for that helper stack lives here — not as an embedded string in the Writerdeck build script.
