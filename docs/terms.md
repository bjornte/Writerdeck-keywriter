# Terms

Short definitions for reading this fork and the Writerdeck project. Product docs: [Writerdeck for reMarkable](https://github.com/bjornte/Writerdeck-for-reMarkable).

## Product

Writerdeck. The full product: phone typing page, always-on server on the tablet, and this editor app. Saves notes as plain Markdown.

Writerdeck-server (daemon). Background program on the tablet. Handles Wi-Fi, files, sync, PIN, and launches the editor. Speaks to this app over a unix socket.

Writerdeck (the app). This binary on the tablet — the full-screen editor you see. Built from this fork.

keywriter / remarkable-keywriter. Dave Singleton’s original Qt Markdown notepad for reMarkable. This repo is a fork of that.

Fork. Our copy of keywriter with Writerdeck-specific changes. Upstream is Dave’s original.

Lobby. The tablet home screen inside Writerdeck (files, settings, sync status) — not the stock reMarkable UI.

Document integrity. The rule that your prose must survive as plain Markdown on disk — no accidental HTML, no silent empties.

## Editor tech

QML. The language for most of what you see: layout, Lobby, caret on screen, applying edit results.

C++. Lower-level code: app start, display, socket→keystrokes, and EditHelper math.

EditHelper. C++ “brain” for caret math, shortcut dispatch, wrapped-line motion, and undo. QML is the hands.

TextEdit. Qt’s on-screen text box. Fine for drawing; weak for visual-line APIs — hence some calibrated fudge.

PlainText / RichText. Edit mode stays plain text. Preview may use rich rendering. Disk stays Markdown.

Chord. A shortcut that holds modifiers (Ctrl, Alt, Shift) with a key — e.g. Alt+Left for previous word.

Visual line vs logical line. A logical line ends at a newline character. A visual line is one wrapped row on screen. Up/Down follow visual lines when text wraps.

Goal column. Remembered horizontal target when you move Up/Down through lines of different lengths (like Mac/Linux editors).

Undo stack. Our own undo/redo history in EditHelper (Qt’s built-in undo is sidelined so socket typing and chords share one history).

Socket input. Keys arrive as messages on `/run/Writerdeck.sock`, turned into synthetic Qt key events. Not the Linux uinput device (unavailable on this kernel).

assemble-qml.sh. Script in this repo that builds committed `main.qml` from `main.qml.in`, edit helpers, and Lobby fragments. Run it after changing those pieces; commit the result. CI does not stitch QML.

## Testing

Keyboard harness. Automated on-device tests in the Writerdeck project. They drive the real editor over the same socket path a phone would use, then check caret, selection, and text.

Critical suite. The smaller “basic editing works” gate (38 scenarios). Must stay green before claiming a deploy is OK.

Full suite. The complete keyboard check (110 scenarios), including wrap and undo tags. Product sign-off.

Edit-session test. Separate smoke check: open a note and confirm the editor stays up (catches broken QML that crashes on launch).

Ship tip. A known-good git commit of this fork that last passed the harness. Day-to-day builds usually track `master`; pin a tip only for rollback.

## Git

Upstream. Dave’s remarkable-keywriter remote. Pull on purpose; prefer Writerdeck behavior where the trees diverged.

Merge-base. Shared ancestor commit that makes ordinary `git merge` possible. This fork’s history is linked to upstream so that works.
