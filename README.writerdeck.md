# Writerdeck fork of remarkable-keywriter

Owned by the [Writerdeck](https://github.com/bjornte/Writerdeck-for-reMarkable) project. Upstream: [dps/remarkable-keywriter](https://github.com/dps/remarkable-keywriter). Default branch: `master`.

**C++ vs QML:** QML files are the screen and how typing/selection works. C++ is the engine under that (app start, display, feeding keys from Writerdeck’s socket). Current editing work is almost all QML.

## C++ (owned here)

Socket reader, Lobby bridge, and rotation watcher live in this tree — not applied at CI time from Writerdeck’s `third_party/keywriter/`:

- `main.cpp` — unix socket → synthetic `QKeyEvent`s; `qputenv` guards so `QT_QPA_PLATFORM` / `QMLSCENE_DEVICE` can override stock epaper
- `lobby_bridge.{h,cpp}` — QML-callable file/sync/vault ops over the socket
- `rotation_watcher.{h,cpp}` — QML `rotationChanged` → server notify
- `edit.pro` — toltec `linux-arm-remarkable-g++`, those sources, `-pthread`

## Lobby / shell QML (owned here)

`main.qml` carries Writerdeck Lobby/shell behaviour (boot Lobby, Home, omni, save paths, scroll, sleep helpers, etc.). Modular Lobby UI lives under `lobby/*.inc` plus `concat-lobby.sh`. CI inserts `edit_mac_helpers.qml.inc` before `showLobby()`, then concatenates Lobby subpages + sleep screen into `main.qml`. No large Python string patches remain in Writerdeck’s build script.

## `edit_mac_helpers.qml.inc`

Edit-mode caret, shift-selection, backspace/delete, wrap/visual-line, undo, combo helpers, edit/cursor property decls, `handleMacKeysOnPressed`, and the cursor/autosave Timers plus text-change Connections (QML). CI inserts this file into `main.qml` before `showLobby()`; Keys.onPressed calls the dispatcher.

Source of truth for that helper stack lives here — not as an embedded string in the Writerdeck build script.
