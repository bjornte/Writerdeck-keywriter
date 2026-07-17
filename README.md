# Writerdeck-keywriter

This is the tablet text editor inside [Writerdeck for reMarkable](https://github.com/bjornte/Writerdeck-for-reMarkable): a Markdown typewriter on a first-gen reMarkable for USB and Bluetooth keyboards.

It is a fork of Dave Singleton’s [remarkable-keywriter](https://github.com/dps/remarkable-keywriter). Writerdeck-server drives it over a unix socket. Do not install this repo alone — deploy through Writerdeck.

Last known-good editor build for automated typing tests: commit `0bb3b70` (all 110 checks passed, including the 38 “basic editing” ones). Everyday builds follow the `master` branch; use that commit hash only if you need to roll back to a proven binary.

Terms: [docs/terms.md](docs/terms.md).

![Writerdeck for reMarkable 1](docs/Writerdeck-for-reMarkable.jpg)

## Testing

The Writerdeck project runs an on-device keyboard harness against this editor: scripted keystrokes over the same unix socket a phone uses, then asserts caret, selection, and text. About 110 scenarios; a critical subset of 38 is the “basic editing works” gate. A separate edit-session smoke test only checks that opening a note does not crash the app.

Harness code and scoreboard live in Writerdeck (`scripts/test-keyboard-harness.sh`, `docs/editor-testing/`). This repo does not run those tests alone.

## What’s different from the original

Dave’s original is a Qt Markdown notepad for reMarkable: USB keyboard, Esc for preview, Ctrl-K note switcher, sundown rendering.

This fork keeps that core and adds what Writerdeck needs:

Socket input. Keystrokes arrive from Writerdeck-server as synthetic Qt events. The stock kernel has no usable uinput path.

Mac and Linux edit chords. Word and line motion, shift-selection, wrap-aware Up/Down, and undo that covers both socket typing and chord edits.

EditHelper in C++. Caret math, chord dispatch, visual-line walk, and undo stacks live in `edit_helper.*`. QML still owns the on-screen TextEdit, goal column, timers, and applying results (`edit_mac_helpers.qml.inc`).

Lobby shell. Files, Home, Settings, and sleep on the tablet; file and vault ops over the same socket (`lobby/`, `lobby_bridge`).

Plain Markdown on disk. Editing stays PlainText. RichText is for preview only.

QML assembly. Edit helpers and Lobby fragments are modular (`edit_mac_helpers.qml.inc`, `lobby/*.inc`, skeleton `main.qml.in`). After changing those, run `./assemble-qml.sh` and commit the regenerated `main.qml` (`qml.qrc` loads that file). Writerdeck CI only clones, asserts, and builds — it does not stitch QML. New editor behavior belongs here, not in Writerdeck’s build script.

## Pulling in Dave’s updates

History is linked to [dps/remarkable-keywriter](https://github.com/dps/remarkable-keywriter) (merge `5946cae`; tree unchanged). Pull from the original on purpose, not every session. In git the remote is often named `upstream` — that just means Dave’s repo:

```bash
git remote add upstream https://github.com/dps/remarkable-keywriter.git   # once
git fetch upstream
git merge upstream/master
# resolve conflicts in favor of Writerdeck where edit/Lobby/socket diverged
git push origin master
```

Prefer a merge commit. Then rebuild Writerdeck via its CI, deploy, and run that project’s edit-session and keyboard harness checks.

## Credit

Original keywriter: [Dave Singleton](https://github.com/dps/remarkable-keywriter). Writerdeck-specific work is LLM-assisted; behavior is checked on-device by Writerdeck’s keyboard harness (see Testing above). How to install Dave’s original on its own (Toltec / standalone) stays in his repo.
