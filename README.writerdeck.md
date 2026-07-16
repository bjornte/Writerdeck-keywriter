# Writerdeck fork of remarkable-keywriter

Owned by the [Writerdeck](https://github.com/bjornte/Writerdeck-for-reMarkable) project. Upstream: [dps/remarkable-keywriter](https://github.com/dps/remarkable-keywriter). Default branch: `master`.

**C++ vs QML:** QML files are the screen and how typing/selection works. C++ is the engine under that (app start, display, feeding keys from Writerdeck’s socket). Current editing work is almost all QML.

## `edit_mac_helpers.qml.inc`

Edit-mode caret, shift-selection, backspace/delete, wrap/visual-line, undo, combo helpers, edit/cursor property decls, and `handleMacKeysOnPressed` (QML). CI (`build-keywriter.sh`) inserts this file into `main.qml` before `showLobby()` after other Writerdeck QML patches; Keys.onPressed calls the dispatcher.

Source of truth for that helper stack lives here — not as an embedded string in the Writerdeck build script.
