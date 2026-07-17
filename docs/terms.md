# Terms

Short definitions for this fork and Writerdeck. Product twin: [Writerdeck docs/terms.md](https://github.com/bjornte/Writerdeck-for-reMarkable/blob/main/docs/terms.md).

## Product

Writerdeck. Wi-Fi Markdown typewriter on a first-gen reMarkable. This repo is its tablet editor.

Writerdeck-server. Always-on program that talks to this app over a unix socket.

keywriter. Dave Singleton’s original Qt Markdown notepad. Upstream of this fork.

Lobby. In-app home on the tablet — not the stock reMarkable UI.

Document integrity. Notes must survive as plain Markdown on disk.

## Editor

QML. Screen and applying edits.

C++ / EditHelper. Startup, socket keys, and the math for chords, wrap, and undo.

TextEdit. Qt’s on-screen text box. Fine for drawing; weak for visual-line APIs.

Visual line / goal column. A wrapped row on screen, and the horizontal target Up/Down tries to keep.

assemble-qml.sh. Builds committed `main.qml` from modular pieces. Run it after changing helpers or Lobby; CI does not stitch QML.

## Testing and git

Keyboard harness. Automated on-device typing checks in the Writerdeck project.

Critical / full suite. Smaller “basic editing works” gate; full product sign-off.

Ship tip. A known-good commit of this fork. Day-to-day builds usually track `master`.

Upstream / merge-base. Dave’s repo, and the shared ancestor that makes ordinary merges possible.
