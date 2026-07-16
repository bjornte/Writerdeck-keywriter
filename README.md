# Writerdeck-keywriter

This repo is the core text editor of the [Writerdeck for reMarkable](https://github.com/bjornte/Writerdeck-for-reMarkable) app.
It is a Fork of [dps/remarkable-keywriter](https://github.com/dps/remarkable-keywriter) by Dave Singleton.
It is driven by Writerdeck-server over a unix socket. 

Do not install this fork as standalone keywriter; use the Writerdeck project for deploy and runtime.

![Writerdeck for reMarkable 1](docs/Writerdeck-for-reMarkable.jpg)

# Technical details

As of July 2026, all changes in this repo from the original writerdeck are vibe coded by various LLMs.

**C++ vs QML:** QML is the screen and how typing/selection works. C++ is the engine under that (app start, display, feeding keys from Writerdeck's socket). Current editing work is almost all QML.

## C++ (owned here)

Socket reader, Lobby bridge, and rotation watcher live in this tree — not applied at CI time from Writerdeck's `third_party/keywriter/`:

- `main.cpp` — unix socket → synthetic `QKeyEvent`s; `qputenv` guards so `QT_QPA_PLATFORM` / `QMLSCENE_DEVICE` can override stock epaper
- `lobby_bridge.{h,cpp}` — QML-callable file/sync/vault ops over the socket
- `rotation_watcher.{h,cpp}` — QML `rotationChanged` → server notify
- `edit.pro` — toltec `linux-arm-remarkable-g++`, those sources, `-pthread`

## Lobby / shell QML (owned here)

`main.qml` carries Writerdeck Lobby/shell behaviour (boot Lobby, Home, omni, save paths, scroll, sleep helpers, etc.). Modular Lobby UI lives under `lobby/*.inc` plus `concat-lobby.sh`. CI inserts `edit_mac_helpers.qml.inc` before `showLobby()`, then concatenates Lobby subpages + sleep screen into `main.qml`. No large Python string patches remain in Writerdeck's build script.

## `edit_mac_helpers.qml.inc`

Edit-mode caret, shift-selection, backspace/delete, wrap/visual-line, undo, combo helpers, edit/cursor property decls, `handleMacKeysOnPressed`, and the cursor/autosave Timers plus text-change Connections (QML). CI inserts this file into `main.qml` before `showLobby()`; Keys.onPressed calls the dispatcher.

Source of truth for that helper stack lives here — not as an embedded string in the Writerdeck build script.

## Upstream

Original keywriter by [Dave Singleton](https://github.com/dps/remarkable-keywriter): Qt Markdown editor for reMarkable with USB keyboard, sundown renderer, Esc edit/preview, Ctrl-K note switcher. Upstream install (Toltec / standalone) remains documented there.
