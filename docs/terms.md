# Terms

Short definitions for this fork and Writerdeck. Product twin: [Writerdeck docs/terms.md](https://github.com/bjornte/Writerdeck-for-reMarkable/blob/main/docs/terms.md).

## Product

Writerdeck. A Markdown typewriter on a first-gen reMarkable for USB and Bluetooth keyboards. This repo is its tablet editor.

Writerdeck-server. Always-on program that talks to this app over a unix socket.

keywriter. Dave Singleton’s original Qt Markdown notepad — the project this fork started from.

Lobby. In-app home on the tablet — not the stock reMarkable UI.

Document integrity. Notes must survive as plain Markdown on disk.

## Editor

QML. Screen language — layout and applying edits on screen.

C++ / EditHelper. Startup, socket keys, and the math for shortcuts, wrap, and undo.

Text box (Qt TextEdit). Qt’s on-screen editor control. Fine for drawing; weak for “which wrapped row am I on?”

Visual line / goal column. A wrapped row on screen, and the horizontal target Up/Down tries to keep.

assemble-qml.sh. Builds committed `main.qml` from modular pieces. Run it after changing helpers or Lobby; the automatic build does not stitch QML.

## Testing and git

Automated typing tests. Scripted keystrokes on the real tablet in the Writerdeck project (`test-keyboard-harness.sh`).

Basic set / full set. Thirty-eight checks for “basic editing works”; one hundred ten checks before calling typing work done.

Known-good commit. A fork revision that last passed those typing tests. Everyday builds usually follow `master`.

The original / shared history. Dave’s remarkable-keywriter repo, and the shared git starting point that makes ordinary merges possible. Developers often nickname Dave’s remote `upstream`; it still means the original.
