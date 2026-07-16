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
                contentY -= 1500
                if (contentY < 0) contentY = 0
            }
            function scrollDown() {
                contentY += 1500
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
    }
}
