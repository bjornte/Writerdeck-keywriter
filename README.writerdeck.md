# Writerdeck fork of remarkable-keywriter

Owned by the [Writerdeck](https://github.com/bjornte/Writerdeck-for-reMarkable) project. Upstream: [dps/remarkable-keywriter](https://github.com/dps/remarkable-keywriter). Default branch: `master`.

**C++ vs QML:** QML files are the screen and how typing/selection works. C++ is the engine under that (app start, display, feeding keys from Writerdeck’s socket). Current editing work is almost all QML.

## C++ (owned here)

Socket reader, Lobby bridge, and rotation watcher live in this tree — not applied at CI time from Writerdeck’s `third_party/keywriter/`:

- `main.cpp` — unix socket → synthetic `QKeyEvent`s; `qputenv` guards so `QT_QPA_PLATFORM` / `QMLSCENE_DEVICE` can override stock epaper
- `lobby_bridge.{h,cpp}` — QML-callable file/sync/vault ops over the socket
- `rotation_watcher.{h,cpp}` — QML `rotationChanged` → server notify
- `edit.pro` — toltec `linux-arm-remarkable-g++`, those sources, `-pthread`

`build-keywriter.sh` in Writerdeck asserts these are present, then runs qmake/make and still applies the large Lobby/shell QML Python patches.

## `edit_mac_helpers.qml.inc`

Edit-mode caret, shift-selection, backspace/delete, wrap/visual-line, undo, combo helpers, edit/cursor property decls, `handleMacKeysOnPressed`, and the cursor/autosave Timers plus text-change Connections (QML). CI (`build-keywriter.sh`) inserts this file into `main.qml` before `showLobby()` after other Writerdeck QML patches; Keys.onPressed calls the dispatcher.

Source of truth for that helper stack lives here — not as an embedded string in the Writerdeck build script.
