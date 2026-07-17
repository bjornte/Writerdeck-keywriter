# Writerdeck-keywriter

This is the tablet text editor inside [Writerdeck for reMarkable](https://github.com/bjornte/Writerdeck-for-reMarkable): a Wi-Fi Markdown typewriter on a first-gen reMarkable. You type from a phone (or keyboard); the tablet shows the page and keeps your notes as plain Markdown on disk.

It is a fork of Dave Singleton’s [remarkable-keywriter](https://github.com/dps/remarkable-keywriter). Writerdeck-server drives it over a unix socket. Do not install this repo alone — deploy through Writerdeck.

Ship tip (keyboard harness green): `6a15e08` — full suite 110/110, critical 38/38 (17 Jul 2026). Day-to-day builds track `master`; pin that SHA only for a known-good rollback.

![Writerdeck for reMarkable 1](docs/Writerdeck-for-reMarkable.jpg)

## What’s different from upstream

Upstream is a Qt Markdown notepad for reMarkable: USB keyboard, Esc for preview, Ctrl-K note switcher, sundown rendering.

This fork keeps that core and adds what Writerdeck needs:

Socket input. Keystrokes arrive from Writerdeck-server as synthetic Qt events. The stock kernel has no usable uinput path.

Mac and Linux edit chords. Word and line motion, shift-selection, wrap-aware Up/Down, and undo that covers both socket typing and chord edits.

EditHelper in C++. Caret math, chord dispatch, visual-line walk, and undo stacks live in `edit_helper.*`. QML still owns the on-screen TextEdit, goal column, timers, and applying results (`edit_mac_helpers.qml.inc`).

Lobby shell. Files, Home, Settings, and sleep on the tablet; file and vault ops over the same socket (`lobby/`, `lobby_bridge`).

Plain Markdown on disk. Editing stays PlainText. RichText is for preview only.

Writerdeck’s CI still inserts the edit helpers into `main.qml` and concatenates Lobby fragments at build time. New editor behavior belongs here, not in that script.

## Credit

Original keywriter: [Dave Singleton](https://github.com/dps/remarkable-keywriter). Writerdeck-specific work is LLM-assisted; behavior is checked on-device by Writerdeck’s keyboard harness. Upstream install (Toltec / standalone) stays documented in Dave’s repo.
