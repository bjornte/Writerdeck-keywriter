#!/usr/bin/env bash
# Assemble main.qml from main.qml.in + edit_mac_helpers.qml.inc + lobby/*.inc.
# Source of truth for modular fragments; commit the regenerated main.qml.
# Writerdeck CI does not run this - it builds the committed main.qml as-is.
set -euo pipefail
cd "$(dirname "$0")"

IN=main.qml.in
OUT=main.qml
HELPERS=edit_mac_helpers.qml.inc

test -f "$IN" || { echo "ERROR: $IN missing" >&2; exit 1; }
test -f "$HELPERS" || { echo "ERROR: $HELPERS missing" >&2; exit 1; }
test -d lobby || { echo "ERROR: lobby/ missing" >&2; exit 1; }

./concat-lobby.sh

python3 - << 'PYEOF'
with open('main.qml.in') as f:
    s = f.read()

old_show = '    function showLobby() {'
with open('edit_mac_helpers.qml.inc') as hf:
    helpers = hf.read()
if not helpers.endswith('\n'):
    helpers += '\n'

assert 'function handleMacArrow' in helpers, "edit_mac_helpers.qml.inc missing handleMacArrow"
assert 'function handleMacBackspace' in helpers, "edit_mac_helpers.qml.inc missing handleMacBackspace"
assert 'editHelper.beginTextEdit' in helpers, "edit_mac_helpers.qml.inc missing editHelper.beginTextEdit"
assert 'editHelper.notifyTextChanged' in helpers, "edit_mac_helpers.qml.inc missing editHelper.notifyTextChanged"
assert 'editHelper.dispatchMacArrow' in helpers, "edit_mac_helpers.qml.inc missing editHelper.dispatchMacArrow"
assert 'function handleMacUndo' in helpers, "edit_mac_helpers.qml.inc missing handleMacUndo"
assert 'property bool cursorStrong' in helpers, "edit_mac_helpers.qml.inc missing cursorStrong"
assert 'function handleMacKeysOnPressed' in helpers, "edit_mac_helpers.qml.inc missing handleMacKeysOnPressed"
assert 'id: cursorTimer' in helpers, "edit_mac_helpers.qml.inc missing cursorTimer"
assert 'id: autosaveTimer' in helpers, "edit_mac_helpers.qml.inc missing autosaveTimer"
assert 'Connections {' in helpers and 'onTextChanged:' in helpers, "edit_mac_helpers.qml.inc missing text-change Connections"
assert old_show in s, "function showLobby not found in main.qml.in"
assert 'function handleMacArrow' not in s, "main.qml.in already contains helpers (edit main.qml.in, not main.qml)"
s = s.replace(old_show, helpers + old_show, 1)

with open('lobby_subpages.qml.inc') as lf:
    lobby_ui = lf.read()
with open('lobby/lobby_vault_numpad.inc') as vf:
    vault_ui = vf.read()
with open('lobby/lobby_no_keyboard.inc') as nf:
    no_kb_ui = nf.read()

lobby_rect = (
    '        ListModel {\n'
    '            id: lobbyNotesModel\n'
    '        }\n'
    + lobby_ui +
    vault_ui +
    no_kb_ui +
    '        Rectangle {\n'
    '            id: sleepScreen\n'
    '            anchors.fill: parent\n'
    '            color: "white"\n'
    '            visible: isSleeping\n'
    '            z: 10\n'
    '            Column {\n'
    '                anchors.centerIn: parent\n'
    '                width: sleepScreen.width * 0.75\n'
    '                spacing: 24\n'
    '                Text {\n'
    '                    text: "Writerdeck is sleeping.\\nWi-Fi is off. Press power to wake."\n'
    '                    color: "black"\n'
    '                    font.pointSize: 18\n'
    '                    font.family: "Noto Sans"\n'
    '                    width: parent.width\n'
    '                    wrapMode: Text.WordWrap\n'
    '                    horizontalAlignment: Text.AlignHCenter\n'
    '                }\n'
    '            }\n'
    '        }'
)

quick_close = '        }\n'
end_anchor = quick_close + '    }\n}'
assert end_anchor in s, "QML end structure (quick+body+Window close) not found in main.qml.in"
assert 'id: sleepScreen' not in s, "main.qml.in already contains sleepScreen"
last_pos = s.rfind(end_anchor)
s = s[:last_pos + len(quick_close)] + lobby_rect + '\n' + s[last_pos + len(quick_close):]

hk = s.find('    function handleKey(event) {')
co = s.find('    Component.onCompleted: {')
assert hk >= 0 and co > hk, "handleKey / Component.onCompleted anchors missing"
assert s[hk:co].count('{') == s[hk:co].count('}'), "handleKey brace mismatch -- QML will fail to load"

with open('main.qml', 'w') as f:
    f.write(s)
print('assembled main.qml (helpers + lobby + sleep screen)')
PYEOF

# Drop generated concat artifact; only main.qml is the rcc input.
rm -f lobby_subpages.qml.inc
echo "  markers:"
grep -n 'property bool isLobby:\|function showLobby\|id: sleepScreen\|handleMacKeysOnPressed\|id: lobby' "$OUT" | head -20
