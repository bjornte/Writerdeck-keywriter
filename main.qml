import QtQuick 2.11
import QtQuick.Window 2.2
import Qt.labs.folderlistmodel 1.0
import io.singleton 1.0

Window {
    id: root
    visible: true
    title: qsTr("edit")
    width: screen.width
    height: screen.height

    property int rotation: 0
    property string doc: "# reMarkable key-writer"
    property int mode: 1
    property int lastCursorPostion: -1
    property bool ctrlPressed: false
    property bool isOmni: false
    property bool isLobby: true
    property bool isSleeping: false
    property string lobbyIP: ""
    property string lobbyPIN: ""
    property int paraSpacing: 28
    property string readFont: "Inter"
    property bool lobbySyncOn: false
    property string lobbySyncRepo: ""
    property int lobbyNoteCount: 0
    property string lobbyLastSync: ""
    property bool lobbySyncReady: false
    property bool lobbySyncing: false
    property string lobbySyncError: ""
    property bool lobbyWifi: true
    property string lobbyKeyboardLayout: "us"
    property string lobbyPinDigits: "6"
    property string lobbySettingsMode: ""
    property int lobbyPage: 0
    property var lobbyTabLabels: ["Files", "Keyboard", "Sync", "Settings", "Shortcuts", "Home"]
    property int lobbyFilesIndex: 0
    property string lobbyLastEditedFile: ""
    property string lobbyFilesMode: ""
    onLobbyFilesModeChanged: writerdeck.notifyLobbyInput(lobbyFilesMode)
    property string lobbyFilesInput: ""
    property int lobbyFilesInputPos: 0
    property bool lobbyOpenInReadMode: false
    property bool lobbyEncryptionEnabled: false
    property string lobbyVaultError: ""
    property string vaultOverlayMode: ""
    property string vaultOverlayReason: ""
    property string vaultPinInput: ""
    property string vaultPinPending: ""
    property bool vaultPinKeepSession: false
    property string vaultPendingLoad: ""
    property string vaultPendingAction: ""
    property string vaultPendingNote: ""
    property string omniQuery: ""
    property string currentFile: ""
    property string folder: "file://%1/Writerdeck-user-documents/".arg(home_dir)

    function isHtmlPayload(t) {
        if (!t || t.length < 9) return false
        var head = t.substring(0, Math.min(256, t.length)).toLowerCase()
        return head.indexOf("<!doctype html") === 0
            || head.indexOf("<html") === 0
            || t.indexOf('name="qrichtext"') >= 0
    }

    function sanitizeLoadedNote(t) {
        if (!isHtmlPayload(t)) return t
        console.log("sanitize: stripping HTML wrapper from loaded note")
        var plain = t
        plain = plain.replace(/<br\s*\/?>/gi, "\n")
        plain = plain.replace(/<\/p>/gi, "\n")
        plain = plain.replace(/<\/div>/gi, "\n")
        plain = plain.replace(/<\/li>/gi, "\n")
        plain = plain.replace(/<[^>]+>/g, "")
        plain = plain.replace(/&nbsp;/g, " ")
        plain = plain.replace(/&amp;/g, "&")
        plain = plain.replace(/&lt;/g, "<")
        plain = plain.replace(/&gt;/g, ">")
        plain = plain.replace(/&quot;/g, '"')
        plain = plain.replace(/&#(\d+);/g, function(_, n) { return String.fromCharCode(parseInt(n, 10)) })
        plain = plain.replace(/\n{3,}/g, "\n\n")
        return plain.replace(/^\s+|\s+$/g, "")
    }

    function toggleMode() {
        if (mode == 0) {
            mode = 1
            syncQueryDisplay()
            query.cursorPosition = lastCursorPostion == -1 ? query.length : lastCursorPostion
            if (currentFile !== "") writerdeck.notifyOpen(currentFile)
        } else {
            doc = query.text
            lastCursorPostion = query.cursorPosition
            mode = 0
            syncQueryDisplay()
            saveFile()
            if (currentFile !== "") writerdeck.notifyReadOpen(currentFile)
        }
    }

    function doLoad(name) {
        var xhr = new XMLHttpRequest
        xhr.open("GET", "http://127.0.0.1:8000/api/notes/" + encodeURIComponent(name))
        xhr.onreadystatechange = function () {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                if (xhr.status === 423) {
                    vaultPendingLoad = name
                    vaultBeginPIN("Enter PIN to open this note", true)
                    return
                }
                if (xhr.status !== 200) {
                    var errMsg = "Could not open note"
                    if (xhr.status === 500 && name.indexOf(".md.enc") >= 0)
                        errMsg = "Cannot decrypt: wrong vault key or corrupted file"
                    vaultOpFailed(errMsg)
                    return
                }
                var response = sanitizeLoadedNote(xhr.responseText)
                isOmni = false
                mode = 1
                currentFile = name
                doc = response
                autosaveSnapshot = response
                if (lobbyOpenInReadMode) {
                    mode = 0
                    lobbyOpenInReadMode = false
                    syncQueryDisplay()
                    writerdeck.notifyReadOpen(name)
                } else {
                    harnessSetWidth(0)
                    syncQueryDisplay()
                    writerdeck.notifyOpen(name)
                }

            }
        }
        xhr.send()
    }

    function saveFile() {
        if (currentFile === "") return 0
        var content = (mode == 1) ? query.text : doc
        if (isHtmlPayload(content)) {
            console.log("save rejected: HTML/qrichtext payload for " + currentFile)
            return 0
        }
        if (mode == 1) doc = content
        console.log("Save " + currentFile)
        var url = "http://127.0.0.1:8000/api/notes/" + encodeURIComponent(currentFile)
        console.log(url)
        var request = new XMLHttpRequest()
        request.open("PUT", url, false)
        request.setRequestHeader("Content-Type", "application/json")
        request.send(JSON.stringify({ content: content }))
        console.log("save -> " + request.status + " " + request.statusText)
        return request.status
    }

    function openNotePicker() {
        isOmni = true
        omniQuery = ""
    }

    function saveAndLoad(name) {
        if (name && name.indexOf(".md.enc") >= 0 && currentFile !== name) {
            vaultPendingLoad = name
            vaultBeginPIN("Enter PIN to edit encrypted note", true)
            return
        }
        var wasLobby = isLobby
        isLobby = false
        if (!wasLobby) {
            if (mode == 1) doc = query.text
            if (currentFile !== "") saveFile()
        } else if (currentFile !== name) {
            currentFile = ""
        }
        doLoad(name)
    }

    function saveAndQuit() {
        if (mode == 1) doc = query.text
        saveFile()
        Qt.quit()
    }

    function setLobbyInfo(ip, pin, syncOn, syncRepo, noteCount, lastSync, syncReady, syncing, keyboardLayout, pinDigits) {
        lobbyIP = ip
        lobbyPIN = pin
        lobbySyncOn = !!syncOn
        lobbySyncRepo = syncRepo || ""
        lobbyNoteCount = noteCount || 0
        lobbyLastSync = lastSync || ""
        lobbySyncReady = !!syncReady
        lobbySyncing = !!syncing
        lobbyKeyboardLayout = keyboardLayout || "us"
        lobbyPinDigits = pinDigits || "6"
    }

    function setEncryptionEnabled(enabled) {
        lobbyEncryptionEnabled = !!enabled
    }

    function vaultOpFailed(msg) {
        lobbyGoPage(0)
        lobbyVaultError = msg || "Operation failed"
    }

    function vaultOnPINAccepted() {
        lobbyVaultError = ""
        if (vaultPendingAction === "encrypt") {
            var encName = vaultPendingNote
            vaultPendingAction = ""
            vaultPendingNote = ""
            if (encName) {
                selectNoteByName(encName)
                writerdeck.encryptNote(encName)
            }
            return
        }
        if (vaultPendingAction === "decrypt") {
            var decName = vaultPendingNote
            vaultPendingAction = ""
            vaultPendingNote = ""
            if (decName) {
                selectNoteByName(decName)
                writerdeck.decryptNote(decName)
            }
            return
        }
        if (vaultPendingAction === "new-encrypted") {
            vaultPendingAction = ""
            lobbyFilesMode = "new-encrypted"
            lobbyFilesInput = ""
            lobbyFilesInputPos = 0
            return
        }
        if (vaultPendingLoad !== "") {
            var pending = vaultPendingLoad
            var readMode = lobbyOpenInReadMode
            vaultPendingLoad = ""
            if (readMode) lobbyOpenInReadMode = false
            Qt.callLater(function() {
                if (isLobby) {
                    if (readMode) {
                        isLobby = false
                        if (mode == 1) doc = query.text
                        saveFile()
                        lobbyOpenInReadMode = true
                        doLoad(pending)
                    } else {
                        isLobby = false
                        currentFile = ""
                        doLoad(pending)
                    }
                } else {
                    doLoad(pending)
                }
            })
        }
    }

    function setLobbySyncStatus(syncError, wifi) {
        lobbySyncError = syncError || ""
        lobbyWifi = !!wifi
    }

    function lobbyGoPage(idx) {
        if (idx < 0 || idx >= lobbyTabLabels.length) return
        lobbyPage = idx
        lobbyFilesMode = ""
        lobbyFilesInput = ""
        lobbyFilesInputPos = 0
        lobbySettingsMode = ""
        if (idx === 0) lobbyRefreshNotes()
    }

    function lobbyRefreshNotes() {
        writerdeck.requestNotesList()
    }

    function setNotesList(items) {
        lobbyNotesModel.clear()
        if (!items) return
        for (var i = 0; i < items.length; i++) {
            var it = items[i]
            lobbyNotesModel.append({
                name: it.name !== undefined ? it.name : "",
                size: it.size !== undefined ? it.size : 0,
                modified: it.modified !== undefined ? it.modified : "",
                encrypted: !!it.encrypted
            })
        }
        if (lobbyLastEditedFile !== "") {
            if (!selectNoteByName(lobbyLastEditedFile))
                lobbyFilesIndex = Math.max(0, lobbyNotesModel.count - 1)
            lobbyLastEditedFile = ""
        } else if (lobbyFilesIndex >= lobbyNotesModel.count) {
            lobbyFilesIndex = Math.max(0, lobbyNotesModel.count - 1)
        }
    }

    function selectNoteByName(name) {
        for (var i = 0; i < lobbyNotesModel.count; i++) {
            if (lobbyNotesModel.get(i).name === name) {
                lobbyFilesIndex = i
                return true
            }
        }
        return false
    }

    function encryptNoteByName(name) {
        if (!selectNoteByName(name)) return
        lobbyEncryptSelected()
    }

    function decryptNoteByName(name) {
        if (!selectNoteByName(name)) return
        lobbyDecryptSelected()
    }

    function lobbyOpenSelected() {
        if (lobbyNotesModel.count === 0) return
        var row = lobbyNotesModel.get(lobbyFilesIndex)
        if (!row || row.name === "") return
        if (row.encrypted) {
            vaultPendingLoad = row.name
            vaultBeginPIN("Enter PIN to edit encrypted note", true); return }
        saveAndLoad(row.name)
    }

    function lobbyReadSelected() {
        if (lobbyNotesModel.count === 0) return
        var row = lobbyNotesModel.get(lobbyFilesIndex)
        if (!row || row.name === "") return
        if (row.encrypted) {
            vaultPendingLoad = row.name
            lobbyOpenInReadMode = true
            vaultBeginPIN("Enter PIN to read encrypted note", true); return }
        isLobby = false
        if (mode == 1) doc = query.text
        saveFile()
        lobbyOpenInReadMode = true
        doLoad(row.name)
    }

    function lobbyFilesInputDisplay() {
        var p = lobbyFilesInputPos
        if (p < 0) p = 0
        if (p > lobbyFilesInput.length) p = lobbyFilesInput.length
        return lobbyFilesInput.slice(0, p) + "_" + lobbyFilesInput.slice(p)
    }

    function lobbyFilesStripSuffix(name) {
        if (name.endsWith(".md.enc")) return name.slice(0, -7)
        if (name.endsWith(".md")) return name.slice(0, -3)
        return name
    }

    function lobbyFilesBeginNew() {
        lobbyFilesMode = "new"
        lobbyFilesInput = ""
        lobbyFilesInputPos = 0
    }

    function lobbyFilesBeginRename() {
        if (lobbyNotesModel.count === 0) return
        var n = lobbyNotesModel.get(lobbyFilesIndex).name
        lobbyFilesInput = lobbyFilesStripSuffix(n)
        lobbyFilesInputPos = lobbyFilesInput.length
        lobbyFilesMode = "rename"
    }

    function lobbyFilesBeginDelete() {
        if (lobbyNotesModel.count === 0) return
        lobbyFilesMode = "confirm-delete"
    }

    function lobbyFilesDoDelete() {
        if (lobbyNotesModel.count === 0) { lobbyFilesMode = ""; return }
        writerdeck.deleteNote(lobbyNotesModel.get(lobbyFilesIndex).name)
        lobbyFilesMode = ""
    }

    function lobbySettingsBeginExit() {
        lobbySettingsMode = "confirm-exit"
    }

    function lobbySettingsDoExit() {
        lobbySettingsMode = ""
        writerdeck.exitWriterdeck()
    }

    function lobbyFilesSubmitInput() {
        var name = lobbyFilesInput.trim()
        if (name === "") { lobbyFilesMode = ""; return }
        if (lobbyFilesMode === "new") {
            writerdeck.createNote(name)
            lobbyFilesMode = ""
            lobbyFilesInput = ""
            lobbyFilesInputPos = 0
        } else if (lobbyFilesMode === "rename") {
            var oldName = lobbyNotesModel.get(lobbyFilesIndex).name
            var newName = name
            if (oldName.endsWith(".md.enc")) newName = name + ".md.enc"
            writerdeck.renameNote(oldName, newName)
            lobbyFilesMode = ""
            lobbyFilesInput = ""
            lobbyFilesInputPos = 0
        } else if (lobbyFilesMode === "new-encrypted") {
            writerdeck.createEncryptedNote(name)
            lobbyFilesMode = ""
            lobbyFilesInput = ""
            lobbyFilesInputPos = 0
        }
    }

    function lobbyFilesBeginNewEncrypted() {
        vaultPendingAction = "new-encrypted"
        vaultBeginPIN("Enter PIN to create encrypted note", false)
    }

    function lobbyEncryptSelected() {
        if (lobbyNotesModel.count === 0) return
        var row = lobbyNotesModel.get(lobbyFilesIndex)
        if (!row || row.encrypted) return
        vaultPendingNote = row.name
        vaultPendingAction = "encrypt"
        vaultBeginPIN("Enter PIN to encrypt note", false)
    }

    function lobbyDecryptSelected() {
        if (lobbyNotesModel.count === 0) return
        var row = lobbyNotesModel.get(lobbyFilesIndex)
        if (!row || !row.encrypted) return
        vaultPendingNote = row.name
        vaultPendingAction = "decrypt"
        vaultBeginPIN("Enter PIN to decrypt note", false)
    }

    function vaultBeginSetup() {
        vaultPinInput = ""
        vaultPinPending = ""
        vaultOverlayReason = ""
        vaultOverlayMode = "setup"
    }

    function vaultBeginPIN(reason, keepSession) {
        vaultPinInput = ""
        vaultPinPending = ""
        vaultOverlayReason = reason || ""
        vaultPinKeepSession = !!keepSession
        vaultOverlayMode = "pin"
    }

    function vaultBeginChangePIN() {
        vaultPinInput = ""
        vaultPinPending = ""
        vaultOverlayReason = ""
        vaultOverlayMode = "change-old"
    }

    function requestVaultPIN(reason, name) {
        var msg = "Phone download: " + (name || "encrypted note")
        if (reason === "download") msg = "Enter PIN on tablet to allow phone download"
        if (name) vaultPendingLoad = name
        vaultBeginPIN(msg, false)
    }

    function vaultNumpadCancel() {
        vaultOverlayMode = ""
        vaultPinInput = ""
        vaultPinPending = ""
        vaultOverlayReason = ""
        vaultPendingAction = ""
        vaultPendingNote = ""
        vaultPendingLoad = ""
        lobbyOpenInReadMode = false
        lobbyVaultError = ""
    }

    function vaultPinDisplay() {
        var n = vaultPinInput.length
        var out = ""
        for (var i = 0; i < 6; i++) out += i < n ? "*" : "-"
        return out
    }

    function vaultNumpadTap(label) {
        if (label === "Bksp") {
            vaultPinInput = vaultPinInput.slice(0, -1)
            return
        }
        if (label === "Done") {
            vaultNumpadSubmit()
            return
        }
        if (vaultPinInput.length < 6) vaultPinInput += label
        if (vaultPinInput.length === 6) vaultNumpadSubmit()
    }

    function vaultNumpadSubmit() {
        if (vaultPinInput.length !== 6) return
        if (vaultOverlayMode === "setup") {
            vaultPinPending = vaultPinInput
            vaultPinInput = ""
            vaultOverlayMode = "confirm"
            return
        }
        if (vaultOverlayMode === "confirm") {
            if (vaultPinInput !== vaultPinPending) { vaultNumpadCancel(); return }
            writerdeck.setVaultPin(vaultPinInput)
            vaultNumpadCancel()
            return
        }
        if (vaultOverlayMode === "pin") {
            writerdeck.verifyVaultPin(vaultPinInput, vaultPinKeepSession)
            vaultOverlayMode = ""
            vaultPinInput = ""
            vaultPinPending = ""
            vaultOverlayReason = ""
            return
        }
        if (vaultOverlayMode === "change-old") {
            vaultPinPending = vaultPinInput
            vaultPinInput = ""
            vaultOverlayMode = "change-new"
            return
        }
        if (vaultOverlayMode === "change-new") {
            vaultOverlayReason = vaultPinInput
            vaultPinInput = ""
            vaultOverlayMode = "change-confirm"
            return
        }
        if (vaultOverlayMode === "change-confirm") {
            if (vaultPinInput !== vaultOverlayReason) { vaultNumpadCancel(); return }
            writerdeck.changeVaultPin(vaultPinPending, vaultPinInput)
            vaultNumpadCancel()
        }
    }

    function vaultHandleDigitKey(digit) {
        if (vaultOverlayMode === "") return false
        vaultNumpadTap(String(digit))
        return true
    }

    function vaultConsumeKey(event) {
        if (vaultOverlayMode === "") return false
        if (event.key === Qt.Key_Escape) { vaultNumpadCancel(); return true }
        if (event.key === Qt.Key_Return) { vaultNumpadSubmit(); return true }
        if (event.key === Qt.Key_Backspace) { vaultNumpadTap("Bksp"); return true }
        if (event.key >= Qt.Key_0 && event.key <= Qt.Key_9)
            return vaultHandleDigitKey(event.key - Qt.Key_0)
        if (event.text && event.text.length === 1) {
            var d = event.text.charCodeAt(0) - 48
            if (d >= 0 && d <= 9) return vaultHandleDigitKey(d)
        }
        return true
    }

    function lobbyKeyChar(event) {
        if (event.text && event.text.length === 1 && event.modifiers === Qt.NoModifier)
            return event.text
        if (event.modifiers !== Qt.NoModifier) return ""
        if (event.key >= Qt.Key_Space && event.key <= Qt.Key_AsciiTilde)
            return String.fromCharCode(event.key)
        return ""
    }

    function lobbyHandleKey(event) {
        if (isOmni) return false
        if (vaultOverlayMode !== "") {
            return vaultConsumeKey(event)
        }
        if (event.modifiers & Qt.ControlModifier) {
            if (event.key === Qt.Key_Left || event.key === Qt.Key_Right)
                return false
        }
        if (lobbyFilesMode === "confirm-delete") {
            if (event.key === Qt.Key_Escape) { lobbyFilesMode = ""; return true }
            if (event.key === Qt.Key_Return) { lobbyFilesDoDelete(); return true }
            return true
        }
        if (lobbySettingsMode === "confirm-exit") {
            if (event.key === Qt.Key_Escape) { lobbySettingsMode = ""; return true }
            if (event.key === Qt.Key_Return) { lobbySettingsDoExit(); return true }
            return true
        }
        if (lobbyFilesMode === "new" || lobbyFilesMode === "rename" || lobbyFilesMode === "new-encrypted") {
            if (event.key === Qt.Key_Escape) {
                lobbyFilesMode = ""
                lobbyFilesInput = ""
                lobbyFilesInputPos = 0
                return true
            }
            if (event.key === Qt.Key_Return) {
                lobbyFilesSubmitInput()
                return true
            }
            if (event.key === Qt.Key_Backspace) {
                if (lobbyFilesInputPos > 0) {
                    var bp = lobbyFilesInputPos
                    lobbyFilesInput = lobbyFilesInput.slice(0, bp - 1) + lobbyFilesInput.slice(bp)
                    lobbyFilesInputPos = bp - 1
                }
                return true
            }
            if (event.key === Qt.Key_Left && event.modifiers === Qt.NoModifier) {
                lobbyFilesInputPos = Math.max(0, lobbyFilesInputPos - 1)
                return true
            }
            if (event.key === Qt.Key_Right && event.modifiers === Qt.NoModifier) {
                lobbyFilesInputPos = Math.min(lobbyFilesInput.length, lobbyFilesInputPos + 1)
                return true
            }
            if (event.key === Qt.Key_Home && event.modifiers === Qt.NoModifier) {
                lobbyFilesInputPos = 0
                return true
            }
            if (event.key === Qt.Key_End && event.modifiers === Qt.NoModifier) {
                lobbyFilesInputPos = lobbyFilesInput.length
                return true
            }
            var ch = lobbyKeyChar(event)
            if (ch !== "") {
                var ip = lobbyFilesInputPos
                lobbyFilesInput = lobbyFilesInput.slice(0, ip) + ch + lobbyFilesInput.slice(ip)
                lobbyFilesInputPos = ip + 1
                return true
            }
            return true
        }
        if (event.key === Qt.Key_Tab) {
            if (event.modifiers & Qt.ShiftModifier)
                lobbyGoPage((lobbyPage + lobbyTabLabels.length - 1) % lobbyTabLabels.length)
            else
                lobbyGoPage((lobbyPage + 1) % lobbyTabLabels.length)
            return true
        }
        if (event.key >= Qt.Key_1 && event.key <= Qt.Key_6) {
            lobbyGoPage(event.key - Qt.Key_1)
            return true
        }
        if (event.key === Qt.Key_Left && event.modifiers === Qt.NoModifier) {
            lobbyGoPage((lobbyPage + lobbyTabLabels.length - 1) % lobbyTabLabels.length)
            return true
        }
        if (event.key === Qt.Key_Right && event.modifiers === Qt.NoModifier) {
            lobbyGoPage((lobbyPage + 1) % lobbyTabLabels.length)
            return true
        }
        if (lobbyPage === 3 && lobbySettingsMode === "") {
            if (!lobbyEncryptionEnabled && event.key === Qt.Key_E && event.modifiers === Qt.NoModifier) {
                vaultBeginSetup(); return true }
            if (lobbyEncryptionEnabled && event.key === Qt.Key_C && event.modifiers === Qt.NoModifier) {
                vaultBeginChangePIN(); return true }
        }
        if (lobbyPage === 0 && lobbyFilesMode === "" && lobbyEncryptionEnabled) {
            if (event.key === Qt.Key_X && event.modifiers === Qt.NoModifier) {
                lobbyEncryptSelected(); return true }
            if (event.key === Qt.Key_Y && event.modifiers === Qt.NoModifier) {
                lobbyDecryptSelected(); return true }
        }
        if (lobbyPage === 0) {
            if (event.key === Qt.Key_Up) {
                lobbyFilesIndex = Math.max(0, lobbyFilesIndex - 1)
                return true
            }
            if (event.key === Qt.Key_Down) {
                lobbyFilesIndex = Math.min(Math.max(0, lobbyNotesModel.count - 1), lobbyFilesIndex + 1)
                return true
            }
            if (event.key === Qt.Key_Return) {
                lobbyOpenSelected()
                return true
            }
            if (event.key === Qt.Key_N) {
                lobbyFilesBeginNew()
                return true
            }
            if (event.key === Qt.Key_D) {
                lobbyFilesBeginDelete()
                return true
            }
            if (event.key === Qt.Key_R && !(event.modifiers & Qt.ControlModifier)) {
                lobbyFilesBeginRename()
                return true
            }
            if (event.key === Qt.Key_V && !(event.modifiers & Qt.ControlModifier)) {
                lobbyReadSelected()
                return true
            }
        }
        return false
    }

    function prepareSleep() {
        if (mode == 1) doc = query.text
        saveFile()
        isLobby = false
        isSleeping = true
    }

    function handleHome() {
        if (isLobby) {
            Qt.quit()
        } else {
            harnessSetWidth(0)
            if (mode == 1) doc = query.text
            saveFile()
            var lastFile = currentFile
            isLobby = true
            currentFile = ""
            doc = ""
            query.text = ""
            autosaveSnapshot = ""
            lobbyFilesMode = ""
            lobbyPage = 0
            lobbyLastEditedFile = lastFile
            lobbyRefreshNotes()
        }
    }

    function readHtml(d) {
        return utils.markdown(d)
            .replace(/<p>/g, '<p style="margin-bottom:' + paraSpacing + 'px">')
            .replace(/<li>/g, '<li style="margin-bottom:8px">')
    }

    function syncQueryDisplay() {
        if (mode == 0) {
            query.textFormat = TextEdit.RichText
            query.text = readHtml(doc)
        } else {
            query.textFormat = TextEdit.PlainText
            query.text = doc
        }
    }

    function setReadFont(name) {
        readFont = name
        if (mode == 0) syncQueryDisplay()
    }

    // Phase 2C: undo/redo orchestration (stacks + merge live in editHelper)
    // Phase 2D: edit/cursor/harness state + Keys.onPressed dispatch
    // Phase B: chord -> action mapping in editHelper; QML applies layout effects
    // Phase C: visual-line math in editHelper; QML keeps goalX + cursor apply
    property bool cursorStrong: true
    property string autosaveSnapshot: ""
    property int harnessTextWidth: 0
    property int harnessDefaultQueryWidth: 0
    property bool harnessPrepareLock: false
    property real goalX: -1
    property bool goalXTrackSuspended: false
    property int lastShiftHorizKey: 0
    property int shiftAnchor: -1
    property int shiftHead: -1

    function syncEditHelperQuery() {
        editHelper.setQueryItem(query)
    }

    // Single entry for edit-mode Keys.onPressed (script injects one call).
    function handleMacKeysOnPressed(event) {
        if (mode != 1) return false
        if (handleMacBackspace(event))
            return true
        if (handleMacEditKeys(event))
            return true
        if (handleMacUndo(event))
            return true
        if (handleMacArrow(event))
            return true
        return false
    }

    function harnessSetCursor(pos) {
        if (mode != 1 || isLobby) return
        var len = query.text.length
        pos = Math.max(0, Math.min(parseInt(pos), len))
        clearShiftSelection()
        query.deselect()
        query.cursorPosition = pos
        rememberGoalX(pos)
        cursorStrong = true
        cursorTimer.stop()
    }

    function harnessSetWidth(w) {
        if (mode != 1) return
        if (harnessDefaultQueryWidth <= 0 && query.width > 0)
            harnessDefaultQueryWidth = query.width
        if (w > 0) {
            harnessTextWidth = w
            query.width = w
        } else if (harnessDefaultQueryWidth > 0) {
            harnessTextWidth = 0
            query.width = harnessDefaultQueryWidth
        }
    }

    function harnessOpenNote(name) {
        isLobby = false
        mode = 1
        currentFile = name
    }

    function harnessSandboxReset(widthPx) {
        if (currentFile === "") return
        harnessPrepareLock = true
        isLobby = false
        mode = 1
        cursorStrong = true
        cursorTimer.stop()
        if (widthPx > 0)
            harnessSetWidth(widthPx)
        else
            harnessSetWidth(0)
        var req = new XMLHttpRequest()
        req.open("GET", "http://127.0.0.1:8000/api/notes/" + encodeURIComponent(currentFile), false)
        req.send()
        if (req.status !== 200) {
            harnessPrepareLock = false
            console.log("harnessSandboxReset: GET failed " + req.status)
            return
        }
        var response = sanitizeLoadedNote(req.responseText)
        doc = response
        syncQueryDisplay()
        autosaveSnapshot = response
        if (widthPx > 0 && query.text.length > 0)
            query.positionToRectangle(query.text.length)
        query.deselect()
        query.cursorPosition = 0
        goalX = -1
        lastShiftHorizKey = 0
        clearShiftSelection()
        clearEditUndoStacks()
        syncEditUndoSnapshot()
        ctrlPressed = false
        query.forceActiveFocus()
        if (query.text.length > 0) {
            query.cursorPosition = query.text.length
            query.cursorPosition = 0
            query.deselect()
        }
        if (typeof flick !== "undefined")
            flick.contentY = 0
        try { if (query.undoStack) query.undoStack.clear() } catch (e) {}
        harnessPrepareLock = false
        syncEditHelperQuery()
    }

    function socketRouteKey(key, mods) {
        if (mode != 1 || isLobby) return
        query.forceActiveFocus()
        syncEditHelperQuery()
        key = parseInt(key)
        mods = parseInt(mods)
        var cmd = (mods & Qt.ControlModifier) !== 0
        var alt = (mods & Qt.AltModifier) !== 0
        var shift = (mods & Qt.ShiftModifier) !== 0
        var text = query.text
        var pos = query.cursorPosition
        if (!shift && cmd && !alt) {
            // Mac: Cmd+Left/Right = line; Cmd+Up/Down and Ctrl+Home/End = document.
            if (key === Qt.Key_Right) {
                moveCursorTo(macLineEndPos(pos, text), false)
                editHelper.setCursorAssoc(-1)
                return
            }
            if (key === Qt.Key_Left) {
                moveCursorTo(macLineStartPos(pos, text), false)
                editHelper.setCursorAssoc(1)
                return
            }
            if (key === Qt.Key_Up) { editHelper.setCursorAssoc(0); moveCursorTo(0, false); return }
            if (key === Qt.Key_Down) { editHelper.setCursorAssoc(0); moveCursorTo(text.length, false); return }
            if (key === Qt.Key_End) { editHelper.setCursorAssoc(0); moveCursorTo(text.length, false); return }
            if (key === Qt.Key_Home) { editHelper.setCursorAssoc(0); moveCursorTo(0, false); return }
        }
        if (!shift && !cmd && alt) {
            editHelper.setCursorAssoc(0)
            if (key === Qt.Key_Right) { moveCursorTo(wordRightPos(pos, text), false); return }
            if (key === Qt.Key_Left) { moveCursorTo(wordLeftPos(pos, text), false); return }
            if (key === Qt.Key_Up) { moveCursorTo(paragraphUpPos(pos, text), false); return }
            if (key === Qt.Key_Down) { moveCursorTo(paragraphDownPos(pos, text), false); return }
        }
        if (shift && cmd && !alt) {
            // Use shiftAnchor/shiftHead - raw query.select parks cursor at
            // selectionEnd so repeat Shift+Ctrl+Right/Down collapsed.
            cursorStrong = true
            cursorTimer.stop()
            if (key === Qt.Key_Right) {
                extendSelectionHorizontal(macLineEndPos(selectionExtendFrom(Qt.Key_Right), text))
                editHelper.setCursorAssoc(-1)
                return
            }
            if (key === Qt.Key_Left) {
                extendSelectionHorizontal(macLineStartPos(selectionExtendFrom(Qt.Key_Left), text))
                editHelper.setCursorAssoc(1)
                return
            }
            editHelper.setCursorAssoc(0)
            if (key === Qt.Key_Down || key === Qt.Key_End) {
                extendSelectionHorizontal(text.length)
                return
            }
            if (key === Qt.Key_Up || key === Qt.Key_Home) {
                extendSelectionHorizontal(0)
                return
            }
        }
        if (shift && alt && !cmd) {
            editHelper.setCursorAssoc(0)
            var from = selectionExtendFrom(key)
            var ap = from
            if (key === Qt.Key_Left) ap = wordLeftPos(from, text)
            else if (key === Qt.Key_Right) ap = wordRightPos(from, text)
            else if (key === Qt.Key_Up) ap = paragraphUpPos(from, text)
            else if (key === Qt.Key_Down) ap = paragraphDownPos(from, text)
            else return
            extendSelectionHorizontal(ap)
            cursorStrong = true
            cursorTimer.stop()
            return
        }
        var event = { key: key, modifiers: mods, accepted: false }
        if (handleMacBackspace(event)) return
        if (handleMacEditKeys(event)) return
        if (handleMacUndo(event)) return
        if (handleMacArrow(event)) return
        if (!shift && !cmd && !alt) {
            editHelper.setCursorAssoc(0)
            if (key === Qt.Key_Right) { moveCursorTo(Math.min(pos + 1, query.text.length), false); return }
            if (key === Qt.Key_Left) { moveCursorTo(Math.max(0, pos - 1), false); return }
        }
    }

    function publishEditorState() {
        var cy = 0
        try { if (typeof flick !== "undefined") cy = Math.round(flick.contentY) } catch (e) {}
        writerdeck.publishState(query.cursorPosition, query.selectionStart,
            query.selectionEnd, query.text.length, mode, isLobby ? 1 : 0,
            vaultOverlayMode, currentFile, query.text, cy)
    }

    function pageLeft() {
        if (typeof flick === "undefined") return
        flick.scrollUp()
    }

    function pageRight() {
        if (typeof flick === "undefined") return
        flick.scrollDown()
    }

    function cursorOnLastLine() {
        if (mode != 1) return false
        var len = query.text.length
        if (len === 0) return true
        var curY = query.positionToRectangle(query.cursorPosition).y
        var endY = query.positionToRectangle(len).y
        return curY >= endY - 1
    }

    function cursorOnFirstLine() {
        if (mode != 1) return false
        var curY = query.positionToRectangle(query.cursorPosition).y
        var topY = query.positionToRectangle(0).y
        return curY <= topY + 1
    }

    // Phase A1: pure string math lives in C++ EditHelper; keep names for callers.
    function isSpaceChar(c) {
        return editHelper.isSpaceChar(c)
    }

    function lineStartPos(pos, text) {
        return editHelper.lineStartPos(pos, text)
    }

    function lineEndPos(pos, text) {
        return editHelper.lineEndPos(pos, text)
    }

    function lineCharCount(lineStart, text) {
        return editHelper.lineCharCount(lineStart, text)
    }

    function rememberGoalX(pos) {
        goalX = query.positionToRectangle(pos).x
    }

    function goalXFor(pos) {
        if (goalX >= 0) return goalX
        return query.positionToRectangle(pos).x
    }

    function deleteWordLeftPos(pos, text) {
        return editHelper.deleteWordLeftPos(pos, text)
    }

    function deleteLineLeftPos(pos, text) {
        return editHelper.deleteLineLeftPos(pos, text)
    }

    function wordLeftPos(pos, text) {
        return editHelper.wordLeftPos(pos, text)
    }

    function wordRightPos(pos, text) {
        return editHelper.wordRightPos(pos, text)
    }

    function paragraphUpPos(pos, text) {
        return editHelper.paragraphUpPos(pos, text)
    }

    function paragraphDownPos(pos, text) {
        return editHelper.paragraphDownPos(pos, text)
    }

    function clearShiftSelection() {
        shiftAnchor = -1
        shiftHead = -1
    }

    function applyShiftSelection(newHead) {
        var len = query.text.length
        newHead = Math.max(0, Math.min(newHead, len))
        // Typing/replace can leave shiftAnchor past EOF or a mismatched
        // shiftHead while the caret is collapsed — re-anchor at the caret.
        if (shiftAnchor > len)
            clearShiftSelection()
        if (shiftHead >= 0 && query.selectionStart === query.selectionEnd
                && shiftHead !== query.cursorPosition)
            clearShiftSelection()
        if (shiftAnchor < 0) {
            shiftAnchor = query.cursorPosition
            shiftHead = shiftAnchor
        }
        if (shiftAnchor > len)
            shiftAnchor = len
        shiftHead = newHead
        query.select(Math.min(shiftAnchor, shiftHead), Math.max(shiftAnchor, shiftHead))
        if (shiftHead === shiftAnchor)
            clearShiftSelection()
    }

    function moveCursorTo(newPos, extend, keepGoalColumn) {
        var len = query.text.length
        var text = query.text
        newPos = Math.max(0, Math.min(newPos, len))
        if (!extend) {
            clearShiftSelection()
            query.deselect()
            // Suspend Connections onCursorPositionChanged so keepGoalColumn
            // survives landing on a shorter line (cm-line-down-goal-col).
            if (keepGoalColumn) goalXTrackSuspended = true
            query.cursorPosition = newPos
            goalXTrackSuspended = false
            if (!keepGoalColumn) rememberGoalX(newPos)
            return
        }
        applyShiftSelection(newPos)
    }

    function lineWrapsVisually(pos, text) {
        return editHelper.lineWrapsVisually(pos, text)
    }

    function extendSelectionHorizontal(newPos) {
        applyShiftSelection(newPos)
    }

    function selectionExtendFrom(key) {
        // If the caret is collapsed but shiftHead still points elsewhere, it is
        // leftover from before typing/touch/note-open — drop it. While a
        // selection is active, or shiftHead matches the caret, keep extending.
        if (shiftHead >= 0) {
            if (query.selectionStart !== query.selectionEnd)
                return shiftHead
            if (shiftHead === query.cursorPosition)
                return shiftHead
            clearShiftSelection()
        }
        var pos = query.cursorPosition
        if (query.selectionStart === query.selectionEnd) return pos
        if (key === Qt.Key_Left || key === Qt.Key_Up)
            return Math.min(query.selectionStart, query.selectionEnd)
        if (key === Qt.Key_Right || key === Qt.Key_Down)
            return Math.max(query.selectionStart, query.selectionEnd)
        return pos
    }

    // Phase B: resolve layout-dependent positions for C++ chord dispatch.
    function resolveMacPosKind(posKind, extendKey) {
        var text = query.text
        var pos = query.cursorPosition
        if (posKind === "macLineStartCursor")
            return macLineStartPos(pos, text)
        if (posKind === "macLineEndCursor")
            return macLineEndPos(pos, text)
        if (posKind === "macLineStartShiftHead")
            return macLineStartPos((shiftHead >= 0) ? shiftHead : pos, text)
        if (posKind === "macLineEndShiftHead")
            return macLineEndPos((shiftHead >= 0) ? shiftHead : pos, text)
        if (posKind === "macLineStartExtend")
            return macLineStartPos(selectionExtendFrom(extendKey), text)
        if (posKind === "macLineEndExtend")
            return macLineEndPos(selectionExtendFrom(extendKey), text)
        return pos
    }

    function applyMacArrowDispatch(r, eventKey) {
        if (!r.handled) return false
        var text = query.text
        var pos = query.cursorPosition
        var action = r.action
        if (action === "collapseSel") {
            editHelper.setCursorAssoc(0)
            var c = r.toMin
                ? Math.min(query.selectionStart, query.selectionEnd)
                : Math.max(query.selectionStart, query.selectionEnd)
            clearShiftSelection()
            query.deselect()
            query.cursorPosition = c
        } else if (action === "moveTo") {
            editHelper.setCursorAssoc(0)
            if (r.extend)
                extendSelectionHorizontal(r.pos)
            else
                moveCursorTo(r.pos, false, r.keepGoalColumn === true)
        } else if (action === "moveToResolved") {
            var p = resolveMacPosKind(r.posKind, r.extendKey)
            if (r.extend)
                extendSelectionHorizontal(p)
            else
                moveCursorTo(p, false)
            // Soft-wrap End/Cmd+Right: assoc -1 so a repeat press stays at the wrap point.
            if (r.posKind === "macLineEndCursor" || r.posKind === "macLineEndExtend")
                editHelper.setCursorAssoc(-1)
            else if (r.posKind === "macLineStartCursor" || r.posKind === "macLineStartExtend")
                editHelper.setCursorAssoc(1)
            else
                editHelper.setCursorAssoc(0)
        } else if (action === "shiftHorizDelta") {
            editHelper.setCursorAssoc(0)
            // Drop stale heads before reading them (typing leaves shiftHead
            // pointing at the pre-replace range while the caret is collapsed).
            if (shiftAnchor > text.length)
                clearShiftSelection()
            if (shiftHead >= 0 && query.selectionStart === query.selectionEnd
                    && shiftHead !== query.cursorPosition)
                clearShiftSelection()
            var headH = (shiftHead >= 0) ? shiftHead : query.cursorPosition
            var newHead = (r.delta < 0)
                ? Math.max(0, headH + r.delta)
                : Math.min(text.length, headH + r.delta)
            applyShiftSelection(newHead)
            lastShiftHorizKey = (r.eventKey !== undefined) ? r.eventKey : eventKey
        } else if (action === "shiftHorizTo") {
            // Drop stale heads before reading them (typing leaves shiftHead
            // pointing at the pre-replace range while the caret is collapsed).
            if (shiftAnchor > text.length)
                clearShiftSelection()
            if (shiftHead >= 0 && query.selectionStart === query.selectionEnd
                    && shiftHead !== query.cursorPosition)
                clearShiftSelection()
            if (shiftAnchor < 0)
                shiftAnchor = (query.selectionStart === query.selectionEnd)
                    ? pos : ((r.posKind === "macLineEndShiftHead")
                        ? Math.min(query.selectionStart, query.selectionEnd)
                        : Math.max(query.selectionStart, query.selectionEnd))
            applyShiftSelection(resolveMacPosKind(r.posKind, 0))
            if (r.posKind === "macLineEndShiftHead")
                editHelper.setCursorAssoc(-1)
            else if (r.posKind === "macLineStartShiftHead")
                editHelper.setCursorAssoc(1)
            else
                editHelper.setCursorAssoc(0)
        } else if (action === "shiftVert") {
            editHelper.setCursorAssoc(0)
            extendSelectionVertical(r.down)
        } else if (action === "moveVert") {
            editHelper.setCursorAssoc(0)
            moveCursorVertical(r.down)
        } else {
            return false
        }
        cursorStrong = true
        cursorTimer.stop()
        return true
    }

    function applyMacBackspaceDispatch(r) {
        if (!r.handled) return false
        editHelper.setCursorAssoc(0)
        if (r.action === "noop") {
            cursorStrong = true
            cursorTimer.stop()
            return true
        }
        if (r.action === "replaceText") {
            if (r.beginEdit)
                beginTextEdit()
            clearShiftSelection()
            query.text = r.text
            query.cursorPosition = r.cursor
            query.deselect()
            doc = query.text
        }
        cursorStrong = true
        cursorTimer.stop()
        return true
    }

    function applyMacEditKeysDispatch(r) {
        if (!r.handled) return false
        if (r.action === "noop") {
            cursorStrong = true
            cursorTimer.stop()
            return true
        }
        if (r.action === "selectAll") {
            query.select(0, r.len)
            doc = query.text
        } else if (r.action === "replaceText") {
            if (r.beginEdit)
                beginTextEdit()
            clearShiftSelection()
            query.text = r.text
            query.cursorPosition = r.cursor
            query.deselect()
            doc = query.text
        } else if (r.action === "insertNewline") {
            var text = query.text
            var pos = r.pos
            beginTextEdit()
            clearShiftSelection()
            query.text = text.slice(0, pos) + "\n" + text.slice(pos)
            query.cursorPosition = pos + 1
            query.deselect()
            doc = query.text
        } else {
            return false
        }
        cursorStrong = true
        cursorTimer.stop()
        return true
    }

    function visualLineDownPos(pos, gx) {
        syncEditHelperQuery()
        return editHelper.visualLineDownPos(pos, (gx !== undefined && gx >= 0) ? gx : -1)
    }

    function visualLineUpPos(pos, gx) {
        syncEditHelperQuery()
        return editHelper.visualLineUpPos(pos, (gx !== undefined && gx >= 0) ? gx : -1)
    }

    function visualLineStartPos(pos) {
        return editHelper.visualLineStartPos(pos)
    }

    function visualLineEndPos(pos) {
        return editHelper.visualLineEndPos(pos)
    }

    function onWrappedLine(pos, text) {
        return editHelper.onWrappedLine(pos, text)
    }

    function macLineStartPos(pos, text) {
        syncEditHelperQuery()
        return editHelper.macLineStartPos(pos, text)
    }

    function macLineEndPos(pos, text) {
        syncEditHelperQuery()
        return editHelper.macLineEndPos(pos, text)
    }

    function lineDownPos(pos, text) {
        var gx = goalXFor(pos)
        goalX = gx
        return visualLineDownPos(pos, gx)
    }

    function lineUpPos(pos, text) {
        if (pos <= 0) return 0
        var gx = goalXFor(pos)
        goalX = gx
        return visualLineUpPos(pos, gx)
    }

    function lineUpForSelection(head, anchor, text) {
        if (head === 0) return 0
        return lineUpPos(head, text)
    }

    function moveCursorVertical(down) {
        var pos = query.cursorPosition
        var text = query.text
        var newPos = down ? lineDownPos(pos, text) : lineUpPos(pos, text)
        if (newPos === pos) {
            if (down) {
                var gx = goalXFor(pos)
                var vis = visualLineDownPos(pos, gx)
                newPos = (vis > pos) ? vis : lineEndPos(pos, text)
            } else {
                newPos = macLineStartPos(pos, text)
            }
        }
        moveCursorTo(newPos, false, true)
    }

    function extendSelectionVertical(down) {
        var text = query.text
        if (shiftAnchor > text.length)
            clearShiftSelection()
        if (shiftHead >= 0 && query.selectionStart === query.selectionEnd
                && shiftHead !== query.cursorPosition)
            clearShiftSelection()
        if (shiftAnchor < 0) {
            shiftAnchor = query.cursorPosition
            shiftHead = shiftAnchor
            if (!down && shiftHead === text.length) {
                var upOnce = lineUpPos(shiftHead, text)
                shiftAnchor = lineStartPos(upOnce, text)
                query.select(shiftAnchor, shiftHead)
                return
            }
        }
        var head = shiftHead
        var newHead = down ? lineDownPos(head, text) : lineUpForSelection(head, shiftAnchor, text)
        // Snap to logical line end/start only when the source line actually
        // wraps visually. Mid-line on a short unwrapped line must keep goal x
        // (cm-select-line-down-mid), not jump to lineEndPos of the next line.
        if (lineStartPos(newHead, text) !== lineStartPos(head, text)) {
            if (down && newHead > head && lineWrapsVisually(head, text))
                newHead = lineEndPos(newHead, text)
            if (!down && newHead < head && lineWrapsVisually(head, text))
                newHead = lineStartPos(newHead, text)
        }
        if (down && newHead === head && head < text.length
                && text.indexOf("\n", head) === -1 && onWrappedLine(head, text)) {
            var vis = visualLineDownPos(head, goalXFor(head))
            if (vis > head) newHead = vis
        }
        // Force a 1-char selection at EOF only on a wrapped last line
        // (wrap-shift-down-last-to-eof). Short unwrapped docs stay collapsed.
        if (down && newHead === head && head === text.length && head > 0
                && shiftAnchor === shiftHead && lineWrapsVisually(head, text))
            newHead = head - 1
        applyShiftSelection(newHead)
    }

    function moveCursorEndOfLine() {
        moveCursorTo(lineEndPos(query.cursorPosition, query.text), false)
    }

    function moveCursorStartOfLine() {
        moveCursorTo(lineStartPos(query.cursorPosition, query.text), false)
    }

    function insertTextDelta(prevText, curText) {
        var r = editHelper.insertTextDelta(prevText, curText)
        return (r === undefined || r === null) ? null : r
    }

    function beginTextEdit() {
        if (harnessPrepareLock || mode != 1)
            return
        editHelper.beginTextEdit(query.text, query.cursorPosition,
                                 query.selectionStart, query.selectionEnd)
    }

    function syncEditUndoSnapshot() {
        editHelper.syncUndoSnapshot(query.text, query.cursorPosition,
                                    query.selectionStart, query.selectionEnd)
    }

    function clearEditUndoStacks() {
        editHelper.clearUndoStacks()
    }

    function isOneCharInsert(prevText, curText) {
        return editHelper.isOneCharInsert(prevText, curText)
    }

    function restoreEditState(st) {
        editHelper.beginRestore()
        query.text = st.text
        query.deselect()
        query.cursorPosition = st.cursor
        doc = query.text
        editHelper.endRestore(query.text, query.cursorPosition,
                              query.selectionStart, query.selectionEnd)
    }

    function editUndo() {
        var st = editHelper.undo(query.text, query.cursorPosition,
                                 query.selectionStart, query.selectionEnd)
        if (st === undefined || st === null) return false
        restoreEditState(st)
        return true
    }

    function editRedo() {
        var st = editHelper.redo(query.text, query.cursorPosition,
                                 query.selectionStart, query.selectionEnd)
        if (st === undefined || st === null) return false
        restoreEditState(st)
        return true
    }

    function handleMacUndo(event) {
        if (mode != 1) return false
        if (!(event.modifiers & Qt.ControlModifier)) return false
        if (event.key === Qt.Key_Z && !(event.modifiers & Qt.ShiftModifier)) {
            if (editUndo()) {
                cursorStrong = true
                cursorTimer.stop()
                event.accepted = true
                return true
            }
            return false
        }
        if (event.key === Qt.Key_Y || (event.key === Qt.Key_Z && (event.modifiers & Qt.ShiftModifier))) {
            if (editRedo()) {
                cursorStrong = true
                cursorTimer.stop()
                event.accepted = true
                return true
            }
            return false
        }
        return false
    }

    function handleMacArrow(event) {
        if (mode != 1) return false
        var r = editHelper.dispatchMacArrow(event.key, event.modifiers, query.text,
            query.cursorPosition, query.selectionStart, query.selectionEnd,
            shiftAnchor, shiftHead)
        if (!applyMacArrowDispatch(r, event.key))
            return false
        event.accepted = true
        return true
    }

    function handleMacEditKeys(event) {
        if (mode != 1) return false
        var r = editHelper.dispatchMacEditKeys(event.key, event.modifiers,
            query.text, query.cursorPosition, query.selectionStart, query.selectionEnd)
        if (!applyMacEditKeysDispatch(r))
            return false
        event.accepted = true
        return true
    }

    function handleMacBackspace(event) {
        if (mode != 1) return false
        syncEditHelperQuery()
        var r = editHelper.dispatchMacBackspace(event.key, event.modifiers, query.text,
            query.cursorPosition, query.selectionStart, query.selectionEnd)
        if (!applyMacBackspaceDispatch(r))
            return false
        event.accepted = true
        return true
    }

    // Phase 3: cursor/autosave Timers + text-change Connections (was build-keywriter.sh §8b).
    // Inserted with the rest of this file before showLobby(); ids resolve on Window.
    Timer {
        id: autosaveTimer
        interval: 45000
        repeat: true
        running: !isLobby && currentFile !== "" && mode == 1
        onTriggered: autosaveTick()
    }
    Timer {
        id: cursorTimer
        interval: 500
        repeat: false
        onTriggered: cursorStrong = true
    }
    Connections {
        target: query
        onCursorPositionChanged: {
            if (harnessPrepareLock || mode != 1 || isLobby || goalXTrackSuspended) return
            rememberGoalX(query.cursorPosition)
        }
        onTextChanged: {
            if (harnessPrepareLock || mode != 1 || isLobby) return
            editHelper.notifyTextChanged(query.text, query.cursorPosition,
                                         query.selectionStart, query.selectionEnd)
            cursorStrong = false
            cursorTimer.restart()
        }
    }
    function showLobby() {
        var lastFile = ""
        if (!isLobby) {
            harnessSetWidth(0)
            if (mode == 1) doc = query.text
            saveFile()
            lastFile = currentFile
            currentFile = ""
            doc = ""
            query.text = ""
            autosaveSnapshot = ""
        }
        isLobby = true
        lobbyFilesMode = ""
        lobbyPage = 0
        if (lastFile !== "") lobbyLastEditedFile = lastFile
        lobbyRefreshNotes()
    }

    function noteDeleted() {
        currentFile = ""
        doc = ""
        query.text = ""
        autosaveSnapshot = ""
        isLobby = true
        lobbyFilesMode = ""
        lobbyRefreshNotes()
    }

    function noteRenamed(name) {
        if (!isLobby) currentFile = name
    }

    function autosaveTick() {
        if (harnessPrepareLock || isLobby || currentFile === "" || mode != 1) return
        if (query.text === autosaveSnapshot) return
        saveFile()
        if (currentFile !== "" && !isHtmlPayload(query.text)) autosaveSnapshot = query.text
    }

    function reloadNote() {
        if (currentFile !== "") doLoad(currentFile)
    }

    function rotateScreen() {
        root.rotation = (root.rotation + 90) % 360
    }

    function initFile(name) {
        console.log("Init " + name)
        var fileUrl = folder + name + ".md"
        var request = new XMLHttpRequest()
        request.open("PUT", fileUrl, false)
        request.send("# " + name)
        console.log("save -> " + request.status + " " + request.statusText)
        return request.status
    }

    function handleKeyDown(event) {
        if (event.key === Qt.Key_Control) {
            ctrlPressed = true
        } else if (event.key === Qt.Key_K && (ctrlPressed || (event.modifiers & Qt.ControlModifier))) {
            if (isLobby) {
                if (isOmni) isOmni = false
                else { lobbyGoPage(0); openNotePicker() }
            } else isOmni = !isOmni
            event.accepted = true
        } else if (event.key === Qt.Key_R && (ctrlPressed || (event.modifiers & Qt.ControlModifier))) {
            if (isLobby) {
                rotateScreen()
                event.accepted = true
            }
        } else if (event.key === Qt.Key_Q && (ctrlPressed || (event.modifiers & Qt.ControlModifier))) {
            saveAndQuit()
        }
    }
    function handleKeyUp(event) {
        if (event.key === Qt.Key_Control) {
            ctrlPressed = false
        }
    }

    function handleKey(event) {
        if (event.key === Qt.Key_Home && event.modifiers === Qt.NoModifier) {
            // Edit mode: line start is handled on press (handleMacArrow); do not
            // treat Key_Home release as physical Home -> lobby.
            if (mode == 1 && !isLobby) {
                event.accepted = true
                return
            }
            handleHome()
            event.accepted = true
            return
        }
        if (vaultOverlayMode !== "") {
            if (vaultConsumeKey(event)) { event.accepted = true; return }
        }
        if (isLobby && !isOmni) {
            if (lobbyHandleKey(event)) {
                event.accepted = true
                return
            }
            if (!(event.modifiers & Qt.ControlModifier)) {
                event.accepted = true
                return
            }
        }
        if (event.key === Qt.Key_Escape) {
            if (isOmni) {
                isOmni = false
            } else if (!(event.modifiers & (Qt.AltModifier | Qt.ControlModifier))) {
                toggleMode()
            }
        }

        if (mode == 0 || isLobby) {
            switch (event.key) {
            case Qt.Key_Right:
                if (ctrlPressed || (event.modifiers & Qt.ControlModifier))
                    root.rotation = (root.rotation + 90) % 360
                break
            case Qt.Key_Left:
                if (ctrlPressed || (event.modifiers & Qt.ControlModifier))
                    root.rotation = (root.rotation - 90) % 360
                break
            }
        }
    }

    Component.onCompleted: {
        if (currentFile !== "") doLoad(currentFile)
    }

    Rectangle {
        rotation: root.rotation
        id: body
        width: root.rotation % 180 ? root.height : root.width
        height: root.rotation % 180 ? root.width : root.height
        anchors.centerIn: parent
        color: "white"
        border.color: "black"
        border.width: 2
        EditUtils {
            id: utils
        }
        FolderListModel {
            id: folderModel
            folder: root.folder
            nameFilters: ["*.md"]
        }

        Flickable {
            id: flick
            anchors.fill: parent
            boundsBehavior: Flickable.StopAtBounds
            contentWidth: query.paintedWidth
            bottomMargin: parent.height /2

            contentHeight: query.paintedHeight
            clip: true

            function ensureVisible(r) {
                if (contentX >= r.x) {
                    contentX = r.x
                } else if (contentX + width <= r.x + r.width) {
                    contentX = r.x + r.width - width
                }
                if (contentY >= r.y) {
                    if (r.y-height/2 > 0)
                        contentY = r.y-height/2
                    else
                        contentY = 0
                } else if (contentY + height <= r.y + r.height) {
                    contentY = r.y + r.height - height/2
                }
            }

            function scrollUp() {
                // Step with the visible viewport, not a portrait-only constant.
                // Landscape body.height is the short side; 1500px overshoots it.
                var step = Math.max(200, Math.round(height * 0.85))
                contentY -= step
                if (contentY < 0) contentY = 0
            }
            function scrollDown() {
                var step = Math.max(200, Math.round(height * 0.85))
                contentY += step
                var maxY = Math.max(0, contentHeight - height)
                if (contentY > maxY) contentY = maxY
            }

            TextEdit {
                id: query
                width: body.width
                height: body.height
                Keys.enabled: true
                wrapMode: TextEdit.Wrap
                textMargin: 44
        objectName: "writerdeckQuery"
                textFormat: mode == 0 ? TextEdit.RichText : TextEdit.PlainText
                font.family: mode == 0 ? readFont : "Noto Mono"
                focus: !isOmni && !isLobby
                renderType: Text.NativeRendering
                Component {
                    id: curDelegate
                    Item {
                        width: 9
                        visible: query.cursorVisible && cursorStrong
                        Rectangle {
                            anchors.fill: parent
                            color: "black"
                        }
                    }
                }
                cursorDelegate: curDelegate
                readOnly: mode == 0 ? true : false
                font.pointSize: mode == 0 ? 12 : 10

                onLinkActivated: {
                    console.log("Link activated: " + link)
                    doLoad(link)
                }

                Keys.onPressed: {
                    if (root.handleMacKeysOnPressed(event))
                        return
                    switch(event.key){
                        case Qt.Key_Down:
                            if (mode == 0)
                                flick.scrollDown()
                            break
                        case Qt.Key_Left:
                            if (event.modifiers === Qt.NoModifier && mode != 1) {
                                flick.scrollUp()
                            }
                            break
                        case Qt.Key_Up:
                            if (mode == 0)
                                flick.scrollUp()
                            break
                        case Qt.Key_Right:
                            if (event.modifiers === Qt.NoModifier && mode != 1) {
                                flick.scrollDown()
                            }
                            break
                        case Qt.Key_PageUp:
                            query.cursorPosition -= 100
                            break
                        case Qt.Key_PageDown:
                            query.cursorPosition += 100
                            break
                        default:
                            handleKeyDown(event)
                            break
                    }
                }

                Keys.onReleased: {
                    handleKeyUp(event)
                    handleKey(event)
                }

                onCursorRectangleChanged: {
                    if (mode == 1) {
                        var margin = 120
                        var viewTop = flick.contentY + margin
                        var viewBot = flick.contentY + flick.height - margin
                        var cy = cursorRectangle.y
                        var cb = cy + cursorRectangle.height
                        if (cy < viewTop || cb > viewBot)
                            flick.ensureVisible(cursorRectangle, margin)
                    }
                }
            }
        }

        Rectangle {
            id: quick
            z: isOmni ? 10 : 0
            anchors.centerIn: parent
            width: parent.width * 0.6
            height: parent.height * 0.6
            color: "black"
            visible: isOmni ? true : false
            radius: 20
            border.width: 5
            border.color: "gray"

            TextEdit {
                id: omniQueryTextEdit
                text: omniQuery
                textFormat: TextEdit.PlainText
                x: 40
                width: parent.width - 20
                color: "white"
                font.pointSize: 24
                font.family: "Noto Mono"
                focus: isOmni
                Keys.enabled: true
                Keys.onPressed: {
                    if (event.key === Qt.Key_Enter
                            || event.key === Qt.Key_Return) {
                        if (mode == 1) doc = query.text
                        saveFile()
                        if (!omniList.currentItem) {
                            initFile(omniQuery)
                            doLoad(omniQuery + ".md")
                        } else {
                            doLoad(omniList.currentItem.text)
                        }
                        isLobby = false
                        isOmni = false
                        event.accepted = true
                        return
                    }

                    handleKeyDown(event)
                }
                Keys.onReleased: {
                    handleKeyUp(event)
                    handleKey(event)
                    omniQuery = omniQueryTextEdit.text
                    folderModel.nameFilters = [omniQuery + "*"]
                }

                Keys.forwardTo: omniList
            }
            ListView {
                id: omniList
                anchors.top: omniQueryTextEdit.bottom
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.leftMargin: 40
                anchors.rightMargin: 40
                anchors.right: parent.right
                highlightResizeDuration: 0
                highlight: Rectangle {
                    color: "white"
                    radius: 5
                    width: 600
                }
                Component {
                    id: fileDelegate
                    Text {
                        width: parent.width
                        text: fileName
                        color: ListView.isCurrentItem ? "black" : "white"
                    }
                }

                model: folderModel
                delegate: fileDelegate
            }
        }
        ListModel {
            id: lobbyNotesModel
        }
        Rectangle {
            id: lobby
            anchors.fill: parent
            color: "white"
            visible: isLobby
            z: 5

            readonly property int pageMargin: 24
            readonly property int tabBtnHeight: 64
            readonly property int rowHeight: 72
            readonly property int actionBtnHeight: 72
            readonly property int tabSpacing: 12
            readonly property int contentSpacing: 12

            FocusScope {
                id: lobbyFocus
                anchors.fill: parent
                focus: isLobby && !isOmni
                Keys.enabled: isLobby && !isOmni
                Keys.onPressed: handleKeyDown(event)
                Keys.onReleased: {
                    handleKeyUp(event)
                    handleKey(event)
                }

                Connections {
                    target: root
                    function onIsLobbyChanged() {
                        if (isLobby) Qt.callLater(function() { lobbyFocus.forceActiveFocus() })
                    }
                }

                // ---- tab bar (touch + keyboard 1-6 / Tab / arrows) ----
                Row {
                    id: lobbyTabRow
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: lobby.pageMargin
                    anchors.rightMargin: lobby.pageMargin
                    anchors.topMargin: lobby.pageMargin
                    height: lobby.tabBtnHeight + 8
                    spacing: lobby.tabSpacing

                    Repeater {
                        model: lobbyTabLabels
                        delegate: Rectangle {
                            width: Math.max(88, (lobby.width - lobby.pageMargin * 2 - lobby.tabSpacing * (lobbyTabLabels.length - 1)) / lobbyTabLabels.length)
                            height: lobby.tabBtnHeight
                            radius: 6
                            color: lobbyPage === index ? "#e0e0e0" : "#f5f5f5"
                            border.color: lobbyPage === index ? "#999" : "#ccc"
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: (index + 1) + " " + modelData
                                font.family: "Noto Sans"
                                font.pointSize: 11
                                color: "black"
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: root.lobbyGoPage(index)
                            }
                        }
                    }
                }

                // ---- page stack ----
                Item {
                    anchors.top: lobbyTabRow.bottom
                    anchors.topMargin: lobby.contentSpacing
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: lobbyHint.top
                    anchors.bottomMargin: lobby.contentSpacing
                    anchors.leftMargin: lobby.pageMargin
                    anchors.rightMargin: lobby.pageMargin

                    // 0 Home
                    Item {
                        visible: lobbyPage === 5
                        anchors.fill: parent
                        Flickable {
                            anchors.fill: parent
                            contentWidth: width
                            contentHeight: homeCol.height
                            clip: true
                            Column {
                                id: homeCol
                                width: parent.width
                                spacing: lobby.contentSpacing
                                Text {
                                    text: "Writerdeck"
                                    color: "black"
                                    font.pointSize: 26
                                    font.family: "Noto Mono"
                                    width: parent.width
                                }
                                Text {
                                    text: "A text editor for use with a physical keyboard.\nWith Markdown support."
                                    color: "#555555"
                                    font.pointSize: 12
                                    font.family: "Noto Sans"
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                }
                                Text {
                                    text: (lobbyNoteCount === 1 ? "1 note" : lobbyNoteCount + " notes") + " on this device."
                                    color: "#555555"
                                    font.pointSize: 11
                                    font.family: "Noto Sans"
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                }
                                Text {
                                    text: "Open the Files tab (1) or press Ctrl-K.\nUse Tab / arrows / 1-6 to switch pages."
                                    color: "#888888"
                                    font.pointSize: 10
                                    font.family: "Noto Sans"
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                }
                                Text {
                                    text: "Open sourced at github.com/bjornte/Writerdeck-for-reMarkable"
                                    color: "#888888"
                                    font.pointSize: 9
                                    font.family: "Noto Mono"
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }
                    }
                    // 1 Files
                    Item {
                        visible: lobbyPage === 0
                        anchors.fill: parent

                        Column {
                            anchors.fill: parent
                            spacing: lobby.contentSpacing

                            Text {
                                text: lobbyFilesMode === "new" ? "New note name:"
                                     : lobbyFilesMode === "rename" ? "Rename to:"
                                     : lobbyFilesMode === "confirm-delete" ? "Delete this note? Enter=yes  Esc=no"
                                     : "Notes"
                                font.family: "Noto Sans"
                                font.pointSize: 13
                                color: "black"
                                width: parent.width
                            }

                            Text {
                                visible: lobbyFilesMode === "new" || lobbyFilesMode === "rename"
                                text: lobbyFilesInputDisplay()
                                font.family: "Noto Mono"
                                font.pointSize: 12
                                color: "#333"
                                width: parent.width
                            }

                            Text {
                                visible: lobbyVaultError !== "" && lobbyFilesMode === ""
                                text: lobbyVaultError
                                font.family: "Noto Sans"
                                font.pointSize: 11
                                color: "#aa0000"
                                width: parent.width
                                wrapMode: Text.WordWrap
                            }

                            ListView {
                                id: lobbyFilesList
                                width: parent.width
                                height: {
                                    var reserved = lobbyFilesBar.height + lobby.contentSpacing * 2
                                    if (lobbyEncryptionEnabled && lobbyFilesMode === "" && lobbyNotesModel.count > 0)
                                        reserved += lobbyFilesVaultBar.height + lobby.contentSpacing
                                    return Math.max(lobby.rowHeight * 2, parent.height - reserved)
                                }
                                clip: true
                                model: lobbyNotesModel
                                currentIndex: lobbyFilesIndex
                                spacing: 4
                                visible: lobbyFilesMode === "" || lobbyFilesMode === "confirm-delete"
                                delegate: Rectangle {
                                    width: lobbyFilesList.width
                                    height: lobby.rowHeight
                                    color: index === lobbyFilesIndex ? "#e8e8e8" : "white"
                                    border.color: "#ddd"
                                    border.width: 1
                                    radius: 4
                                    Text {
                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.left: parent.left
                                        anchors.leftMargin: 16
                                        text: lobbyFilesStripSuffix(model.name) + (model.encrypted ? " [private]" : "")
                                        font.family: "Noto Mono"
                                        font.pointSize: 11
                                        color: "black"
                                        elide: Text.ElideRight
                                        width: parent.width - 32
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            if (index === lobbyFilesIndex)
                                                root.lobbyOpenSelected()
                                            else {
                                                lobbyFilesIndex = index
                                                lobbyFilesList.currentIndex = index
                                            }
                                        }
                                        onDoubleClicked: root.lobbyOpenSelected()
                                    }
                                }
                                onCurrentIndexChanged: lobbyFilesIndex = currentIndex
                                highlight: Rectangle { color: "#d0d0d0"; radius: 4 }
                            }

                            Row {
                                id: lobbyFilesBar
                                width: parent.width
                                height: lobby.actionBtnHeight + 8
                                spacing: lobby.tabSpacing
                                visible: lobbyFilesMode === ""

                                Repeater {
                                    model: ["New", "Edit", "Read", "Rename", "Delete"]
                                    delegate: Rectangle {
                                        width: (lobbyFilesBar.width - lobby.tabSpacing * 4) / 5
                                        height: lobby.actionBtnHeight
                                        radius: 6
                                        color: "#f0f0f0"
                                        border.color: "#bbb"
                                        border.width: 1
                                        Text {
                                            anchors.centerIn: parent
                                            text: modelData
                                            font.family: "Noto Sans"
                                            font.pointSize: 11
                                        }
                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                if (modelData === "New") root.lobbyFilesBeginNew()
                                                else if (modelData === "Edit") root.lobbyOpenSelected()
                                                else if (modelData === "Read") root.lobbyReadSelected()
                                                else if (modelData === "Rename") root.lobbyFilesBeginRename()
                                                else if (modelData === "Delete") root.lobbyFilesBeginDelete()
                                            }
                                        }
                                    }
                                }
                            }

                            Row {
                                id: lobbyFilesVaultBar
                                width: parent.width
                                height: lobby.actionBtnHeight + 4
                                spacing: lobby.tabSpacing
                                visible: lobbyFilesMode === "" && lobbyEncryptionEnabled && lobbyNotesModel.count > 0

                                property bool selectedEncrypted: {
                                    if (lobbyNotesModel.count === 0) return false
                                    var row = lobbyNotesModel.get(lobbyFilesIndex)
                                    return row && row.encrypted
                                }

                                Rectangle {
                                    visible: !lobbyFilesVaultBar.selectedEncrypted
                                    width: (lobbyFilesVaultBar.width - lobby.tabSpacing) / 2
                                    height: lobby.actionBtnHeight
                                    radius: 6
                                    color: "#f0f0f0"
                                    border.color: "#bbb"
                                    border.width: 1
                                    Text {
                                        anchors.centerIn: parent
                                        text: "Encrypt"
                                        font.family: "Noto Sans"
                                        font.pointSize: 11
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: root.lobbyEncryptSelected()
                                    }
                                }
                                Rectangle {
                                    visible: !lobbyFilesVaultBar.selectedEncrypted
                                    width: (lobbyFilesVaultBar.width - lobby.tabSpacing) / 2
                                    height: lobby.actionBtnHeight
                                    radius: 6
                                    color: "#f0f0f0"
                                    border.color: "#bbb"
                                    border.width: 1
                                    Text {
                                        anchors.centerIn: parent
                                        text: "New encrypted"
                                        font.family: "Noto Sans"
                                        font.pointSize: 11
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: root.lobbyFilesBeginNewEncrypted()
                                    }
                                }
                                Rectangle {
                                    visible: lobbyFilesVaultBar.selectedEncrypted
                                    width: lobbyFilesVaultBar.width
                                    height: lobby.actionBtnHeight
                                    radius: 6
                                    color: "#f0f0f0"
                                    border.color: "#bbb"
                                    border.width: 1
                                    Text {
                                        anchors.centerIn: parent
                                        text: "Decrypt"
                                        font.family: "Noto Sans"
                                        font.pointSize: 11
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: root.lobbyDecryptSelected()
                                    }
                                }
                            }
                        }
                    }
                    // 2 Keyboard
                    Item {
                        visible: lobbyPage === 1
                        anchors.fill: parent
                        Flickable {
                            anchors.fill: parent
                            contentWidth: width
                            contentHeight: kbCol.height
                            clip: true
                            Column {
                                id: kbCol
                                width: parent.width
                                spacing: lobby.contentSpacing
                                Text {
                                    text: "USB keyboard"
                                    font.pointSize: 14
                                    font.family: "Noto Sans"
                                    color: "black"
                                    width: parent.width
                                }
                                Text {
                                    text: "Connect with a USB OTG cable.\nChanging layout restarts Writerdeck."
                                    font.pointSize: 11
                                    font.family: "Noto Sans"
                                    color: "#555555"
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                }
                                Text {
                                    text: "Layout"
                                    font.pointSize: 12
                                    font.family: "Noto Sans"
                                    color: "black"
                                    width: parent.width
                                }
                                Row {
                                    id: kbLayoutRow
                                    width: parent.width
                                    spacing: lobby.tabSpacing
                                    property var layoutOptions: [
                                        { id: "us", label: "US QWERTY" },
                                        { id: "no", label: "Norwegian" }
                                    ]
                                    Repeater {
                                        model: kbLayoutRow.layoutOptions
                                        delegate: Rectangle {
                                            width: (kbLayoutRow.width - lobby.tabSpacing * (kbLayoutRow.layoutOptions.length - 1)) / kbLayoutRow.layoutOptions.length
                                            height: lobby.actionBtnHeight
                                            radius: 6
                                            property bool selected: lobbyKeyboardLayout === modelData.id
                                            color: selected ? "#e8e8e8" : "#f0f0f0"
                                            border.color: selected ? "black" : "#bbb"
                                            border.width: selected ? 2 : 1
                                            Text {
                                                anchors.centerIn: parent
                                                text: modelData.label
                                                font.family: "Noto Sans"
                                                font.pointSize: 11
                                            }
                                            MouseArea {
                                                anchors.fill: parent
                                                onClicked: writerdeck.setKeyboardLayout(modelData.id)
                                            }
                                        }
                                    }
                                }
                                Text {
                                    text: "Bluetooth / phone"
                                    font.pointSize: 14
                                    font.family: "Noto Sans"
                                    color: "black"
                                    width: parent.width
                                }
                                Text {
                                    text: "Pair the keyboard to your phone, then open:\nhttp://" + lobbyIP + ":8000\nTyping is forwarded over Wi-Fi.\n" + (lobbyPIN !== "" ? ("PIN: " + lobbyPIN) : "PIN is not set")
                                    font.pointSize: 11
                                    font.family: "Noto Sans"
                                    color: "#555555"
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }
                    }
                    // 3 Sync
                    Item {
                        visible: lobbyPage === 2
                        anchors.fill: parent
                        Flickable {
                            anchors.fill: parent
                            contentWidth: width
                            contentHeight: syncCol.height
                            clip: true
                            Column {
                                id: syncCol
                                width: parent.width
                                spacing: lobby.contentSpacing
                                Text {
                                    text: "GitHub sync"
                                    font.pointSize: 14
                                    font.family: "Noto Sans"
                                    color: "black"
                                    width: parent.width
                                }
                                Text {
                                    visible: !(lobbySyncOn && lobbySyncRepo !== "" && !lobbySyncReady)
                                        && !(lobbySyncOn && lobbySyncRepo !== "" && lobbySyncReady && lobbySyncError !== "")
                                    text: lobbySyncOn && lobbySyncRepo !== ""
                                        ? (lobbyLastSync !== ""
                                            ? "Last sync was " + lobbyLastSync + ".\nNotes sync to github.com/" + lobbySyncRepo
                                            : "Notes sync to github.com/" + lobbySyncRepo)
                                        : ("Sync not configured.\nSet up in phone Sync setup:\nhttp://" + lobbyIP + ":8000")
                                    font.pointSize: 11
                                    font.family: "Noto Sans"
                                    color: lobbySyncOn && lobbySyncRepo !== "" ? "#1b5e20" : "#555555"
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                }
                                Rectangle {
                                    visible: lobbySyncOn && lobbySyncRepo !== "" && lobbySyncReady && lobbySyncError !== ""
                                    width: parent.width
                                    height: syncErrCol.height + 20
                                    color: "white"
                                    border.color: "black"
                                    border.width: 2
                                    radius: 4
                                    Column {
                                        id: syncErrCol
                                        anchors.centerIn: parent
                                        width: parent.width - 20
                                        spacing: 10
                                        Text {
                                            text: !lobbyWifi ? "SYNC OFFLINE" : "SYNC FAILED"
                                            font.pointSize: 16
                                            font.bold: true
                                            font.family: "Noto Sans"
                                            color: "black"
                                            width: parent.width
                                        }
                                        Text {
                                            text: lobbySyncError
                                            font.pointSize: 13
                                            font.family: "Noto Sans"
                                            color: "black"
                                            width: parent.width
                                            wrapMode: Text.WordWrap
                                            lineHeight: 1.25
                                        }
                                    }
                                }
                                Rectangle {
                                    visible: lobbySyncOn && lobbySyncRepo !== "" && !lobbySyncReady
                                    width: parent.width
                                    height: tokenWarnCol.height + 20
                                    color: "white"
                                    border.color: "black"
                                    border.width: 2
                                    radius: 4
                                    Column {
                                        id: tokenWarnCol
                                        anchors.centerIn: parent
                                        width: parent.width - 20
                                        spacing: 10
                                        Text {
                                            text: "TOKEN NEEDED"
                                            font.pointSize: 16
                                            font.bold: true
                                            font.family: "Noto Sans"
                                            color: "black"
                                            width: parent.width
                                        }
                                        Text {
                                            text: "GitHub token is not on the tablet.\nOpen phone Sync setup and tap Save:\nhttp://" + lobbyIP + ":8000\nRepo: github.com/" + lobbySyncRepo
                                            font.pointSize: 13
                                            font.family: "Noto Sans"
                                            color: "black"
                                            width: parent.width
                                            wrapMode: Text.WordWrap
                                            lineHeight: 1.25
                                        }
                                    }
                                }
                                Rectangle {
                                    visible: lobbySyncOn && lobbySyncRepo !== ""
                                    width: parent.width
                                    height: lobby.actionBtnHeight
                                    radius: 6
                                    color: (lobbySyncReady && !lobbySyncing) ? "#f0f0f0" : "white"
                                    border.color: lobbySyncReady ? "#bbb" : "black"
                                    border.width: lobbySyncReady ? 1 : 2
                                    Text {
                                        anchors.centerIn: parent
                                        text: !lobbySyncReady ? "Token needed — phone Sync setup"
                                            : (lobbySyncing ? "Syncing…" : "Sync now")
                                        font.family: "Noto Sans"
                                        font.pointSize: !lobbySyncReady ? 14 : 12
                                        font.bold: !lobbySyncReady
                                        color: (lobbySyncReady && !lobbySyncing) ? "black" : (lobbySyncReady ? "#888888" : "black")
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        enabled: lobbySyncReady && !lobbySyncing
                                        onClicked: writerdeck.syncNow()
                                    }
                                }
                                Text {
                                    text: "Sync also runs automatically on save, Home, and every few minutes."
                                    font.pointSize: 10
                                    font.family: "Noto Sans"
                                    color: "#888888"
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }
                    }
                    // 4 Settings
                    Item {
                        visible: lobbyPage === 3
                        anchors.fill: parent
                        Flickable {
                            anchors.fill: parent
                            contentWidth: width
                            contentHeight: setCol.height
                            clip: true
                            Column {
                                id: setCol
                                width: parent.width
                                spacing: lobby.contentSpacing
                                Text {
                                    text: lobbySettingsMode === "confirm-exit"
                                          ? "Stop Writerdeck? Enter=yes  Esc=no"
                                          : "Settings"
                                    font.pointSize: 14
                                    font.family: "Noto Sans"
                                    color: "black"
                                    width: parent.width
                                }

                                Column {
                                    width: parent.width
                                    spacing: lobby.contentSpacing
                                    visible: lobbySettingsMode === ""

                                    Text {
                                        text: "\nReading font"
                                        font.pointSize: 12
                                        font.family: "Noto Sans"
                                        color: "black"
                                        width: parent.width
                                    }
                                    Grid {
                                        id: fontGrid
                                        width: parent.width
                                        columns: 2
                                        rowSpacing: lobby.tabSpacing
                                        columnSpacing: lobby.tabSpacing
                                        property var fontOptions: [
                                            { id: "Inter", label: "Inter" },
                                            { id: "Literata", label: "Literata" },
                                            { id: "EB Garamond", label: "EB Garamond" },
                                            { id: "DejaVu Sans", label: "DejaVu Sans" }
                                        ]
                                        Repeater {
                                            model: fontGrid.fontOptions
                                            delegate: Rectangle {
                                                width: (fontGrid.width - lobby.tabSpacing) / 2
                                                height: lobby.actionBtnHeight
                                                radius: 6
                                                property bool selected: readFont === modelData.id
                                                color: selected ? "#e8e8e8" : "#f0f0f0"
                                                border.color: selected ? "black" : "#bbb"
                                                border.width: selected ? 2 : 1
                                                Text {
                                                    anchors.centerIn: parent
                                                    text: modelData.label
                                                    font.family: "Noto Sans"
                                                    font.pointSize: 11
                                                }
                                                MouseArea {
                                                    anchors.fill: parent
                                                    onClicked: writerdeck.setReadFont(modelData.id)
                                                }
                                            }
                                        }
                                    }

                                    Text {
                                        text: "\nPrivate notes"
                                        font.pointSize: 12
                                        font.family: "Noto Sans"
                                        color: "black"
                                        width: parent.width
                                    }
                                    Text {
                                        text: lobbyEncryptionEnabled
                                            ? "On — encrypted notes require PIN to open, read, or edit"
                                            : "Off — optional encryption with a separate 6-digit PIN. Recovery via GitHub secret/pin when sync is on."
                                        font.pointSize: 10
                                        font.family: "Noto Sans"
                                        color: "#666666"
                                        width: parent.width
                                        wrapMode: Text.WordWrap
                                    }
                                    Row {
                                        width: parent.width
                                        spacing: lobby.tabSpacing
                                        visible: !lobbyEncryptionEnabled
                                        Rectangle {
                                            width: (parent.width - lobby.tabSpacing) / 2
                                            height: lobby.actionBtnHeight
                                            radius: 6
                                            color: "#f0f0f0"
                                            border.color: "#bbb"
                                            border.width: 1
                                            Text {
                                                anchors.centerIn: parent
                                                text: "Enable"
                                                font.family: "Noto Sans"
                                                font.pointSize: 11
                                            }
                                            MouseArea {
                                                anchors.fill: parent
                                                onClicked: root.vaultBeginSetup()
                                            }
                                        }
                                    }
                                    Row {
                                        width: parent.width
                                        spacing: lobby.tabSpacing
                                        visible: lobbyEncryptionEnabled
                                        Rectangle {
                                            width: parent.width
                                            height: lobby.actionBtnHeight
                                            radius: 6
                                            color: "#f0f0f0"
                                            border.color: "#bbb"
                                            border.width: 1
                                            Text {
                                                anchors.centerIn: parent
                                                text: "Change PIN"
                                                font.family: "Noto Sans"
                                                font.pointSize: 11
                                            }
                                            MouseArea {
                                                anchors.fill: parent
                                                onClicked: root.vaultBeginChangePIN()
                                            }
                                        }
                                    }

                                    Text {
                                        text: "\nPIN for phone pairing"
                                        font.pointSize: 12
                                        font.family: "Noto Sans"
                                        color: "black"
                                        width: parent.width
                                    }
                                    Text {
                                        text: "Adding a PIN ensures that only intended devices can access your notes"
                                        font.pointSize: 10
                                        font.family: "Noto Sans"
                                        color: "#666666"
                                        width: parent.width
                                        wrapMode: Text.WordWrap
                                    }
                                    Column {
                                        width: parent.width
                                        spacing: lobby.tabSpacing
                                        Repeater {
                                            model: [
                                                { id: "6", label: "6 digits" },
                                                { id: "4", label: "4 digits" },
                                                { id: "none", label: "No PIN", warn: "Anyone on Wi-Fi can read and edit notes" }
                                            ]
                                            delegate: Rectangle {
                                                width: parent.width
                                                height: modelData.warn ? lobby.actionBtnHeight + 28 : lobby.actionBtnHeight
                                                radius: 6
                                                property bool selected: lobbyPinDigits === modelData.id
                                                color: selected ? "#e8e8e8" : "#f0f0f0"
                                                border.color: selected ? "black" : "#bbb"
                                                border.width: selected ? 2 : 1
                                                Column {
                                                    anchors.centerIn: parent
                                                    width: parent.width - 16
                                                    spacing: 2
                                                    Text {
                                                        anchors.horizontalCenter: parent.horizontalCenter
                                                        text: modelData.label
                                                        font.family: "Noto Sans"
                                                        font.pointSize: 11
                                                        color: "black"
                                                    }
                                                    Text {
                                                        visible: !!modelData.warn
                                                        anchors.horizontalCenter: parent.horizontalCenter
                                                        text: modelData.warn || ""
                                                        font.family: "Noto Sans"
                                                        font.pointSize: 9
                                                        color: "#666666"
                                                        horizontalAlignment: Text.AlignHCenter
                                                        wrapMode: Text.WordWrap
                                                        width: parent.width
                                                    }
                                                }
                                                MouseArea {
                                                    anchors.fill: parent
                                                    onClicked: writerdeck.setPinDigits(modelData.id)
                                                }
                                            }
                                        }
                                    }

                                    Text {
                                        text: "\nDisplay rotation"
                                        font.pointSize: 12
                                        font.family: "Noto Sans"
                                        color: "black"
                                        width: parent.width
                                    }
                                    Text {
                                        text: root.rotation + " degrees. Ctrl-R or Ctrl+arrows to rotate."
                                        font.pointSize: 10
                                        font.family: "Noto Sans"
                                        color: "#666666"
                                        width: parent.width
                                        wrapMode: Text.WordWrap
                                    }
                                    Rectangle {
                                        width: parent.width
                                        height: lobby.actionBtnHeight
                                        radius: 6
                                        color: "#f0f0f0"
                                        border.color: "#bbb"
                                        border.width: 1
                                        Text {
                                            anchors.centerIn: parent
                                            text: "Rotate 90"
                                            font.family: "Noto Sans"
                                            font.pointSize: 12
                                        }
                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: root.rotateScreen()
                                        }
                                    }

                                    Text {
                                        text: "\nService"
                                        font.pointSize: 12
                                        font.family: "Noto Sans"
                                        color: "black"
                                        width: parent.width
                                    }
                                    Text {
                                        text: "Stop Writerdeck and return the tablet to the stock reMarkable UI. Reconnect later via SSH or reboot."
                                        font.pointSize: 10
                                        font.family: "Noto Sans"
                                        color: "#666666"
                                        width: parent.width
                                        wrapMode: Text.WordWrap
                                    }
                                    Rectangle {
                                        width: parent.width
                                        height: lobby.actionBtnHeight
                                        radius: 6
                                        color: "#f0f0f0"
                                        border.color: "#bbb"
                                        border.width: 1
                                        Text {
                                            anchors.centerIn: parent
                                            text: "Exit Writerdeck"
                                            font.family: "Noto Sans"
                                            font.pointSize: 12
                                        }
                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: root.lobbySettingsBeginExit()
                                        }
                                    }
                                }
                            }
                        }
                    }
                    // 5 Shortcuts
                    Item {
                        visible: lobbyPage === 4
                        anchors.fill: parent
                        Flickable {
                            anchors.fill: parent
                            contentWidth: width
                            contentHeight: scCol.height
                            clip: true
                            Column {
                                id: scCol
                                width: parent.width
                                spacing: lobby.contentSpacing
                                Text {
                                    text: "Shortcuts"
                                    font.pointSize: 14
                                    font.family: "Noto Sans"
                                    color: "black"
                                    width: parent.width
                                }
                                Text {
                                    text: "Lobby: Tab / arrows / 1-6 switch pages\nFiles: Up/Down select  Enter edit  v read  n d r\nStock UI: Esc (USB) or L+R page buttons → Lobby\nCtrl-K: quick file picker\nCtrl-C/X/V: copy cut paste\nCtrl-R: rotate  Ctrl-Q: quit\nHome: exit to reMarkable UI"
                                    font.pointSize: 10
                                    font.family: "Noto Mono"
                                    color: "#555555"
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }
                    }
                }
                Text {
                    id: lobbyHint
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: lobby.pageMargin
                    anchors.rightMargin: lobby.pageMargin
                    anchors.bottomMargin: lobby.pageMargin
                    height: 48
                    verticalAlignment: Text.AlignVCenter
                    font.family: "Noto Mono"
                    font.pointSize: 9
                    color: "#888888"
                    text: lobbyPage === 0 && lobbyFilesMode === ""
                        ? "n new  Enter edit  v read  r rename  d delete"
                        : "Tab next page  1-6 jump  Ctrl-K files"
                }
            }
        }
        Rectangle {
            id: vaultOverlay
            anchors.fill: parent
            color: "#f8f8f8"
            visible: vaultOverlayMode !== ""
            z: 25

            Column {
                anchors.centerIn: parent
                width: parent.width * 0.7
                spacing: lobby.contentSpacing

                Text {
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    font.family: "Noto Sans"
                    font.pointSize: 14
                    color: "black"
                    text: vaultOverlayMode === "setup" ? "Choose a 6-digit private PIN"
                        : vaultOverlayMode === "confirm" ? "Confirm private PIN"
                        : vaultOverlayMode === "change-old" ? "Enter current private PIN"
                        : vaultOverlayMode === "change-new" ? "Enter new private PIN"
                        : vaultOverlayMode === "change-confirm" ? "Confirm new private PIN"
                        : vaultOverlayReason !== "" ? vaultOverlayReason
                        : "Enter private PIN"
                }

                Text {
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    font.family: "Noto Mono"
                    font.pointSize: 18
                    color: "#333"
                    text: vaultPinDisplay()
                }

                Grid {
                    id: vaultPad
                    width: parent.width
                    columns: 3
                    rowSpacing: 8
                    columnSpacing: 8
                    Repeater {
                        model: ["1","2","3","4","5","6","7","8","9","Bksp","0","Done"]
                        delegate: Rectangle {
                            width: (vaultPad.width - 16) / 3
                            height: lobby.actionBtnHeight
                            radius: 6
                            color: "#f0f0f0"
                            border.color: "#bbb"
                            border.width: 1
                            Text {
                                anchors.centerIn: parent
                                text: modelData
                                font.family: "Noto Sans"
                                font.pointSize: 12
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: vaultNumpadTap(modelData)
                            }
                        }
                    }
                }

                Rectangle {
                    width: parent.width
                    height: lobby.actionBtnHeight
                    radius: 6
                    color: "#f0f0f0"
                    border.color: "#bbb"
                    border.width: 1
                    Text {
                        anchors.centerIn: parent
                        text: "Cancel"
                        font.family: "Noto Sans"
                        font.pointSize: 12
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: vaultNumpadCancel()
                    }
                }
            }
        }
        Rectangle {
            id: sleepScreen
            anchors.fill: parent
            color: "white"
            visible: isSleeping
            z: 10
            Column {
                anchors.centerIn: parent
                width: sleepScreen.width * 0.75
                spacing: 24
                Text {
                    text: "Writerdeck is sleeping.\nWi-Fi is off. Press power to wake."
                    color: "black"
                    font.pointSize: 18
                    font.family: "Noto Sans"
                    width: parent.width
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }
    }
}
