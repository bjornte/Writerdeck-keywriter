import QtQuick 2.11
import QtQuick.Window 2.2
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
    property bool isLobby: true
    property bool isSleeping: false
    property string lobbyIP: ""
    property string lobbyPIN: ""
    property int lobbyPort: 8000
    property bool lobbyPhoneConnected: false
    property bool lobbyUsbKeyboard: false
    property string lobbyQrPath: ""
    property bool lobbyShowNoKeyboard: false
    property string lobbyNoKeyboardPending: ""
    property int paraSpacing: 28
    property string readFont: "Inter"
    property bool lobbySyncOn: false
    property string lobbySyncRepo: ""
    property int lobbyNoteCount: 0
    property string lobbyLastSync: ""
    property int lobbyLastSyncAt: 0
    property int lobbySyncPending: 0
    property bool lobbySyncReady: false
    property bool lobbySyncing: false
    property string lobbySyncError: ""
    property bool lobbyWifi: true
    property string lobbyKeyboardLayout: "us"
    property string lobbyPinDigits: "6"
    property string lobbySettingsMode: ""
    property int lobbySettingsPickIndex: 0
    // Font pangram shown in the Settings font picker (fixed English sample).
    readonly property string lobbyFontSample: "the quick brown fox jumps over the lazy dog"
    property int lobbyPage: 0
    property var lobbyTabShortcutIds: ["tabs.files", "tabs.keyboard", "tabs.sync", "tabs.settings", "tabs.shortcuts", "tabs.about"]
    // Depends on lobbyUi.revision so language / pack reloads refresh tab titles.
    property var lobbyTabLabels: {
        return [
            root.lobbyT("tabs.files"),
            root.lobbyT("tabs.keyboard"),
            root.lobbyT("tabs.sync"),
            root.lobbyT("tabs.settings"),
            root.lobbyT("tabs.shortcuts"),
            root.lobbyT("tabs.about")
        ]
    }

    // String helpers that rebind when lobby-ui language/packs reload.
    function lobbyT(key) {
        var _ = lobbyUi.revision
        return lobbyUi.str(key)
    }
    function lobbyTf(key, a1, a2, a3) {
        var _ = lobbyUi.revision
        if (a3 !== undefined && a3 !== null && a3 !== "")
            return lobbyUi.strf(key, a1, a2, a3)
        if (a2 !== undefined && a2 !== null && a2 !== "")
            return lobbyUi.strf(key, a1, a2)
        return lobbyUi.strf(key, a1)
    }
    property string lobbyVersionText: ""
    property int lobbyFilesIndex: 0
    // How many note rows fit on one Files page (set from list height; e-ink pages, no flick).
    // Page-aligned window: decisions.md §35.
    property int lobbyFilesPageSize: 6
    // After New / New encrypted, open this file once it appears in the notes list.
    property string lobbyOpenAfterCreate: ""
    property string lobbyLastEditedFile: ""
    property string lobbyFilesMode: ""
    onLobbyFilesModeChanged: {
        if (vaultOverlayMode === "")
            writerdeck.notifyLobbyInput(lobbyFilesMode)
        if (lobbyFilesMode === "")
            lobbyFilesInputError = ""
    }
    property string lobbyFilesInput: ""
    property int lobbyFilesInputPos: 0
    // Shown inside New / Rename / New-encrypted dialog (not the Files header box).
    property string lobbyFilesInputError: ""
    // Set while a create/rename request is in flight so vaultopfailed can reopen the dialog.
    property string lobbyFilesPendingMode: ""
    property string lobbyFilesPendingInput: ""
    property bool lobbyOpenInReadMode: false
    property bool lobbyEncryptionEnabled: false
    property string lobbyVaultError: ""
    property string vaultOverlayMode: ""
    onVaultOverlayModeChanged: {
        if (vaultOverlayMode !== "")
            writerdeck.notifyLobbyInput("pin")
        else
            writerdeck.notifyLobbyInput(lobbyFilesMode)
        lobbyKeepFocus()
    }
    property string vaultOverlayReason: ""
    property string vaultPinInput: ""
    property string vaultPinPending: ""
    property bool vaultPinKeepSession: false
    property string vaultPendingLoad: ""
    property string vaultPendingAction: ""
    property string vaultPendingNote: ""
    property string currentFile: ""
    property string folder: "file://%1/Writerdeck-user-documents/".arg(home_dir)

    function lobbyKeepFocus() {
        if (!isLobby) return
        Qt.callLater(function() {
            if (isLobby && typeof lobbyFocus !== "undefined" && lobbyFocus)
                lobbyFocus.forceActiveFocus()
        })
    }

    function lobbyFilesPageCount() {
        var ps = Math.max(1, lobbyFilesPageSize)
        if (lobbyNotesModel.count <= 0) return 1
        return Math.ceil(lobbyNotesModel.count / ps)
    }

    function lobbyFilesPageIndex() {
        var ps = Math.max(1, lobbyFilesPageSize)
        if (lobbyFilesIndex < 0) return 0
        return Math.floor(lobbyFilesIndex / ps)
    }

    function lobbyFilesPageStart() {
        return lobbyFilesPageIndex() * Math.max(1, lobbyFilesPageSize)
    }

    function lobbyFilesSetIndex(i) {
        var last = Math.max(0, lobbyNotesModel.count - 1)
        lobbyFilesIndex = Math.max(0, Math.min(last, i))
    }

    function lobbyFilesNormalizedName(name, encrypted) {
        name = (name || "").trim()
        if (name === "") return ""
        if (encrypted) {
            if (name.endsWith(".md.enc")) return name
            if (name.endsWith(".md")) return name + ".enc"
            return name + ".md.enc"
        }
        if (name.endsWith(".md") || name.endsWith(".md.enc")) return name
        return name + ".md"
    }

    // Case-insensitive stem; plain and encrypted share a key ("Doc.md" / "doc.md.enc" → "doc").
    function lobbyFilesTitleKey(name) {
        name = name || ""
        if (name.endsWith(".md.enc")) name = name.slice(0, -7)
        else if (name.endsWith(".md")) name = name.slice(0, -3)
        return name.toLowerCase()
    }

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
                    vaultBeginPIN(root.lobbyT("vault.open"), true)
                    return
                }
                if (xhr.status !== 200) {
                    var errMsg = root.lobbyT("files.openFailed")
                    if (xhr.status === 500 && name.indexOf(".md.enc") >= 0)
                        errMsg = root.lobbyT("files.decryptCorrupt")
                    vaultOpFailed(errMsg)
                    return
                }
                var response = sanitizeLoadedNote(xhr.responseText)
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

    function saveAndLoad(name) {
        if (name && name.indexOf(".md.enc") >= 0 && currentFile !== name) {
            vaultPendingLoad = name
            vaultBeginPIN(root.lobbyT("vault.edit"), true)
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

    function setLobbySyncClock(lastSyncAt, syncPending) {
        lobbyLastSyncAt = lastSyncAt || 0
        lobbySyncPending = syncPending || 0
    }

    // Relative sync time for Lobby (server still sends English lastSync for logs/compat).
    function lobbyFormatSyncAgo(unix) {
        if (!unix || unix <= 0) return ""
        var sec = Math.floor(Date.now() / 1000) - unix
        if (sec < 0) sec = 0
        if (sec < 60) return root.lobbyT("sync.agoJustNow")
        if (sec < 3600) {
            var m = Math.floor(sec / 60)
            return m === 1 ? root.lobbyT("sync.agoMinute") : root.lobbyTf("sync.agoMinutes", "" + m)
        }
        if (sec < 86400) {
            var h = Math.floor(sec / 3600)
            return h === 1 ? root.lobbyT("sync.agoHour") : root.lobbyTf("sync.agoHours", "" + h)
        }
        var d = Math.floor(sec / 86400)
        return d === 1 ? root.lobbyT("sync.agoDay") : root.lobbyTf("sync.agoDays", "" + d)
    }

    function lobbyLastSyncLabel() {
        if (lobbySyncOn && lobbySyncRepo !== "" && !lobbySyncReady)
            return ""
        var ago = lobbyFormatSyncAgo(lobbyLastSyncAt)
        if (lobbySyncPending > 0) {
            var pend = lobbySyncPending === 1
                ? root.lobbyT("sync.pendingOne")
                : root.lobbyTf("sync.pendingMany", "" + lobbySyncPending)
            if (ago === "") return pend
            return root.lobbyTf("sync.agoWithPending", ago, pend)
        }
        return ago
    }

    function setLobbyKeyboardPresence(phoneConnected, usbKeyboard, port, qrPath) {
        lobbyPhoneConnected = !!phoneConnected
        lobbyUsbKeyboard = !!usbKeyboard
        lobbyPort = port || 8000
        lobbyQrPath = qrPath || ""
        if (lobbyShowNoKeyboard && lobbyKeyboardReady())
            lobbyContinueAfterKeyboard()
    }

    function lobbyPhoneUrl() {
        if (lobbyIP === "") return "(no Wi-Fi address yet)"
        return "http://" + lobbyIP + ":" + lobbyPort
    }

    // USB keyboard or an open phone/laptop page (WebSocket). Touch actions show
    // the tip only when neither is present. Key chords skip the tip (fromKey).
    // Continue / a key while the tip is up uses fromKey once — never a sticky flag.
    function lobbyKeyboardReady() {
        return lobbyUsbKeyboard || lobbyPhoneConnected
    }

    function lobbyDialogIsOpen() {
        if (lobbyShowNoKeyboard) return true
        if (lobbySettingsMode === "confirm-exit"
                || lobbySettingsMode === "pick-language"
                || lobbySettingsMode === "pick-font"
                || lobbySettingsMode === "pick-pin")
            return true
        return lobbyFilesMode === "confirm-delete"
            || lobbyFilesMode === "new"
            || lobbyFilesMode === "rename"
            || lobbyFilesMode === "new-encrypted"
    }

    function lobbyDialogTitle() {
        if (lobbyShowNoKeyboard) return root.lobbyT("dialog.connectTitle")
        if (lobbyFilesMode === "confirm-delete") return root.lobbyT("dialog.deleteTitle")
        if (lobbyFilesMode === "new") return root.lobbyT("dialog.newTitle")
        if (lobbyFilesMode === "rename") return root.lobbyT("dialog.renameTitle")
        if (lobbyFilesMode === "new-encrypted") return root.lobbyT("dialog.newEncryptedTitle")
        if (lobbySettingsMode === "confirm-exit") return root.lobbyT("dialog.exitTitle")
        if (lobbySettingsMode === "pick-language") return root.lobbyT("dialog.languageTitle")
        if (lobbySettingsMode === "pick-font") return root.lobbyT("dialog.fontTitle")
        if (lobbySettingsMode === "pick-pin") return root.lobbyT("dialog.pinTitle")
        return ""
    }

    function lobbySettingsLanguageOptions() {
        return [
            { id: "en", label: "English" },
            { id: "no", label: "Norsk" },
            { id: "es", label: "Espanol" },
            { id: "de", label: "Deutsch" },
            { id: "fr", label: "Francais" }
        ]
    }

    function lobbySettingsFontOptions() {
        return [
            { id: "Inter", label: "Inter" },
            { id: "Literata", label: "Literata" },
            { id: "EB Garamond", label: "EB Garamond" },
            { id: "DejaVu Sans", label: "DejaVu Sans" }
        ]
    }

    function lobbySettingsPinOptions() {
        return [
            { id: "6", labelKey: "settings.pin6" },
            { id: "4", labelKey: "settings.pin4" },
            { id: "none", labelKey: "settings.pinNone", warnKey: "settings.pinNoneWarn" }
        ]
    }

    function lobbySettingsLanguageLabel() {
        var opts = lobbySettingsLanguageOptions()
        for (var i = 0; i < opts.length; i++) {
            if (opts[i].id === lobbyUi.language)
                return opts[i].label
        }
        return lobbyUi.language
    }

    function lobbySettingsPinLabel() {
        if (lobbyPinDigits === "6") return root.lobbyT("settings.pin6")
        if (lobbyPinDigits === "4") return root.lobbyT("settings.pin4")
        if (lobbyPinDigits === "none") return root.lobbyT("settings.pinNone")
        return lobbyPinDigits
    }

    function lobbySettingsBeginPickLanguage() {
        var opts = lobbySettingsLanguageOptions()
        lobbySettingsPickIndex = 0
        for (var i = 0; i < opts.length; i++) {
            if (opts[i].id === lobbyUi.language) {
                lobbySettingsPickIndex = i
                break
            }
        }
        lobbySettingsMode = "pick-language"
    }

    function lobbySettingsBeginPickFont() {
        var opts = lobbySettingsFontOptions()
        lobbySettingsPickIndex = 0
        for (var i = 0; i < opts.length; i++) {
            if (opts[i].id === readFont) {
                lobbySettingsPickIndex = i
                break
            }
        }
        lobbySettingsMode = "pick-font"
    }

    function lobbySettingsBeginPickPin() {
        var opts = lobbySettingsPinOptions()
        lobbySettingsPickIndex = 0
        for (var i = 0; i < opts.length; i++) {
            if (opts[i].id === lobbyPinDigits) {
                lobbySettingsPickIndex = i
                break
            }
        }
        lobbySettingsMode = "pick-pin"
    }

    function lobbySettingsCancelPick() {
        lobbySettingsMode = ""
        lobbyKeepFocus()
    }

    function lobbySettingsApplyPick(index) {
        if (index === undefined || index < 0)
            index = lobbySettingsPickIndex
        if (lobbySettingsMode === "pick-language") {
            var langs = lobbySettingsLanguageOptions()
            if (index >= 0 && index < langs.length) {
                if (lobbyUi.setLanguage(langs[index].id)) {
                    writerdeck.notifyLanguageChanged()
                    lobbyRefreshVersion()
                }
            }
        } else if (lobbySettingsMode === "pick-font") {
            var fonts = lobbySettingsFontOptions()
            if (index >= 0 && index < fonts.length)
                writerdeck.setReadFont(fonts[index].id)
        } else if (lobbySettingsMode === "pick-pin") {
            var pins = lobbySettingsPinOptions()
            if (index >= 0 && index < pins.length)
                writerdeck.setPinDigits(pins[index].id)
        } else if (lobbySettingsMode === "confirm-exit") {
            lobbySettingsDoExit()
            return
        }
        lobbySettingsMode = ""
        lobbyKeepFocus()
    }

    function lobbySettingsPickCount() {
        if (lobbySettingsMode === "pick-language") return lobbySettingsLanguageOptions().length
        if (lobbySettingsMode === "pick-font") return lobbySettingsFontOptions().length
        if (lobbySettingsMode === "pick-pin") return lobbySettingsPinOptions().length
        return 0
    }

    function lobbyDialogSelectedNoteLabel() {
        if (lobbyNotesModel.count === 0) return ""
        if (lobbyFilesIndex < 0 || lobbyFilesIndex >= lobbyNotesModel.count) return ""
        var row = lobbyNotesModel.get(lobbyFilesIndex)
        return row ? lobbyFilesStripSuffix(row.name) : ""
    }

    function lobbyEnsureKeyboard(pending) {
        if (lobbyKeyboardReady())
            return true
        lobbyNoKeyboardPending = pending || ""
        lobbyShowNoKeyboard = true
        writerdeck.notifyLobbyInput("no-keyboard")
        lobbyKeepFocus()
        return false
    }

    function lobbyDismissNoKeyboard() {
        lobbyShowNoKeyboard = false
        lobbyNoKeyboardPending = ""
        if (vaultOverlayMode === "")
            writerdeck.notifyLobbyInput(lobbyFilesMode)
        lobbyKeepFocus()
    }

    function lobbyContinueAfterKeyboard() {
        var pending = lobbyNoKeyboardPending
        lobbyShowNoKeyboard = false
        lobbyNoKeyboardPending = ""
        if (vaultOverlayMode === "")
            writerdeck.notifyLobbyInput(lobbyFilesMode)
        // fromKey=true: one-shot past the tip for this action only.
        if (pending === "edit")
            lobbyOpenSelected(true)
        else if (pending === "new")
            lobbyFilesBeginNew(true)
        else if (pending === "rename")
            lobbyFilesBeginRename(true)
        else if (pending === "new-encrypted")
            lobbyFilesBeginNewEncrypted(true)
        lobbyKeepFocus()
    }

    function setEncryptionEnabled(enabled) {
        lobbyEncryptionEnabled = !!enabled
    }

    function vaultOpFailed(msg) {
        // Wrong PIN (or other verify failure) while encrypt/decrypt/open is pending:
        // keep the pad up with a clear message instead of dumping to Files.
        lobbyOpenAfterCreate = ""
        if (vaultPendingAction !== "" || vaultPendingLoad !== "") {
            vaultOverlayMode = "pin"
            vaultPinInput = ""
            vaultOverlayReason = msg || root.lobbyT("vault.wrongPin")
            lobbyKeepFocus()
            return
        }
        // Create/rename failures belong in the name dialog, not the Files header.
        if (lobbyFilesMode === "new" || lobbyFilesMode === "rename" || lobbyFilesMode === "new-encrypted") {
            lobbyFilesInputError = msg || root.lobbyT("files.operationFailed")
            lobbyKeepFocus()
            return
        }
        if (lobbyFilesPendingMode === "new" || lobbyFilesPendingMode === "rename"
                || lobbyFilesPendingMode === "new-encrypted") {
            var mode = lobbyFilesPendingMode
            var input = lobbyFilesPendingInput
            lobbyFilesPendingMode = ""
            lobbyFilesPendingInput = ""
            lobbyPage = 0
            lobbyFilesMode = mode
            lobbyFilesInput = input
            lobbyFilesInputPos = input.length
            lobbyFilesInputError = msg || root.lobbyT("files.operationFailed")
            lobbyKeepFocus()
            return
        }
        lobbyGoPage(0)
        lobbyVaultError = msg || root.lobbyT("files.operationFailed")
    }

    function vaultOnPINAccepted() {
        lobbyVaultError = ""
        vaultOverlayMode = ""
        vaultPinInput = ""
        vaultPinPending = ""
        vaultOverlayReason = ""
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
            lobbyFilesInputError = ""
            lobbyFilesPendingMode = ""
            lobbyFilesPendingInput = ""
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

    function setLobbySyncStatus(syncError, wifi, syncErrorKey) {
        if (syncErrorKey === "noWifi")
            lobbySyncError = root.lobbyT("sync.errNoWifi")
        else
            lobbySyncError = syncError || ""
        lobbyWifi = !!wifi
    }

    function lobbyGoPage(idx) {
        if (idx < 0 || idx >= lobbyTabLabels.length) return
        lobbyPage = idx
        lobbyFilesMode = ""
        lobbyFilesInput = ""
        lobbyFilesInputPos = 0
        lobbyFilesInputError = ""
        lobbyFilesPendingMode = ""
        lobbyFilesPendingInput = ""
        lobbySettingsMode = ""
        lobbyOpenAfterCreate = ""
        if (lobbyShowNoKeyboard)
            lobbyDismissNoKeyboard()
        if (idx === 0) lobbyRefreshNotes()
        if (idx === 1) writerdeck.requestLobbyInfo()
        if (idx === 5) lobbyRefreshVersion()
        lobbyKeepFocus()
    }

    // About tab: one product stamp from server + editor, then compare to GitHub.
    function lobbyRefreshVersion() {
        lobbyVersionText = root.lobbyT("home.versionChecking")
        var ed = "unknown"
        try {
            if (writerdeck && writerdeck.productVersion)
                ed = writerdeck.productVersion
        } catch (e0) {}
        var chk = new XMLHttpRequest()
        chk.open("GET", "http://127.0.0.1:8000/api/version/check?editor="
                 + encodeURIComponent(ed))
        chk.onreadystatechange = function() {
            if (chk.readyState !== XMLHttpRequest.DONE) return
            if (chk.status === 200) {
                try {
                    var k = JSON.parse(chk.responseText)
                    var ver = k.version || ""
                    var latest = k.latest || ""
                    if (k.status === "latest")
                        lobbyVersionText = root.lobbyTf("home.versionLatest", ver)
                    else if (k.status === "outdated")
                        lobbyVersionText = root.lobbyTf("home.versionOutdated", ver, latest)
                    else if (k.status === "ahead")
                        lobbyVersionText = root.lobbyTf("home.versionAhead", ver, latest)
                    else if (k.status === "mismatchLatest")
                        lobbyVersionText = root.lobbyTf("home.versionMismatchLatest", ver, latest)
                    else if (k.status === "mismatch")
                        lobbyVersionText = root.lobbyTf("home.versionMismatch", ver)
                    else if (k.status === "offline")
                        lobbyVersionText = root.lobbyTf("home.versionOffline", ver)
                    else if (k.message)
                        lobbyVersionText = k.message
                    else
                        lobbyVersionText = root.lobbyT("home.versionUnknown")
                    return
                } catch (e2) {}
            }
            lobbyVersionText = root.lobbyT("home.versionUnknown")
        }
        chk.send()
    }

    function lobbyRefreshNotes() {
        writerdeck.requestNotesList()
    }

    function setNotesList(items) {
        // Home save + refresh can deliver two noteslists. Keep the selected name across clear/rebuild
        // so the second push does not wipe last-edited focus (ListView clear sets currentIndex to -1).
        var openAfter = lobbyOpenAfterCreate
        var prefer = lobbyLastEditedFile
        var keepName = ""
        if (prefer === "" && openAfter === "" && lobbyFilesIndex >= 0 && lobbyFilesIndex < lobbyNotesModel.count) {
            var cur = lobbyNotesModel.get(lobbyFilesIndex)
            if (cur && cur.name) keepName = cur.name
        }
        lobbyNotesModel.clear()
        if (!items) {
            lobbyFilesSetIndex(0)
            if (openAfter !== "")
                lobbyOpenAfterCreate = ""
            lobbyFilesPendingMode = ""
            lobbyFilesPendingInput = ""
            return
        }
        for (var i = 0; i < items.length; i++) {
            var it = items[i]
            lobbyNotesModel.append({
                name: it.name !== undefined ? it.name : "",
                size: it.size !== undefined ? it.size : 0,
                modified: it.modified !== undefined ? it.modified : "",
                encrypted: !!it.encrypted
            })
        }
        // Successful create/rename pushes a notes list — drop in-flight dialog restore state.
        if (lobbyFilesPendingMode !== "") {
            lobbyFilesPendingMode = ""
            lobbyFilesPendingInput = ""
        }
        if (openAfter !== "") {
            if (selectNoteByName(openAfter)) {
                lobbyOpenAfterCreate = ""
                Qt.callLater(function() {
                    if (isLobby)
                        lobbyOpenSelected(true)
                })
            } else {
                // Create failed or name missing — do not open a later match by accident.
                lobbyOpenAfterCreate = ""
                lobbyFilesSetIndex(Math.max(0, lobbyNotesModel.count - 1))
            }
            return
        }
        var target = prefer !== "" ? prefer : keepName
        if (target !== "") {
            if (selectNoteByName(target)) {
                if (prefer !== "")
                    lobbyLastEditedFile = ""
            } else {
                lobbyFilesSetIndex(Math.max(0, lobbyNotesModel.count - 1))
            }
        } else if (lobbyFilesIndex < 0 || lobbyFilesIndex >= lobbyNotesModel.count) {
            lobbyFilesSetIndex(Math.max(0, lobbyNotesModel.count - 1))
        }
    }

    function selectNoteByName(name) {
        for (var i = 0; i < lobbyNotesModel.count; i++) {
            if (lobbyNotesModel.get(i).name === name) {
                lobbyFilesSetIndex(i)
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

    function lobbyOpenSelected(fromKey) {
        // fromKey: key chord already proves a keyboard path (USB or phone).
        if (!fromKey && !lobbyEnsureKeyboard("edit")) return
        if (lobbyNotesModel.count === 0) return
        var row = lobbyNotesModel.get(lobbyFilesIndex)
        if (!row || row.name === "") return
        if (row.encrypted) {
            vaultPendingLoad = row.name
            vaultBeginPIN(root.lobbyT("vault.edit"), true); return }
        saveAndLoad(row.name)
    }

    // Tap on the already-selected Files row: edit when a keyboard is present,
    // otherwise open read (no connect-keyboard tip). Edit button still requires
    // a keyboard via lobbyOpenSelected.
    function lobbyTapSelected() {
        if (lobbyKeyboardReady())
            lobbyOpenSelected()
        else
            lobbyReadSelected()
    }

    function lobbyReadSelected() {
        if (lobbyNotesModel.count === 0) return
        var row = lobbyNotesModel.get(lobbyFilesIndex)
        if (!row || row.name === "") return
        if (row.encrypted) {
            vaultPendingLoad = row.name
            lobbyOpenInReadMode = true
            vaultBeginPIN(root.lobbyT("vault.read"), true); return }
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

    function lobbyFilesBeginNew(fromKey) {
        if (!fromKey && !lobbyEnsureKeyboard("new")) return
        lobbyFilesInputError = ""
        lobbyFilesPendingMode = ""
        lobbyFilesPendingInput = ""
        lobbyFilesMode = "new"
        lobbyFilesInput = ""
        lobbyFilesInputPos = 0
    }

    function lobbyFilesBeginRename(fromKey) {
        if (!fromKey && !lobbyEnsureKeyboard("rename")) return
        if (lobbyNotesModel.count === 0) return
        var n = lobbyNotesModel.get(lobbyFilesIndex).name
        lobbyFilesInputError = ""
        lobbyFilesPendingMode = ""
        lobbyFilesPendingInput = ""
        lobbyFilesInput = lobbyFilesStripSuffix(n)
        lobbyFilesInputPos = lobbyFilesInput.length
        lobbyFilesMode = "rename"
    }

    function lobbyFilesBeginDelete() {
        if (lobbyNotesModel.count === 0) return
        lobbyFilesMode = "confirm-delete"
    }

    function lobbyFilesBeginDownload() {
        if (lobbyNotesModel.count === 0) return
        if (!lobbyPhoneConnected) {
            lobbyVaultError = root.lobbyT("files.downloadNeedPhone")
            return
        }
        lobbyVaultError = ""
        writerdeck.offerDownload(lobbyNotesModel.get(lobbyFilesIndex).name)
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

    function lobbyFilesNameTaken(targetName, ignoreName) {
        var key = lobbyFilesTitleKey(targetName)
        if (!key) return false
        for (var i = 0; i < lobbyNotesModel.count; i++) {
            var row = lobbyNotesModel.get(i)
            if (!row || !row.name) continue
            if (ignoreName && row.name === ignoreName) continue
            if (lobbyFilesTitleKey(row.name) === key) return true
        }
        return false
    }

    function lobbyFilesSubmitInput() {
        var name = lobbyFilesInput.trim()
        if (name === "") { lobbyFilesMode = ""; return }
        if (lobbyFilesMode === "new") {
            var newTarget = lobbyFilesNormalizedName(name, false)
            if (lobbyFilesNameTaken(newTarget, "")) {
                lobbyOpenAfterCreate = ""
                lobbyFilesInputError = root.lobbyT("files.nameExists")
                return
            }
            lobbyFilesInputError = ""
            lobbyFilesPendingMode = "new"
            lobbyFilesPendingInput = lobbyFilesInput
            lobbyOpenAfterCreate = newTarget
            writerdeck.createNote(name)
            lobbyFilesMode = ""
            lobbyFilesInput = ""
            lobbyFilesInputPos = 0
        } else if (lobbyFilesMode === "rename") {
            var oldName = lobbyNotesModel.get(lobbyFilesIndex).name
            var newName = name
            if (oldName.endsWith(".md.enc")) newName = name + ".md.enc"
            else newName = lobbyFilesNormalizedName(name, false)
            if (newName !== oldName && lobbyFilesNameTaken(newName, oldName)) {
                lobbyOpenAfterCreate = ""
                lobbyFilesInputError = root.lobbyT("files.nameExists")
                return
            }
            lobbyFilesInputError = ""
            lobbyFilesPendingMode = "rename"
            lobbyFilesPendingInput = lobbyFilesInput
            writerdeck.renameNote(oldName, newName)
            lobbyFilesMode = ""
            lobbyFilesInput = ""
            lobbyFilesInputPos = 0
        } else if (lobbyFilesMode === "new-encrypted") {
            var encTarget = lobbyFilesNormalizedName(name, true)
            if (lobbyFilesNameTaken(encTarget, "")) {
                lobbyOpenAfterCreate = ""
                lobbyFilesInputError = root.lobbyT("files.nameExists")
                return
            }
            lobbyFilesInputError = ""
            lobbyFilesPendingMode = "new-encrypted"
            lobbyFilesPendingInput = lobbyFilesInput
            lobbyOpenAfterCreate = encTarget
            writerdeck.createEncryptedNote(name)
            lobbyFilesMode = ""
            lobbyFilesInput = ""
            lobbyFilesInputPos = 0
        }
    }

    function lobbyFilesBeginNewEncrypted(fromKey) {
        if (!fromKey && !lobbyEnsureKeyboard("new-encrypted")) return
        vaultPendingAction = "new-encrypted"
        vaultBeginPIN(root.lobbyT("vault.create"), false)
    }

    function lobbyEncryptSelected() {
        if (lobbyNotesModel.count === 0) return
        var row = lobbyNotesModel.get(lobbyFilesIndex)
        if (!row || row.encrypted) return
        vaultPendingNote = row.name
        vaultPendingAction = "encrypt"
        vaultBeginPIN(root.lobbyT("vault.encrypt"), false)
    }

    function lobbyDecryptSelected() {
        if (lobbyNotesModel.count === 0) return
        var row = lobbyNotesModel.get(lobbyFilesIndex)
        if (!row || !row.encrypted) return
        vaultPendingNote = row.name
        vaultPendingAction = "decrypt"
        vaultBeginPIN(root.lobbyT("vault.decrypt"), false)
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
        var msg = root.lobbyTf("vault.downloadTitle", name || root.lobbyT("vault.downloadFallbackName"))
        if (reason === "download") msg = root.lobbyT("vault.download")
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
            vaultPinInput = ""
            // Stay on the pad until vaultpinok or vaultopfailed.
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

    // Phone keys arrive as text codepoints (event.key == 0); USB sends Qt::Key_A..Z.
    // Lobby action letter: Ctrl/Cmd+letter only; bindings from lobby-ui.json.
    function lobbyChordLetter(event) {
        if (event.modifiers & Qt.AltModifier)
            return ""
        var ctrl = !!(event.modifiers & (Qt.ControlModifier | Qt.MetaModifier)) || !!ctrlPressed
        if (!ctrl)
            return ""
        if (event.key < Qt.Key_A || event.key > Qt.Key_Z)
            return ""
        var letter = String.fromCharCode(event.key).toLowerCase()
        if (letter === lobbyUi.shortcut("files.rename")
                && !(lobbyPage === 0 && lobbyFilesMode === ""))
            return ""
        return letter
    }

    function lobbyCtrlLetter(event) {
        if (event.modifiers & Qt.AltModifier)
            return ""
        var ctrl = !!(event.modifiers & (Qt.ControlModifier | Qt.MetaModifier)) || !!ctrlPressed
        if (!ctrl)
            return ""
        if (event.key < Qt.Key_A || event.key > Qt.Key_Z)
            return ""
        return String.fromCharCode(event.key).toLowerCase()
    }

    function lobbyShortcutIs(letter, actionId) {
        return letter !== "" && letter === lobbyUi.shortcut(actionId)
    }

    function lobbyShortcutIsEnter(actionId) {
        return lobbyUi.shortcut(actionId) === "enter"
    }

    function lobbyMatchesEnter(event, actionId) {
        return lobbyShortcutIsEnter(actionId)
            && (event.key === Qt.Key_Return || event.key === Qt.Key_Enter)
    }

    function lobbyHandleGlobalChord(event) {
        var letter = lobbyCtrlLetter(event)
        if (letter === "")
            return false
        if (!isLobby && lobbyShortcutIs(letter, "global.toLobby")) {
            handleToLobby()
            return true
        }
        if (isLobby && lobbyShortcutIs(letter, "global.quit")) {
            handleQuitToStock()
            return true
        }
        return false
    }

    function lobbyHandleKey(event) {
        if (lobbyShowNoKeyboard) {
            if (event.key === Qt.Key_Escape) {
                lobbyDismissNoKeyboard()
                return true
            }
            // Phone or USB key while tip is up — continue the pending action.
            lobbyContinueAfterKeyboard()
            return true
        }
        if (vaultOverlayMode !== "") {
            return vaultConsumeKey(event)
        }
        if (lobbyFilesMode === "confirm-delete") {
            if (event.key === Qt.Key_Escape) { lobbyFilesMode = ""; return true }
            if (event.key === Qt.Key_Return) { lobbyFilesDoDelete(); return true }
            return true
        }
        if (lobbySettingsMode === "confirm-exit") {
            if (event.key === Qt.Key_Escape) { lobbySettingsCancelPick(); return true }
            if (event.key === Qt.Key_Return) { lobbySettingsApplyPick(); return true }
            return true
        }
        if (lobbySettingsMode === "pick-language"
                || lobbySettingsMode === "pick-font"
                || lobbySettingsMode === "pick-pin") {
            if (event.key === Qt.Key_Escape) { lobbySettingsCancelPick(); return true }
            if (event.key === Qt.Key_Return) { lobbySettingsApplyPick(); return true }
            if (event.key === Qt.Key_Up || event.key === Qt.Key_Left) {
                var nUp = lobbySettingsPickCount()
                if (nUp > 0)
                    lobbySettingsPickIndex = (lobbySettingsPickIndex + nUp - 1) % nUp
                return true
            }
            if (event.key === Qt.Key_Down || event.key === Qt.Key_Right) {
                var nDn = lobbySettingsPickCount()
                if (nDn > 0)
                    lobbySettingsPickIndex = (lobbySettingsPickIndex + 1) % nDn
                return true
            }
            return true
        }
        if (lobbyFilesMode === "new" || lobbyFilesMode === "rename" || lobbyFilesMode === "new-encrypted") {
            if (event.key === Qt.Key_Escape) {
                lobbyFilesInputError = ""
                lobbyFilesPendingMode = ""
                lobbyFilesPendingInput = ""
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
                    lobbyFilesInputError = ""
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
                lobbyFilesInputError = ""
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
        if (event.key === Qt.Key_Left && event.modifiers === Qt.NoModifier) {
            lobbyGoPage((lobbyPage + lobbyTabLabels.length - 1) % lobbyTabLabels.length)
            return true
        }
        if (event.key === Qt.Key_Right && event.modifiers === Qt.NoModifier) {
            lobbyGoPage((lobbyPage + 1) % lobbyTabLabels.length)
            return true
        }
        if (lobbyHandleGlobalChord(event))
            return true
        var tabLetter = lobbyChordLetter(event)
        if (tabLetter !== "") {
            for (var ti = 0; ti < lobbyTabShortcutIds.length; ti++) {
                if (lobbyShortcutIs(tabLetter, lobbyTabShortcutIds[ti])) {
                    lobbyGoPage(ti)
                    return true
                }
            }
        }
        if (lobbyPage === 1) {
            var kbLetter = lobbyChordLetter(event)
            if (lobbyShortcutIs(kbLetter, "keyboard.us")) {
                writerdeck.setKeyboardLayout("us"); return true }
            if (lobbyShortcutIs(kbLetter, "keyboard.no")) {
                writerdeck.setKeyboardLayout("no"); return true }
            if (lobbyShortcutIs(kbLetter, "keyboard.es")) {
                writerdeck.setKeyboardLayout("es"); return true }
            if (lobbyShortcutIs(kbLetter, "keyboard.de")) {
                writerdeck.setKeyboardLayout("de"); return true }
            if (lobbyShortcutIs(kbLetter, "keyboard.fr")) {
                writerdeck.setKeyboardLayout("fr"); return true }
        }
        if (lobbyPage === 2) {
            if (lobbyMatchesEnter(event, "sync.now")
                    || lobbyShortcutIs(lobbyChordLetter(event), "sync.now")) {
                if (lobbySyncReady && !lobbySyncing) writerdeck.syncNow()
                return true
            }
        }
        if (lobbyPage === 3 && lobbySettingsMode === "") {
            var setLetter = lobbyChordLetter(event)
            if (!lobbyEncryptionEnabled && lobbyShortcutIs(setLetter, "settings.enableVault")) {
                vaultBeginSetup(); return true }
            if (lobbyEncryptionEnabled && lobbyShortcutIs(setLetter, "settings.changePin")) {
                vaultBeginChangePIN(); return true }
            if (lobbyShortcutIs(setLetter, "settings.language")) {
                lobbySettingsBeginPickLanguage(); return true }
            if (lobbyShortcutIs(setLetter, "settings.font")) {
                lobbySettingsBeginPickFont(); return true }
            if (lobbyShortcutIs(setLetter, "settings.pin")) {
                lobbySettingsBeginPickPin(); return true }
            if (lobbyShortcutIs(setLetter, "settings.rotation")) {
                var rots = [0, 90, 180, 270]
                var ri = rots.indexOf(root.rotation)
                if (ri < 0) ri = 0
                root.setScreenRotation(rots[(ri + 1) % rots.length])
                return true
            }
            if (lobbyShortcutIs(setLetter, "settings.exit")) {
                lobbySettingsBeginExit(); return true }
        }
        if (lobbyPage === 0 && lobbyFilesMode === "" && lobbyEncryptionEnabled) {
            var vaultLetter = lobbyChordLetter(event)
            if (lobbyShortcutIs(vaultLetter, "files.encrypt")) { lobbyEncryptSelected(); return true }
            if (lobbyShortcutIs(vaultLetter, "files.decrypt")) { lobbyDecryptSelected(); return true }
            if (lobbyShortcutIs(vaultLetter, "files.newEncrypted")) { lobbyFilesBeginNewEncrypted(true); return true }
        }
        if (lobbyPage === 0) {
            var ps = Math.max(1, lobbyFilesPageSize)
            var last = Math.max(0, lobbyNotesModel.count - 1)
            if (event.key === Qt.Key_Up) {
                lobbyFilesSetIndex(lobbyFilesIndex - 1)
                return true
            }
            if (event.key === Qt.Key_Down) {
                lobbyFilesSetIndex(lobbyFilesIndex + 1)
                return true
            }
            if (event.key === Qt.Key_PageUp) {
                lobbyFilesSetIndex(lobbyFilesIndex - ps)
                return true
            }
            if (event.key === Qt.Key_PageDown) {
                lobbyFilesSetIndex(Math.min(last, lobbyFilesIndex + ps))
                return true
            }
            if (lobbyMatchesEnter(event, "files.edit")
                    || lobbyShortcutIs(lobbyChordLetter(event), "files.edit")) {
                lobbyOpenSelected(true)
                return true
            }
            var letter = lobbyChordLetter(event)
            if (lobbyShortcutIs(letter, "files.new")) {
                lobbyFilesBeginNew(true)
                return true
            }
            if (lobbyShortcutIs(letter, "files.delete")) {
                lobbyFilesBeginDelete()
                return true
            }
            if (lobbyShortcutIs(letter, "files.rename")) {
                lobbyFilesBeginRename(true)
                return true
            }
            if (lobbyShortcutIs(letter, "files.read")) {
                lobbyReadSelected()
                return true
            }
            if (lobbyShortcutIs(letter, "files.download")) {
                lobbyFilesBeginDownload()
                return true
            }
        }
        // settings.rotation letter rotates on any Lobby page (Settings also cycles via its own handler).
        if (lobbyShortcutIs(lobbyChordLetter(event), "settings.rotation")) {
            rotateScreen()
            return true
        }
        return false
    }

    function prepareSleep() {
        if (mode == 1) doc = query.text
        saveFile()
        isLobby = false
        isSleeping = true
    }

    function handleQuitToStock() {
        Qt.quit()
    }

    function handleToLobby() {
        harnessSetWidth(0)
        if (mode == 1) doc = query.text
        // Remember before saveFile: sync XHR can re-enter the event loop and deliver
        // noteslist before this function continues.
        var lastFile = currentFile
        if (lastFile !== "") lobbyLastEditedFile = lastFile
        saveFile()
        isLobby = true
        currentFile = ""
        doc = ""
        query.text = ""
        autosaveSnapshot = ""
        lobbyFilesMode = ""
        lobbyPage = 0
        lobbyRefreshNotes()
    }

    // Physical Home button (socket cmd). Honors hardware_home bindings only.
    function handleHome() {
        if (isLobby) {
            if (lobbyUi.shortcut("global.quit") === "hardware_home")
                handleQuitToStock()
        } else if (lobbyUi.shortcut("global.toLobby") === "hardware_home") {
            handleToLobby()
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
    // Mirrors editHelper.cursorAssoc for caret paint + harness (soft-wrap stickiness).
    property int caretAssoc: 0
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
                var lineEnd = macLineEndPos(pos, text)
                setCaretAssoc(-1)
                moveCursorTo(lineEnd, false)
                return
            }
            if (key === Qt.Key_Left) {
                var lineStart = macLineStartPos(pos, text)
                setCaretAssoc(1)
                moveCursorTo(lineStart, false)
                return
            }
            if (key === Qt.Key_Up) { setCaretAssoc(0); moveCursorTo(0, false); return }
            if (key === Qt.Key_Down) { setCaretAssoc(0); moveCursorTo(text.length, false); return }
            if (key === Qt.Key_End) { setCaretAssoc(0); moveCursorTo(text.length, false); return }
            if (key === Qt.Key_Home) { setCaretAssoc(0); moveCursorTo(0, false); return }
        }
        if (!shift && !cmd && alt) {
            setCaretAssoc(0)
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
                setCaretAssoc(-1)
                return
            }
            if (key === Qt.Key_Left) {
                extendSelectionHorizontal(macLineStartPos(selectionExtendFrom(Qt.Key_Left), text))
                setCaretAssoc(1)
                return
            }
            setCaretAssoc(0)
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
            setCaretAssoc(0)
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
            setCaretAssoc(0)
            if (key === Qt.Key_Right) { moveCursorTo(Math.min(pos + 1, query.text.length), false); return }
            if (key === Qt.Key_Left) { moveCursorTo(Math.max(0, pos - 1), false); return }
        }
    }

    function publishEditorState() {
        var cy = 0
        try { if (typeof flick !== "undefined") cy = Math.round(flick.contentY) } catch (e) {}
        writerdeck.publishState(query.cursorPosition, query.selectionStart,
            query.selectionEnd, query.text.length, mode, isLobby ? 1 : 0,
            vaultOverlayMode, currentFile, query.text, cy,
            editHelper.cursorAssoc(), caretPaintY())
    }

    // Y where the caret should *look* (previous visual row when stuck at wrap end).
    function caretPaintY() {
        var pos = query.cursorPosition
        var here = query.positionToRectangle(pos)
        if (editHelper.cursorAssoc() < 0 && pos > 0) {
            var prev = query.positionToRectangle(pos - 1)
            if (Math.abs(here.y - prev.y) > 0.5)
                return Math.round(prev.y)
        }
        return Math.round(here.y)
    }

    function caretStickyDx() {
        var pos = query.cursorPosition
        var here = query.positionToRectangle(pos)
        if (editHelper.cursorAssoc() >= 0 || pos <= 0)
            return 0
        var prev = query.positionToRectangle(pos - 1)
        if (Math.abs(here.y - prev.y) < 0.5)
            return 0
        return (prev.x + Math.max(prev.width, 1)) - here.x
    }

    function caretStickyDy() {
        var pos = query.cursorPosition
        var here = query.positionToRectangle(pos)
        if (editHelper.cursorAssoc() >= 0 || pos <= 0)
            return 0
        var prev = query.positionToRectangle(pos - 1)
        if (Math.abs(here.y - prev.y) < 0.5)
            return 0
        return prev.y - here.y
    }

    function setCaretAssoc(a) {
        editHelper.setCursorAssoc(a)
        caretAssoc = a
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
        // Soft-wrap exclusive end shares an index with the next row start.
        // Probe the painted row so goalX is the right edge, not col 0 of the next.
        syncEditHelperQuery()
        goalX = query.positionToRectangle(editHelper.wrapProbePos(pos)).x
    }

    function goalXFor(pos) {
        if (goalX >= 0) return goalX
        syncEditHelperQuery()
        return query.positionToRectangle(editHelper.wrapProbePos(pos)).x
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
            setCaretAssoc(0)
            var c = r.toMin
                ? Math.min(query.selectionStart, query.selectionEnd)
                : Math.max(query.selectionStart, query.selectionEnd)
            clearShiftSelection()
            query.deselect()
            query.cursorPosition = c
        } else if (action === "moveTo") {
            setCaretAssoc(0)
            if (r.extend)
                extendSelectionHorizontal(r.pos)
            else
                moveCursorTo(r.pos, false, r.keepGoalColumn === true)
        } else if (action === "moveToResolved") {
            var p = resolveMacPosKind(r.posKind, r.extendKey)
            // Set affinity before moveCursorTo so rememberGoalX / onCursorPositionChanged
            // probe the painted visual row (not the next row's col 0 at a wrap point).
            if (r.posKind === "macLineEndCursor" || r.posKind === "macLineEndExtend")
                setCaretAssoc(-1)
            else if (r.posKind === "macLineStartCursor" || r.posKind === "macLineStartExtend")
                setCaretAssoc(1)
            else
                setCaretAssoc(0)
            if (r.extend)
                extendSelectionHorizontal(p)
            else
                moveCursorTo(p, false)
        } else if (action === "shiftHorizDelta") {
            setCaretAssoc(0)
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
                setCaretAssoc(-1)
            else if (r.posKind === "macLineStartShiftHead")
                setCaretAssoc(1)
            else
                setCaretAssoc(0)
        } else if (action === "shiftVert") {
            extendSelectionVertical(r.down)
            setCaretAssoc(0)
        } else if (action === "moveVert") {
            // Keep assoc through the step so Up/Down from a wrap-end stay on
            // the visual row the caret is painted on; clear after landing.
            moveCursorVertical(r.down)
            setCaretAssoc(0)
        } else {
            return false
        }
        cursorStrong = true
        cursorTimer.stop()
        return true
    }

    function applyMacBackspaceDispatch(r) {
        if (!r.handled) return false
        setCaretAssoc(0)
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
        // End / Cmd+Right leave assoc -1 at the exclusive wrap point. Down must
        // land on the next visual row's end (same exclusivity), not the last glyph.
        var fromVisualEnd = (caretAssoc < 0)
        var newPos = down ? lineDownPos(pos, text) : lineUpPos(pos, text)
        if (newPos === pos) {
            if (down) {
                var gx = goalXFor(pos)
                var vis = visualLineDownPos(pos, gx)
                if (vis > pos) {
                    newPos = vis
                } else if (onWrappedLine(pos, text)) {
                    // Soft-wrap: never fall through to logical paragraph end
                    // (that was End-then-Down jumping to EOF at the wrap point).
                    newPos = pos
                } else {
                    newPos = lineEndPos(pos, text)
                }
            } else {
                newPos = macLineStartPos(pos, text)
            }
        }
        if (down && fromVisualEnd && newPos !== pos)
            newPos = macLineEndPos(newPos, text)
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
            lastFile = currentFile
            if (lastFile !== "") lobbyLastEditedFile = lastFile
            saveFile()
            currentFile = ""
            doc = ""
            query.text = ""
            autosaveSnapshot = ""
        }
        isLobby = true
        lobbyFilesMode = ""
        lobbyPage = 0
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

    function setScreenRotation(deg) {
        var d = Math.round(Number(deg)) % 360
        if (d < 0) d += 360
        if (d !== 0 && d !== 90 && d !== 180 && d !== 270)
            d = 0
        root.rotation = d
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
        } else if (lobbyHandleGlobalChord(event)) {
            event.accepted = true
        }
    }
    function handleKeyUp(event) {
        if (event.key === Qt.Key_Control) {
            ctrlPressed = false
        }
    }

    function handleKey(event) {
        if (event.key === Qt.Key_Home && event.modifiers === Qt.NoModifier) {
            // Keyboard Home is not hardware_home. Caret Home/End stay in edit
            // (handleMacArrow on press) and New/Rename fields; never Lobby/quit.
            event.accepted = true
            return
        }
        if (isLobby) {
            // Lobby chords (including vault PIN) run on Keys.onPressed via
            // lobbyHandleKey. Accept releases so they do not fall through to
            // edit/preview Escape toggle.
            event.accepted = true
            return
        }
        if (event.key === Qt.Key_Escape) {
            if (!(event.modifiers & (Qt.AltModifier | Qt.ControlModifier))) {
                toggleMode()
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
                focus: !isLobby
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
        ListModel {
            id: lobbyNotesModel
        }
        Rectangle {
            id: lobby
            anchors.fill: parent
            color: lobbyUi.pageBg
            visible: isLobby
            z: 5

            readonly property int pageMargin: lobbyUi.pageMargin
            readonly property int tabBtnHeight: lobbyUi.tabBtnHeight
            readonly property int rowHeight: lobbyUi.rowHeight
            readonly property int actionBtnHeight: lobbyUi.actionBtnHeight
            readonly property int tabSpacing: lobbyUi.tabSpacing
            readonly property int contentSpacing: lobbyUi.contentSpacing
            readonly property int btnBorder: lobbyUi.btnBorder
            readonly property int btnBorderSelected: lobbyUi.btnBorderSelected
            readonly property int shortcutBadgeMargin: lobbyUi.shortcutBadgeMargin
            readonly property color textColor: lobbyUi.textColor
            readonly property color borderColor: lobbyUi.borderColor
            readonly property color badgeTextColor: lobbyUi.badgeTextColor
            readonly property color badgeBorderColor: lobbyUi.badgeBorderColor
            readonly property color btnFill: lobbyUi.btnFill
            readonly property color btnFillSelected: lobbyUi.btnFillSelected
            readonly property color tabFill: lobbyUi.tabFill
            readonly property color tabFillSelected: lobbyUi.tabFillSelected
            readonly property color dialogBg: lobbyUi.dialogBg
            readonly property color dialogScrim: lobbyUi.dialogScrim
            readonly property color vaultWash: lobbyUi.vaultWash
            readonly property int btnRadius: lobbyUi.btnRadius
            readonly property int dialogRadius: lobbyUi.dialogRadius
            readonly property int badgeRadius: lobbyUi.badgeRadius
            readonly property int bannerRadius: lobbyUi.bannerRadius
            readonly property int labelPointSize: lobbyUi.labelPointSize
            readonly property int badgePointSize: lobbyUi.badgePointSize
            readonly property int titlePointSize: lobbyUi.titlePointSize
            readonly property int sectionPointSize: lobbyUi.sectionPointSize
            readonly property int rowPointSize: lobbyUi.rowPointSize
            readonly property int dialogTitlePointSize: lobbyUi.dialogTitlePointSize
            readonly property int bannerPointSize: lobbyUi.bannerPointSize
            readonly property int helpPointSize: lobbyUi.helpPointSize
            readonly property int pageStripHeight: lobbyUi.pageStripHeight
            readonly property int listRowInset: lobbyUi.listRowInset
            readonly property int tabRowExtraHeight: lobbyUi.tabRowExtraHeight
            readonly property double dialogWidthFraction: lobbyUi.dialogWidthFraction
            readonly property int dialogPadding: lobbyUi.dialogPadding
            readonly property int settingsLandscapeScrollGutter: lobbyUi.settingsLandscapeScrollGutter
            readonly property int fontPickerNamePointSize: lobbyUi.fontPickerNamePointSize
            readonly property int fontPickerSamplePointSize: lobbyUi.fontPickerSamplePointSize
            readonly property int fontPickerRowExtra: lobbyUi.fontPickerRowExtra
            readonly property double dialogCancelWidthFraction: lobbyUi.dialogCancelWidthFraction

            // Button caption + optional keycap. Loader sets labelText / shortcutKey / pointSize
            // (and optional labelBold / labelColor).
            Component {
                id: lobbyBtnLabelComp
                Item {
                    anchors.fill: parent
                    readonly property string labelText: parent.labelText
                    readonly property string shortcutKey: parent.shortcutKey
                    readonly property int pointSize: parent.pointSize
                    readonly property bool labelBold: !!parent.labelBold
                    readonly property color labelColor: parent.labelColor !== undefined ? parent.labelColor : lobby.textColor
                    readonly property bool showBadge: shortcutKey !== ""

                    Text {
                        anchors.left: parent.left
                        anchors.right: keyBadge.left
                        anchors.leftMargin: 6
                        anchors.rightMargin: showBadge ? 2 : 6
                        anchors.verticalCenter: parent.verticalCenter
                        text: labelText
                        font.family: "Noto Sans"
                        font.pointSize: pointSize
                        font.bold: labelBold
                        color: labelColor
                        horizontalAlignment: Text.AlignHCenter
                        elide: Text.ElideRight
                    }
                    Rectangle {
                        id: keyBadge
                        anchors.right: parent.right
                        anchors.rightMargin: lobby.shortcutBadgeMargin
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.topMargin: lobby.shortcutBadgeMargin
                        anchors.bottomMargin: lobby.shortcutBadgeMargin
                        width: showBadge ? height : 0
                        radius: lobby.badgeRadius
                        color: "white"
                        border.color: lobby.badgeBorderColor
                        border.width: showBadge ? lobby.btnBorder : 0
                        // Keep layout width when keyboard drops; only fade the keycap.
                        opacity: (showBadge && root.lobbyKeyboardReady()) ? 1 : 0
                        Text {
                            anchors.centerIn: parent
                            visible: showBadge
                            text: shortcutKey
                            font.family: "Noto Sans"
                            font.pointSize: lobby.badgePointSize
                            font.bold: false
                            color: lobby.badgeTextColor
                        }
                    }
                }
            }

            FocusScope {
                id: lobbyFocus
                anchors.fill: parent
                focus: isLobby
                Keys.enabled: isLobby
                Keys.onPressed: {
                    handleKeyDown(event)
                    if (lobbyHandleKey(event))
                        event.accepted = true
                }
                Keys.onReleased: {
                    handleKeyUp(event)
                    handleKey(event)
                }

                Connections {
                    target: root
                    function onIsLobbyChanged() {
                        if (isLobby) Qt.callLater(function() { lobbyFocus.forceActiveFocus() })
                    }
                    // Touch on tabs/buttons/Flickable can steal focus; keys must stay on Lobby.
                    function onActiveFocusItemChanged() {
                        if (!isLobby) return
                        if (root.activeFocusItem === lobbyFocus) return
                        Qt.callLater(function() {
                            if (isLobby)
                                lobbyFocus.forceActiveFocus()
                        })
                    }
                }

                // ---- tab bar (touch + Tab / arrows / optional tabs.* chords) ----
                Row {
                    id: lobbyTabRow
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: lobby.pageMargin
                    anchors.rightMargin: lobby.pageMargin
                    anchors.topMargin: lobby.pageMargin
                    height: lobby.tabBtnHeight + lobby.tabRowExtraHeight
                    spacing: lobby.tabSpacing

                    Repeater {
                        model: lobbyTabLabels
                        delegate: Rectangle {
                            width: Math.max(88, (lobby.width - lobby.pageMargin * 2 - lobby.tabSpacing * (lobbyTabLabels.length - 1)) / lobbyTabLabels.length)
                            height: lobby.tabBtnHeight
                            radius: lobby.btnRadius
                            color: lobbyPage === index ? lobby.tabFillSelected : lobby.tabFill
                            border.color: lobby.borderColor
                            border.width: lobbyPage === index ? lobby.btnBorderSelected : lobby.btnBorder

                            Loader {
                                anchors.fill: parent
                                property string labelText: modelData
                                property string shortcutKey: lobbyUi.shortcutBadge(lobbyTabShortcutIds[index])
                                property int pointSize: lobby.labelPointSize
                                sourceComponent: lobbyBtnLabelComp
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    root.lobbyGoPage(index)
                                    root.lobbyKeepFocus()
                                }
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
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: lobby.pageMargin
                    anchors.leftMargin: lobby.pageMargin
                    anchors.rightMargin: lobby.pageMargin
                    // 5 About
                    Item {
                        visible: lobbyPage === 5
                        anchors.fill: parent
                        Flickable {
                            anchors.fill: parent
                            contentWidth: width
                            contentHeight: aboutCol.height
                            clip: true
                            focus: false
                            interactive: true
                            flickableDirection: Flickable.VerticalFlick
                            Column {
                                id: aboutCol
                                width: parent.width
                                spacing: lobby.contentSpacing
                                Text {
                                    text: root.lobbyT("home.brand")
                                    color: lobby.textColor
                                    font.pointSize: lobby.titlePointSize
                                    font.family: "Noto Mono"
                                    width: parent.width
                                }
                                Text {
                                    text: root.lobbyT("home.tagline")
                                    color: lobby.textColor
                                    font.pointSize: lobby.sectionPointSize
                                    font.family: "Noto Sans"
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                }
                                Text {
                                    text: lobbyVersionText !== "" ? lobbyVersionText
                                          : root.lobbyT("home.versionChecking")
                                    color: lobby.textColor
                                    font.pointSize: lobby.labelPointSize
                                    font.family: "Noto Sans"
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                }
                                Text {
                                    text: lobbyNoteCount === 1
                                          ? root.lobbyT("home.docsOne")
                                          : root.lobbyTf("home.docsMany", "" + lobbyNoteCount)
                                    color: lobby.textColor
                                    font.pointSize: lobby.labelPointSize
                                    font.family: "Noto Sans"
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                }
                                Text {
                                    text: root.lobbyT("home.tip")
                                    color: lobby.textColor
                                    font.pointSize: lobby.helpPointSize
                                    font.family: "Noto Sans"
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                }
                                Text {
                                    text: root.lobbyT("home.opensource")
                                    color: lobby.textColor
                                    font.pointSize: 9
                                    font.family: "Noto Mono"
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }
                    }
                    // 1 Files — strict vertical stack (header / list / footer).
                    // Every slot has an explicit height (0 when unused). The list only
                    // fills between header.bottom and footer.top so chrome cannot sink
                    // or be painted under the note rows.
                    Item {
                        visible: lobbyPage === 0
                        anchors.fill: parent

                        // ---- header: vault / open errors (height 0 when idle) ----
                        Item {
                            id: lobbyFilesHeader
                            anchors.top: parent.top
                            anchors.left: parent.left
                            anchors.right: parent.right
                            height: lobbyFilesFeedbackBox.visible ? lobbyFilesFeedbackBox.height : 0
                            z: 2

                            Rectangle {
                                id: lobbyFilesFeedbackBox
                                anchors.top: parent.top
                                anchors.left: parent.left
                                anchors.right: parent.right
                                visible: lobbyVaultError !== "" && lobbyFilesMode === ""
                                height: lobbyFilesFeedbackCol.height + 24
                                color: lobby.dialogBg
                                border.color: lobby.borderColor
                                border.width: lobby.btnBorder
                                radius: lobby.dialogRadius
                                clip: true

                                Column {
                                    id: lobbyFilesFeedbackCol
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.top: parent.top
                                    anchors.leftMargin: 16
                                    anchors.rightMargin: 16
                                    anchors.topMargin: 12
                                    spacing: 8

                                    Text {
                                        width: parent.width
                                        wrapMode: Text.WordWrap
                                        font.family: "Noto Sans"
                                        font.pointSize: 13
                                        color: lobby.textColor
                                        text: lobbyVaultError
                                    }

                                    Text {
                                        width: parent.width
                                        horizontalAlignment: Text.AlignHCenter
                                        font.family: "Noto Sans"
                                        font.pointSize: lobby.labelPointSize
                                        color: lobby.textColor
                                        text: root.lobbyT("files.tapDismiss")
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        lobbyVaultError = ""
                                        root.lobbyKeepFocus()
                                    }
                                }
                            }
                        }

                        // ---- footer: pagination strip + action bars (pinned to bottom) ----
                        Column {
                            id: lobbyFilesFooter
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            spacing: lobby.contentSpacing
                            z: 2

                            // Prev / Page N/M / Next — only when notes spill one screen.
                            Row {
                                id: lobbyFilesPageStrip
                                width: parent.width
                                height: visible ? lobby.pageStripHeight : 0
                                spacing: lobby.tabSpacing
                                visible: lobbyFilesMode === "" && lobbyNotesModel.count > lobbyFilesPageSize

                                property bool canPrev: root.lobbyFilesPageIndex() > 0
                                property bool canNext: root.lobbyFilesPageIndex() + 1 < root.lobbyFilesPageCount()

                                Rectangle {
                                    width: (lobbyFilesPageStrip.width - lobby.tabSpacing * 2) / 4
                                    height: lobby.pageStripHeight
                                    radius: lobby.btnRadius
                                    color: lobby.btnFill
                                    border.color: lobby.borderColor
                                    border.width: lobby.btnBorder
                                    opacity: lobbyFilesPageStrip.canPrev ? 1 : 0.45
                                    Text {
                                        anchors.centerIn: parent
                                        text: root.lobbyT("files.prev")
                                        font.family: "Noto Sans"
                                        font.pointSize: lobby.labelPointSize
                                        color: lobby.textColor
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        enabled: lobbyFilesPageStrip.canPrev
                                        onClicked: {
                                            var ps = Math.max(1, lobbyFilesPageSize)
                                            root.lobbyFilesSetIndex(lobbyFilesIndex - ps)
                                            root.lobbyKeepFocus()
                                        }
                                    }
                                }
                                Item {
                                    width: (lobbyFilesPageStrip.width - lobby.tabSpacing * 2) / 2
                                    height: lobby.pageStripHeight
                                    Text {
                                        anchors.centerIn: parent
                                        text: root.lobbyTf("files.page",
                                                           "" + (root.lobbyFilesPageIndex() + 1),
                                                           "" + root.lobbyFilesPageCount())
                                        font.family: "Noto Sans"
                                        font.pointSize: lobby.labelPointSize
                                        color: lobby.textColor
                                    }
                                }
                                Rectangle {
                                    width: (lobbyFilesPageStrip.width - lobby.tabSpacing * 2) / 4
                                    height: lobby.pageStripHeight
                                    radius: lobby.btnRadius
                                    color: lobby.btnFill
                                    border.color: lobby.borderColor
                                    border.width: lobby.btnBorder
                                    opacity: lobbyFilesPageStrip.canNext ? 1 : 0.45
                                    Text {
                                        anchors.centerIn: parent
                                        text: root.lobbyT("files.next")
                                        font.family: "Noto Sans"
                                        font.pointSize: lobby.labelPointSize
                                        color: lobby.textColor
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        enabled: lobbyFilesPageStrip.canNext
                                        onClicked: {
                                            var ps = Math.max(1, lobbyFilesPageSize)
                                            var last = Math.max(0, lobbyNotesModel.count - 1)
                                            root.lobbyFilesSetIndex(Math.min(last, lobbyFilesIndex + ps))
                                            root.lobbyKeepFocus()
                                        }
                                    }
                                }
                            }

                            Rectangle {
                                id: lobbyFilesPageSep
                                width: parent.width
                                height: visible ? 1 : 0
                                color: lobby.borderColor
                                visible: lobbyFilesPageStrip.visible
                            }

                            Row {
                                id: lobbyFilesBar
                                width: parent.width
                                height: visible ? (lobby.actionBtnHeight + 8) : 0
                                spacing: lobby.tabSpacing
                                visible: lobbyFilesMode === ""

                                Repeater {
                                    model: [
                                        { action: "files.new" },
                                        { action: "files.edit" },
                                        { action: "files.read" },
                                        { action: "files.rename" },
                                        { action: "files.delete" },
                                        { action: "files.download" }
                                    ]
                                    delegate: Rectangle {
                                        width: (lobbyFilesBar.width - lobby.tabSpacing * 5) / 6
                                        height: lobby.actionBtnHeight
                                        radius: lobby.btnRadius
                                        color: lobby.btnFill
                                        border.color: lobby.borderColor
                                        border.width: lobby.btnBorder
                                        Loader {
                                            anchors.fill: parent
                                            property string labelText: root.lobbyT(modelData.action)
                                            property string shortcutKey: lobbyUi.shortcutBadge(modelData.action)
                                            property int pointSize: 10
                                            sourceComponent: lobbyBtnLabelComp
                                        }
                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                if (modelData.action === "files.new") root.lobbyFilesBeginNew()
                                                else if (modelData.action === "files.edit") root.lobbyOpenSelected()
                                                else if (modelData.action === "files.read") root.lobbyReadSelected()
                                                else if (modelData.action === "files.rename") root.lobbyFilesBeginRename()
                                                else if (modelData.action === "files.delete") root.lobbyFilesBeginDelete()
                                                else if (modelData.action === "files.download") root.lobbyFilesBeginDownload()
                                                root.lobbyKeepFocus()
                                            }
                                        }
                                    }
                                }
                            }

                            Row {
                                id: lobbyFilesVaultBar
                                width: parent.width
                                height: visible ? (lobby.actionBtnHeight + 4) : 0
                                spacing: lobby.tabSpacing
                                visible: lobbyFilesMode === "" && lobbyEncryptionEnabled && lobbyNotesModel.count > 0

                                property bool selectedEncrypted: {
                                    if (lobbyNotesModel.count === 0) return false
                                    if (lobbyFilesIndex < 0 || lobbyFilesIndex >= lobbyNotesModel.count)
                                        return false
                                    var row = lobbyNotesModel.get(lobbyFilesIndex)
                                    return !!(row && row.encrypted)
                                }

                                Rectangle {
                                    visible: !lobbyFilesVaultBar.selectedEncrypted
                                    width: (lobbyFilesVaultBar.width - lobby.tabSpacing) / 2
                                    height: lobby.actionBtnHeight
                                    radius: lobby.btnRadius
                                    color: lobby.btnFill
                                    border.color: lobby.borderColor
                                    border.width: lobby.btnBorder
                                    Loader {
                                        anchors.fill: parent
                                        property string labelText: root.lobbyT("files.encrypt")
                                        property string shortcutKey: lobbyUi.shortcutBadge("files.encrypt")
                                        property int pointSize: lobby.labelPointSize
                                        sourceComponent: lobbyBtnLabelComp
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            root.lobbyEncryptSelected()
                                            root.lobbyKeepFocus()
                                        }
                                    }
                                }
                                Rectangle {
                                    visible: !lobbyFilesVaultBar.selectedEncrypted
                                    width: (lobbyFilesVaultBar.width - lobby.tabSpacing) / 2
                                    height: lobby.actionBtnHeight
                                    radius: lobby.btnRadius
                                    color: lobby.btnFill
                                    border.color: lobby.borderColor
                                    border.width: lobby.btnBorder
                                    Loader {
                                        anchors.fill: parent
                                        property string labelText: root.lobbyT("files.newEncrypted")
                                        property string shortcutKey: lobbyUi.shortcutBadge("files.newEncrypted")
                                        property int pointSize: lobby.labelPointSize
                                        sourceComponent: lobbyBtnLabelComp
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            root.lobbyFilesBeginNewEncrypted()
                                            root.lobbyKeepFocus()
                                        }
                                    }
                                }
                                Rectangle {
                                    visible: lobbyFilesVaultBar.selectedEncrypted
                                    width: lobbyFilesVaultBar.width
                                    height: lobby.actionBtnHeight
                                    radius: lobby.btnRadius
                                    color: lobby.btnFill
                                    border.color: lobby.borderColor
                                    border.width: lobby.btnBorder
                                    Loader {
                                        anchors.fill: parent
                                        property string labelText: root.lobbyT("files.decrypt")
                                        property string shortcutKey: lobbyUi.shortcutBadge("files.decrypt")
                                        property int pointSize: lobby.labelPointSize
                                        sourceComponent: lobbyBtnLabelComp
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            root.lobbyDecryptSelected()
                                            root.lobbyKeepFocus()
                                        }
                                    }
                                }
                            }
                        }

                        // ---- list: only the band between header and footer ----
                        Item {
                            id: lobbyFilesList
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.top: lobbyFilesHeader.bottom
                            anchors.topMargin: lobbyFilesHeader.height > 0 ? lobby.contentSpacing : 0
                            anchors.bottom: lobbyFilesFooter.top
                            anchors.bottomMargin: lobbyFilesFooter.height > 0 ? lobby.contentSpacing : 0
                            visible: lobbyFilesMode === ""
                            clip: true
                            z: 1

                            property int pageSize: Math.max(1, Math.floor(Math.max(0, height) / lobby.rowHeight))
                            property int pageStart: root.lobbyFilesPageStart()
                            property int visibleCount: {
                                var end = Math.min(lobbyNotesModel.count, pageStart + pageSize)
                                return Math.max(0, end - pageStart)
                            }

                            onPageSizeChanged: {
                                if (pageSize > 0 && lobbyFilesPageSize !== pageSize)
                                    lobbyFilesPageSize = pageSize
                            }
                            Component.onCompleted: {
                                if (pageSize > 0)
                                    lobbyFilesPageSize = pageSize
                            }

                            Column {
                                anchors.fill: parent
                                spacing: 0
                                Repeater {
                                    model: lobbyFilesList.visibleCount
                                    delegate: Item {
                                        width: lobbyFilesList.width
                                        height: lobby.rowHeight
                                        property int noteIndex: lobbyFilesList.pageStart + index
                                        property var noteRow: lobbyNotesModel.get(noteIndex)
                                        Text {
                                            anchors.left: parent.left
                                            anchors.leftMargin: lobby.listRowInset
                                            anchors.right: parent.right
                                            anchors.rightMargin: lobby.listRowInset
                                            anchors.verticalCenter: parent.verticalCenter
                                            text: (noteIndex === lobbyFilesIndex
                                                   ? root.lobbyT("files.marker")
                                                   : root.lobbyT("files.markerPad"))
                                                  + lobbyFilesStripSuffix(noteRow ? noteRow.name : "")
                                                  + (noteRow && noteRow.encrypted
                                                     ? root.lobbyT("files.privateSuffix") : "")
                                            font.family: "Noto Sans"
                                            font.pointSize: lobby.rowPointSize
                                            color: lobby.textColor
                                            elide: Text.ElideRight
                                        }
                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                if (noteIndex === lobbyFilesIndex)
                                                    root.lobbyTapSelected()
                                                else
                                                    root.lobbyFilesSetIndex(noteIndex)
                                                root.lobbyKeepFocus()
                                            }
                                            onDoubleClicked: root.lobbyTapSelected()
                                        }
                                    }
                                }
                            }
                        }
                    }
                    // 2 Keyboard
                    Item {
                        visible: lobbyPage === 1
                        anchors.fill: parent
                        // Live presence while this tab is open (USB plug / phone page).
                        Timer {
                            interval: 2000
                            running: lobbyPage === 1 && isLobby
                            repeat: true
                            onTriggered: writerdeck.requestLobbyInfo()
                        }
                        Flickable {
                            anchors.fill: parent
                            contentWidth: width
                            contentHeight: kbCol.height
                            clip: true
                            focus: false
                            interactive: true
                            flickableDirection: Flickable.VerticalFlick
                            Column {
                                id: kbCol
                                width: parent.width
                                spacing: lobby.contentSpacing

                                // ---- Bluetooth keyboard (phone bridge) ----
                                Rectangle {
                                    width: parent.width
                                    height: btKbInner.height + 24
                                    color: lobby.dialogBg
                                    border.color: lobby.borderColor
                                    border.width: lobbyPhoneConnected ? lobby.btnBorderSelected : lobby.btnBorder
                                    radius: lobby.btnRadius
                                    Column {
                                        id: btKbInner
                                        anchors.left: parent.left
                                        anchors.right: parent.right
                                        anchors.top: parent.top
                                        anchors.margins: 12
                                        spacing: 10
                                        Text {
                                            text: root.lobbyT("keyboard.btTitle")
                                                  + (lobbyPhoneConnected
                                                     ? root.lobbyT("keyboard.connected")
                                                     : root.lobbyT("keyboard.notConnected"))
                                            font.pointSize: lobby.rowPointSize
                                            font.family: "Noto Sans"
                                            color: lobby.textColor
                                            width: parent.width
                                            wrapMode: Text.WordWrap
                                        }
                                        Text {
                                            text: root.lobbyT("keyboard.btBody")
                                            font.pointSize: lobby.labelPointSize
                                            font.family: "Noto Sans"
                                            color: lobby.textColor
                                            width: parent.width
                                            wrapMode: Text.WordWrap
                                        }
                                        Text {
                                            text: root.lobbyPhoneUrl()
                                            font.pointSize: 13
                                            font.family: "Noto Mono"
                                            color: lobby.textColor
                                            width: parent.width
                                            wrapMode: Text.WrapAnywhere
                                            horizontalAlignment: Text.AlignHCenter
                                        }
                                        Text {
                                            visible: lobbyPIN !== ""
                                            text: root.lobbyT("keyboard.pinPrefix") + lobbyPIN
                                            font.pointSize: lobby.sectionPointSize
                                            font.family: "Noto Sans"
                                            color: lobby.textColor
                                            width: parent.width
                                            horizontalAlignment: Text.AlignHCenter
                                        }
                                        Image {
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            width: Math.min(parent.width * 0.55, 280)
                                            height: width
                                            fillMode: Image.PreserveAspectFit
                                            visible: lobbyQrPath !== ""
                                            source: lobbyQrPath !== "" ? ("file://" + lobbyQrPath) : ""
                                            cache: false
                                        }
                                    }
                                }

                                // ---- USB keyboard ----
                                Rectangle {
                                    width: parent.width
                                    height: usbKbInner.height + 24
                                    color: lobby.dialogBg
                                    border.color: lobby.borderColor
                                    border.width: lobbyUsbKeyboard ? lobby.btnBorderSelected : lobby.btnBorder
                                    radius: lobby.btnRadius
                                    Column {
                                        id: usbKbInner
                                        anchors.left: parent.left
                                        anchors.right: parent.right
                                        anchors.top: parent.top
                                        anchors.margins: 12
                                        spacing: 10
                                        Text {
                                            text: root.lobbyT("keyboard.usbTitle")
                                                  + (lobbyUsbKeyboard
                                                     ? root.lobbyT("keyboard.connected")
                                                     : root.lobbyT("keyboard.notConnected"))
                                            font.pointSize: lobby.rowPointSize
                                            font.family: "Noto Sans"
                                            color: lobby.textColor
                                            width: parent.width
                                            wrapMode: Text.WordWrap
                                        }
                                        Text {
                                            text: root.lobbyT("keyboard.usbBody")
                                            font.pointSize: lobby.labelPointSize
                                            font.family: "Noto Sans"
                                            color: lobby.textColor
                                            width: parent.width
                                            wrapMode: Text.WordWrap
                                        }
                                        Text {
                                            text: root.lobbyT("keyboard.layout")
                                            font.pointSize: lobby.sectionPointSize
                                            font.family: "Noto Sans"
                                            color: lobby.textColor
                                            width: parent.width
                                        }
                                        Grid {
                                            id: kbLayoutGrid
                                            width: parent.width
                                            columns: 2
                                            spacing: lobby.tabSpacing
                                            Repeater {
                                                model: [
                                                    { id: "us", labelKey: "keyboard.us", action: "keyboard.us" },
                                                    { id: "no", labelKey: "keyboard.no", action: "keyboard.no" },
                                                    { id: "es", labelKey: "keyboard.es", action: "keyboard.es" },
                                                    { id: "de", labelKey: "keyboard.de", action: "keyboard.de" },
                                                    { id: "fr", labelKey: "keyboard.fr", action: "keyboard.fr" }
                                                ]
                                                delegate: Rectangle {
                                                    width: (kbLayoutGrid.width - lobby.tabSpacing) / 2
                                                    height: lobby.actionBtnHeight
                                                    radius: lobby.btnRadius
                                                    property bool selected: lobbyKeyboardLayout === modelData.id
                                                    color: selected ? lobby.btnFillSelected : lobby.btnFill
                                                    border.color: lobby.borderColor
                                                    border.width: selected ? lobby.btnBorderSelected : lobby.btnBorder
                                                    Loader {
                                                        anchors.fill: parent
                                                        property string labelText: root.lobbyT(modelData.labelKey)
                                                        property string shortcutKey: lobbyUi.shortcutBadge(modelData.action)
                                                        property int pointSize: lobby.labelPointSize
                                                        sourceComponent: lobbyBtnLabelComp
                                                    }
                                                    MouseArea {
                                                        anchors.fill: parent
                                                        onClicked: {
                                                            writerdeck.setKeyboardLayout(modelData.id)
                                                            root.lobbyKeepFocus()
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
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
                            focus: false
                            interactive: true
                            flickableDirection: Flickable.VerticalFlick
                            Column {
                                id: syncCol
                                width: parent.width
                                spacing: lobby.contentSpacing
                                Text {
                                    text: root.lobbyT("sync.title")
                                    font.pointSize: lobby.rowPointSize
                                    font.family: "Noto Sans"
                                    color: lobby.textColor
                                    width: parent.width
                                }
                                Text {
                                    visible: !(lobbySyncOn && lobbySyncRepo !== "" && !lobbySyncReady)
                                        && !(lobbySyncOn && lobbySyncRepo !== "" && lobbySyncReady && lobbySyncError !== "")
                                    text: lobbySyncOn && lobbySyncRepo !== ""
                                        ? (root.lobbyLastSyncLabel() !== ""
                                            ? root.lobbyTf("sync.lastSync", root.lobbyLastSyncLabel(), lobbySyncRepo)
                                            : root.lobbyTf("sync.syncingTo", lobbySyncRepo))
                                        : root.lobbyTf("sync.notConfigured", lobbyIP, "" + lobbyPort)
                                    font.pointSize: lobby.labelPointSize
                                    font.family: "Noto Sans"
                                    color: lobby.textColor
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                }
                                Rectangle {
                                    visible: lobbySyncOn && lobbySyncRepo !== "" && lobbySyncReady && lobbySyncError !== ""
                                    width: parent.width
                                    height: syncErrCol.height + 20
                                    color: lobby.dialogBg
                                    border.color: lobby.borderColor
                                    border.width: lobby.btnBorder
                                    radius: lobby.bannerRadius
                                    Column {
                                        id: syncErrCol
                                        anchors.centerIn: parent
                                        width: parent.width - 20
                                        spacing: 10
                                        Text {
                                            text: !lobbyWifi ? root.lobbyT("sync.offline") : root.lobbyT("sync.failed")
                                            font.pointSize: lobby.bannerPointSize
                                            font.bold: true
                                            font.family: "Noto Sans"
                                            color: lobby.textColor
                                            width: parent.width
                                        }
                                        Text {
                                            text: lobbySyncError
                                            font.pointSize: 13
                                            font.family: "Noto Sans"
                                            color: lobby.textColor
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
                                    color: lobby.dialogBg
                                    border.color: lobby.borderColor
                                    border.width: lobby.btnBorder
                                    radius: lobby.bannerRadius
                                    Column {
                                        id: tokenWarnCol
                                        anchors.centerIn: parent
                                        width: parent.width - 20
                                        spacing: 10
                                        Text {
                                            text: root.lobbyT("sync.tokenNeeded")
                                            font.pointSize: lobby.bannerPointSize
                                            font.bold: true
                                            font.family: "Noto Sans"
                                            color: lobby.textColor
                                            width: parent.width
                                        }
                                        Text {
                                            text: root.lobbyTf("sync.tokenBody", lobbyIP, "" + lobbyPort, lobbySyncRepo)
                                            font.pointSize: 13
                                            font.family: "Noto Sans"
                                            color: lobby.textColor
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
                                    radius: lobby.btnRadius
                                    color: (lobbySyncReady && !lobbySyncing) ? lobby.btnFill : lobby.dialogBg
                                    border.color: lobby.borderColor
                                    border.width: lobbySyncReady ? lobby.btnBorder : lobby.btnBorderSelected
                                    Loader {
                                        anchors.fill: parent
                                        property string labelText: !lobbySyncReady ? root.lobbyT("sync.tokenBtn")
                                            : (lobbySyncing ? root.lobbyT("sync.syncing") : root.lobbyT("sync.now"))
                                        property string shortcutKey: (lobbySyncReady && !lobbySyncing)
                                            ? lobbyUi.shortcutBadge("sync.now") : ""
                                        property int pointSize: !lobbySyncReady ? 14 : 12
                                        property bool labelBold: !lobbySyncReady
                                        property color labelColor: lobby.textColor
                                        sourceComponent: lobbyBtnLabelComp
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        enabled: lobbySyncReady && !lobbySyncing
                                        onClicked: {
                                            writerdeck.syncNow()
                                            root.lobbyKeepFocus()
                                        }
                                    }
                                }
                                Text {
                                    text: root.lobbyT("sync.footnote")
                                    font.pointSize: lobby.helpPointSize
                                    font.family: "Noto Sans"
                                    color: lobby.textColor
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
                            id: settingsFlick
                            anchors.fill: parent
                            // Landscape only: empty right strip so a finger can flick
                            // without hitting full-width buttons (visual.settingsLandscapeScrollGutter).
                            property int scrollGutter: root.rotation % 180 != 0
                                                       ? lobby.settingsLandscapeScrollGutter : 0
                            contentWidth: width
                            contentHeight: setCol.height
                            clip: true
                            focus: false
                            interactive: true
                            flickableDirection: Flickable.VerticalFlick
                            Column {
                                id: setCol
                                width: parent.width - settingsFlick.scrollGutter
                                spacing: lobby.contentSpacing
                                Text {
                                    text: root.lobbyT("settings.title")
                                    font.pointSize: lobby.rowPointSize
                                    font.family: "Noto Sans"
                                    color: lobby.textColor
                                    width: parent.width
                                }

                                Column {
                                    width: parent.width
                                    spacing: lobby.contentSpacing
                                    visible: lobbySettingsMode === ""

                                    Text {
                                        text: root.lobbyT("settings.languageSection")
                                        font.pointSize: lobby.sectionPointSize
                                        font.family: "Noto Sans"
                                        color: lobby.textColor
                                        width: parent.width
                                    }
                                    Text {
                                        text: root.lobbyT("settings.languageHelp")
                                        font.pointSize: lobby.helpPointSize
                                        font.family: "Noto Sans"
                                        color: lobby.textColor
                                        width: parent.width
                                        wrapMode: Text.WordWrap
                                    }
                                    Rectangle {
                                        width: parent.width
                                        height: lobby.actionBtnHeight
                                        radius: lobby.btnRadius
                                        color: lobby.btnFill
                                        border.color: lobby.borderColor
                                        border.width: lobby.btnBorder
                                        Loader {
                                            anchors.fill: parent
                                            property string labelText: root.lobbySettingsLanguageLabel()
                                            property string shortcutKey: lobbyUi.shortcutBadge("settings.language")
                                            property int pointSize: lobby.labelPointSize
                                            sourceComponent: lobbyBtnLabelComp
                                        }
                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                root.lobbySettingsBeginPickLanguage()
                                                root.lobbyKeepFocus()
                                            }
                                        }
                                    }

                                    Text {
                                        text: root.lobbyT("settings.fontSection")
                                        font.pointSize: lobby.sectionPointSize
                                        font.family: "Noto Sans"
                                        color: lobby.textColor
                                        width: parent.width
                                    }
                                    Text {
                                        text: root.lobbyT("settings.fontHelp")
                                        font.pointSize: lobby.helpPointSize
                                        font.family: "Noto Sans"
                                        color: lobby.textColor
                                        width: parent.width
                                        wrapMode: Text.WordWrap
                                    }
                                    Rectangle {
                                        width: parent.width
                                        height: lobby.actionBtnHeight
                                        radius: lobby.btnRadius
                                        color: lobby.btnFill
                                        border.color: lobby.borderColor
                                        border.width: lobby.btnBorder
                                        Loader {
                                            anchors.fill: parent
                                            property string labelText: readFont
                                            property string shortcutKey: lobbyUi.shortcutBadge("settings.font")
                                            property int pointSize: lobby.labelPointSize
                                            sourceComponent: lobbyBtnLabelComp
                                        }
                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                root.lobbySettingsBeginPickFont()
                                                root.lobbyKeepFocus()
                                            }
                                        }
                                    }

                                    Text {
                                        text: root.lobbyT("settings.privateSection")
                                        font.pointSize: lobby.sectionPointSize
                                        font.family: "Noto Sans"
                                        color: lobby.textColor
                                        width: parent.width
                                    }
                                    Text {
                                        text: lobbyEncryptionEnabled
                                            ? root.lobbyT("settings.privateOn")
                                            : root.lobbyT("settings.privateOff")
                                        font.pointSize: 10
                                        font.family: "Noto Sans"
                                        color: lobby.textColor
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
                                            radius: lobby.btnRadius
                                            color: lobby.btnFill
                                            border.color: lobby.borderColor
                                            border.width: lobby.btnBorder
                                            Loader {
                                                anchors.fill: parent
                                                property string labelText: root.lobbyT("settings.enable")
                                                property string shortcutKey: lobbyUi.shortcutBadge("settings.enableVault")
                                                property int pointSize: lobby.labelPointSize
                                                sourceComponent: lobbyBtnLabelComp
                                            }
                                            MouseArea {
                                                anchors.fill: parent
                                                onClicked: {
                                                    root.vaultBeginSetup()
                                                    root.lobbyKeepFocus()
                                                }
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
                                            radius: lobby.btnRadius
                                            color: lobby.btnFill
                                            border.color: lobby.borderColor
                                            border.width: lobby.btnBorder
                                            Loader {
                                                anchors.fill: parent
                                                property string labelText: root.lobbyT("settings.changePin")
                                                property string shortcutKey: lobbyUi.shortcutBadge("settings.changePin")
                                                property int pointSize: lobby.labelPointSize
                                                sourceComponent: lobbyBtnLabelComp
                                            }
                                            MouseArea {
                                                anchors.fill: parent
                                                onClicked: {
                                                    root.vaultBeginChangePIN()
                                                    root.lobbyKeepFocus()
                                                }
                                            }
                                        }
                                    }

                                    Text {
                                        text: root.lobbyT("settings.pinSection")
                                        font.pointSize: lobby.sectionPointSize
                                        font.family: "Noto Sans"
                                        color: lobby.textColor
                                        width: parent.width
                                    }
                                    Text {
                                        text: root.lobbyT("settings.pinHelp")
                                        font.pointSize: lobby.helpPointSize
                                        font.family: "Noto Sans"
                                        color: lobby.textColor
                                        width: parent.width
                                        wrapMode: Text.WordWrap
                                    }
                                    Rectangle {
                                        width: parent.width
                                        height: lobby.actionBtnHeight
                                        radius: lobby.btnRadius
                                        color: lobby.btnFill
                                        border.color: lobby.borderColor
                                        border.width: lobby.btnBorder
                                        Loader {
                                            anchors.fill: parent
                                            property string labelText: root.lobbySettingsPinLabel()
                                            property string shortcutKey: lobbyUi.shortcutBadge("settings.pin")
                                            property int pointSize: lobby.labelPointSize
                                            sourceComponent: lobbyBtnLabelComp
                                        }
                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                root.lobbySettingsBeginPickPin()
                                                root.lobbyKeepFocus()
                                            }
                                        }
                                    }

                                    Text {
                                        text: root.lobbyT("settings.rotationSection")
                                        font.pointSize: lobby.sectionPointSize
                                        font.family: "Noto Sans"
                                        color: lobby.textColor
                                        width: parent.width
                                    }
                                    Text {
                                        text: root.lobbyT("settings.rotationHelp")
                                        font.pointSize: lobby.helpPointSize
                                        font.family: "Noto Sans"
                                        color: lobby.textColor
                                        width: parent.width
                                        wrapMode: Text.WordWrap
                                    }
                                    Row {
                                        id: rotationRow
                                        width: parent.width
                                        spacing: lobby.tabSpacing
                                        Repeater {
                                            model: [
                                                { deg: 0, labelKey: "settings.rot0" },
                                                { deg: 90, labelKey: "settings.rot90" },
                                                { deg: 180, labelKey: "settings.rot180" },
                                                { deg: 270, labelKey: "settings.rot270" }
                                            ]
                                            delegate: Rectangle {
                                                width: (rotationRow.width - lobby.tabSpacing * 3) / 4
                                                height: lobby.actionBtnHeight
                                                radius: lobby.btnRadius
                                                property bool selected: root.rotation === modelData.deg
                                                color: selected ? lobby.btnFillSelected : lobby.btnFill
                                                border.color: lobby.borderColor
                                                border.width: selected ? lobby.btnBorderSelected : lobby.btnBorder
                                                Text {
                                                    anchors.centerIn: parent
                                                    text: root.lobbyT(modelData.labelKey)
                                                    font.family: "Noto Sans"
                                                    font.pointSize: lobby.sectionPointSize
                                                    color: lobby.textColor
                                                }
                                                MouseArea {
                                                    anchors.fill: parent
                                                    onClicked: {
                                                        root.setScreenRotation(modelData.deg)
                                                        root.lobbyKeepFocus()
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    Text {
                                        text: root.lobbyT("settings.serviceSection")
                                        font.pointSize: lobby.sectionPointSize
                                        font.family: "Noto Sans"
                                        color: lobby.textColor
                                        width: parent.width
                                    }
                                    Text {
                                        text: root.lobbyT("settings.serviceHelp")
                                        font.pointSize: lobby.helpPointSize
                                        font.family: "Noto Sans"
                                        color: lobby.textColor
                                        width: parent.width
                                        wrapMode: Text.WordWrap
                                    }
                                    Rectangle {
                                        width: parent.width
                                        height: lobby.actionBtnHeight
                                        radius: lobby.btnRadius
                                        color: lobby.btnFill
                                        border.color: lobby.borderColor
                                        border.width: lobby.btnBorder
                                        Loader {
                                            anchors.fill: parent
                                            property string labelText: root.lobbyT("settings.exit")
                                            property string shortcutKey: lobbyUi.shortcutBadge("settings.exit")
                                            property int pointSize: 12
                                            sourceComponent: lobbyBtnLabelComp
                                        }
                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                root.lobbySettingsBeginExit()
                                                root.lobbyKeepFocus()
                                            }
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
                            focus: false
                            interactive: true
                            flickableDirection: Flickable.VerticalFlick
                            Column {
                                id: scCol
                                width: parent.width
                                spacing: lobby.contentSpacing
                                Text {
                                    text: root.lobbyT("shortcuts.title")
                                    font.pointSize: lobby.rowPointSize
                                    font.family: "Noto Sans"
                                    color: lobby.textColor
                                    width: parent.width
                                }
                                Text {
                                    text: root.lobbyT("shortcuts.body")
                                    font.pointSize: lobby.labelPointSize
                                    font.family: "Noto Sans"
                                    color: lobby.textColor
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                    lineHeight: 1.25
                                }
                            }
                        }
                    }
                }
            }
        }
        Rectangle {
            id: vaultOverlay
            anchors.fill: parent
            color: lobby.vaultWash
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
                    font.pointSize: lobby.rowPointSize
                    color: lobby.textColor
                    text: vaultOverlayMode === "setup" ? root.lobbyT("vault.setup")
                        : vaultOverlayMode === "confirm" ? root.lobbyT("vault.confirm")
                        : vaultOverlayMode === "change-old" ? root.lobbyT("vault.changeOld")
                        : vaultOverlayMode === "change-new" ? root.lobbyT("vault.changeNew")
                        : vaultOverlayMode === "change-confirm" ? root.lobbyT("vault.changeConfirm")
                        : vaultOverlayReason !== "" ? vaultOverlayReason
                        : root.lobbyT("vault.enter")
                }

                Text {
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    font.family: "Noto Mono"
                    font.pointSize: 18
                    color: lobby.textColor
                    text: vaultPinDisplay()
                }

                Grid {
                    id: vaultPad
                    width: parent.width
                    columns: 3
                    rowSpacing: 8
                    columnSpacing: 8
                    Repeater {
                        // Sixth digit auto-submits; a Done key was redundant (and a no-op under 6 digits).
                        model: ["1","2","3","4","5","6","7","8","9","Bksp","0",""]
                        delegate: Rectangle {
                            width: (vaultPad.width - 16) / 3
                            height: lobby.actionBtnHeight
                            radius: lobby.btnRadius
                            color: modelData === "" ? "transparent" : lobby.btnFill
                            border.color: modelData === "" ? "transparent" : lobby.borderColor
                            border.width: modelData === "" ? 0 : lobby.btnBorder
                            Text {
                                anchors.centerIn: parent
                                text: modelData === "Bksp" ? root.lobbyT("vault.bksp") : modelData
                                font.family: "Noto Sans"
                                font.pointSize: lobby.sectionPointSize
                                color: lobby.textColor
                            }
                            MouseArea {
                                anchors.fill: parent
                                enabled: modelData !== ""
                                onClicked: {
                                    vaultNumpadTap(modelData)
                                    root.lobbyKeepFocus()
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    width: parent.width
                    height: lobby.actionBtnHeight
                    radius: lobby.btnRadius
                    color: lobby.btnFill
                    border.color: lobby.borderColor
                    border.width: lobby.btnBorder
                    Text {
                        anchors.centerIn: parent
                        text: root.lobbyT("dialog.cancel")
                        font.family: "Noto Sans"
                        font.pointSize: lobby.sectionPointSize
                        color: lobby.textColor
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            vaultNumpadCancel()
                            root.lobbyKeepFocus()
                        }
                    }
                }
            }
        }
        // Shared Lobby dialog chrome: grey scrim + white floating box (black type).
        // Kind comes from lobbyShowNoKeyboard / lobbyFilesMode — keep content in one piece.
        Rectangle {
            id: lobbyDialogScrim
            anchors.fill: parent
            color: lobby.dialogScrim
            visible: root.lobbyDialogIsOpen()
            z: 24

            Rectangle {
                id: lobbyDialogBox
                anchors.centerIn: parent
                width: Math.min(parent.width * lobby.dialogWidthFraction, parent.width - lobby.dialogPadding)
                height: lobbyDialogCol.height + lobby.dialogPadding
                color: lobby.dialogBg
                border.color: lobby.borderColor
                border.width: lobby.btnBorderSelected
                radius: lobby.dialogRadius

                Column {
                    id: lobbyDialogCol
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: 24
                    width: parent.width - lobby.dialogPadding
                    spacing: 16

                    Text {
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        font.family: "Noto Sans"
                        font.pointSize: lobby.dialogTitlePointSize
                        color: lobby.textColor
                        text: root.lobbyDialogTitle()
                    }

                    // ---- confirm-delete body ----
                    Text {
                        width: parent.width
                        visible: !lobbyShowNoKeyboard && lobbyFilesMode === "confirm-delete"
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        elide: Text.ElideMiddle
                        font.family: "Noto Sans"
                        font.pointSize: 13
                        color: lobby.textColor
                        text: root.lobbyDialogSelectedNoteLabel()
                    }

                    // ---- new / rename / new-encrypted body ----
                    Text {
                        width: parent.width
                        visible: !lobbyShowNoKeyboard
                                 && (lobbyFilesMode === "new"
                                     || lobbyFilesMode === "rename"
                                     || lobbyFilesMode === "new-encrypted")
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WrapAnywhere
                        font.family: "Noto Mono"
                        font.pointSize: 14
                        color: lobby.textColor
                        text: root.lobbyFilesInputDisplay()
                    }

                    Text {
                        width: parent.width
                        visible: !lobbyShowNoKeyboard
                                 && lobbyFilesInputError !== ""
                                 && (lobbyFilesMode === "new"
                                     || lobbyFilesMode === "rename"
                                     || lobbyFilesMode === "new-encrypted")
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        font.family: "Noto Sans"
                        font.pointSize: 12
                        color: lobby.textColor
                        text: lobbyFilesInputError
                    }

                    // ---- no-keyboard body ----
                    Text {
                        width: parent.width
                        visible: lobbyShowNoKeyboard
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        font.family: "Noto Sans"
                        font.pointSize: 12
                        color: lobby.textColor
                        text: root.lobbyT("dialog.noKeyboardBody")
                    }

                    Text {
                        width: parent.width
                        visible: lobbyShowNoKeyboard
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        font.family: "Noto Mono"
                        font.pointSize: 13
                        color: lobby.textColor
                        text: root.lobbyPhoneUrl()
                    }

                    Text {
                        width: parent.width
                        visible: lobbyShowNoKeyboard && lobbyPIN !== ""
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        font.family: "Noto Sans"
                        font.pointSize: 12
                        color: lobby.textColor
                        text: root.lobbyT("dialog.pinPrefix") + lobbyPIN
                    }

                    Image {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: Math.min(parent.width * 0.55, 280)
                        height: width
                        fillMode: Image.PreserveAspectFit
                        visible: lobbyShowNoKeyboard && lobbyQrPath !== ""
                        source: (lobbyShowNoKeyboard && lobbyQrPath !== "") ? ("file://" + lobbyQrPath) : ""
                        cache: false
                    }

                    // ---- Settings pickers (language / font / PIN) ----
                    Column {
                        width: parent.width
                        spacing: lobby.tabSpacing
                        visible: !lobbyShowNoKeyboard && lobbySettingsMode === "pick-language"
                        Repeater {
                            model: root.lobbySettingsLanguageOptions()
                            delegate: Rectangle {
                                width: parent.width
                                height: lobby.actionBtnHeight
                                radius: lobby.btnRadius
                                property bool selected: index === lobbySettingsPickIndex
                                color: selected ? lobby.btnFillSelected : lobby.btnFill
                                border.color: lobby.borderColor
                                border.width: selected ? lobby.btnBorderSelected : lobby.btnBorder
                                Text {
                                    anchors.centerIn: parent
                                    text: modelData.label
                                    font.family: "Noto Sans"
                                    font.pointSize: lobby.labelPointSize
                                    color: lobby.textColor
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: root.lobbySettingsApplyPick(index)
                                }
                            }
                        }
                    }

                    Column {
                        width: parent.width
                        spacing: lobby.tabSpacing
                        visible: !lobbyShowNoKeyboard && lobbySettingsMode === "pick-font"
                        Repeater {
                            model: root.lobbySettingsFontOptions()
                            delegate: Rectangle {
                                width: parent.width
                                height: lobby.actionBtnHeight + lobby.fontPickerRowExtra
                                radius: lobby.btnRadius
                                property bool selected: index === lobbySettingsPickIndex
                                color: selected ? lobby.btnFillSelected : lobby.btnFill
                                border.color: lobby.borderColor
                                border.width: selected ? lobby.btnBorderSelected : lobby.btnBorder
                                Column {
                                    anchors.centerIn: parent
                                    width: parent.width - 16
                                    spacing: 4
                                    Text {
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        text: modelData.label
                                        font.family: modelData.id
                                        font.pointSize: lobby.fontPickerNamePointSize
                                        color: lobby.textColor
                                    }
                                    Text {
                                        width: parent.width
                                        horizontalAlignment: Text.AlignHCenter
                                        wrapMode: Text.WordWrap
                                        text: root.lobbyFontSample
                                        font.family: modelData.id
                                        font.pointSize: lobby.fontPickerSamplePointSize
                                        color: lobby.textColor
                                    }
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: root.lobbySettingsApplyPick(index)
                                }
                            }
                        }
                    }

                    Column {
                        width: parent.width
                        spacing: lobby.tabSpacing
                        visible: !lobbyShowNoKeyboard && lobbySettingsMode === "pick-pin"
                        Repeater {
                            model: root.lobbySettingsPinOptions()
                            delegate: Rectangle {
                                width: parent.width
                                height: modelData.warnKey ? lobby.actionBtnHeight + 28 : lobby.actionBtnHeight
                                radius: lobby.btnRadius
                                property bool selected: index === lobbySettingsPickIndex
                                color: selected ? lobby.btnFillSelected : lobby.btnFill
                                border.color: lobby.borderColor
                                border.width: selected ? lobby.btnBorderSelected : lobby.btnBorder
                                Column {
                                    anchors.centerIn: parent
                                    width: parent.width - 16
                                    spacing: 2
                                    Text {
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        text: root.lobbyT(modelData.labelKey)
                                        font.family: "Noto Sans"
                                        font.pointSize: lobby.labelPointSize
                                        color: lobby.textColor
                                    }
                                    Text {
                                        visible: !!modelData.warnKey
                                        width: parent.width
                                        horizontalAlignment: Text.AlignHCenter
                                        wrapMode: Text.WordWrap
                                        text: modelData.warnKey ? root.lobbyT(modelData.warnKey) : ""
                                        font.family: "Noto Sans"
                                        font.pointSize: 9
                                        color: lobby.textColor
                                    }
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: root.lobbySettingsApplyPick(index)
                                }
                            }
                        }
                    }

                    // ---- buttons ----
                    Row {
                        width: parent.width
                        spacing: lobby.tabSpacing
                        visible: !lobbyShowNoKeyboard && lobbyFilesMode === "confirm-delete"

                        Rectangle {
                            width: (parent.width - lobby.tabSpacing) / 2
                            height: lobby.actionBtnHeight
                            radius: lobby.btnRadius
                            color: lobby.btnFill
                            border.color: lobby.borderColor
                            border.width: lobby.btnBorder
                            Text {
                                anchors.centerIn: parent
                                text: root.lobbyT("dialog.cancel")
                                font.family: "Noto Sans"
                                font.pointSize: 12
                                color: lobby.textColor
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    lobbyFilesMode = ""
                                    root.lobbyKeepFocus()
                                }
                            }
                        }
                        Rectangle {
                            width: (parent.width - lobby.tabSpacing) / 2
                            height: lobby.actionBtnHeight
                            radius: lobby.btnRadius
                            color: lobby.btnFill
                            border.color: lobby.borderColor
                            border.width: lobby.btnBorderSelected
                            Text {
                                anchors.centerIn: parent
                                text: root.lobbyT("dialog.delete")
                                font.family: "Noto Sans"
                                font.pointSize: 12
                                color: lobby.textColor
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    root.lobbyFilesDoDelete()
                                    root.lobbyKeepFocus()
                                }
                            }
                        }
                    }

                    Row {
                        width: parent.width
                        spacing: lobby.tabSpacing
                        visible: !lobbyShowNoKeyboard && lobbySettingsMode === "confirm-exit"

                        Rectangle {
                            width: (parent.width - lobby.tabSpacing) / 2
                            height: lobby.actionBtnHeight
                            radius: lobby.btnRadius
                            color: lobby.btnFill
                            border.color: lobby.borderColor
                            border.width: lobby.btnBorder
                            Text {
                                anchors.centerIn: parent
                                text: root.lobbyT("dialog.cancel")
                                font.family: "Noto Sans"
                                font.pointSize: 12
                                color: lobby.textColor
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: root.lobbySettingsCancelPick()
                            }
                        }
                        Rectangle {
                            width: (parent.width - lobby.tabSpacing) / 2
                            height: lobby.actionBtnHeight
                            radius: lobby.btnRadius
                            color: lobby.btnFill
                            border.color: lobby.borderColor
                            border.width: lobby.btnBorderSelected
                            Text {
                                anchors.centerIn: parent
                                text: root.lobbyT("settings.exit")
                                font.family: "Noto Sans"
                                font.pointSize: 12
                                color: lobby.textColor
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: root.lobbySettingsApplyPick()
                            }
                        }
                    }

                    // Separator + narrower Cancel — not another full-width choice.
                    Rectangle {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width * 0.4
                        height: 2
                        color: lobby.borderColor
                        visible: !lobbyShowNoKeyboard
                                 && (lobbySettingsMode === "pick-language"
                                     || lobbySettingsMode === "pick-font"
                                     || lobbySettingsMode === "pick-pin")
                    }

                    Item {
                        width: parent.width
                        height: lobby.actionBtnHeight
                        visible: !lobbyShowNoKeyboard
                                 && (lobbySettingsMode === "pick-language"
                                     || lobbySettingsMode === "pick-font"
                                     || lobbySettingsMode === "pick-pin")

                        Rectangle {
                            width: Math.max(120, parent.width * lobby.dialogCancelWidthFraction)
                            height: lobby.actionBtnHeight
                            anchors.horizontalCenter: parent.horizontalCenter
                            radius: lobby.btnRadius
                            color: lobby.btnFill
                            border.color: lobby.borderColor
                            border.width: lobby.btnBorder
                            Text {
                                anchors.centerIn: parent
                                text: root.lobbyT("dialog.cancel")
                                font.family: "Noto Sans"
                                font.pointSize: 12
                                color: lobby.textColor
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: root.lobbySettingsCancelPick()
                            }
                        }
                    }

                    Row {
                        width: parent.width
                        spacing: lobby.tabSpacing
                        visible: !lobbyShowNoKeyboard
                                 && (lobbyFilesMode === "new"
                                     || lobbyFilesMode === "rename"
                                     || lobbyFilesMode === "new-encrypted")

                        Rectangle {
                            width: (parent.width - lobby.tabSpacing) / 2
                            height: lobby.actionBtnHeight
                            radius: lobby.btnRadius
                            color: lobby.btnFill
                            border.color: lobby.borderColor
                            border.width: lobby.btnBorder
                            Text {
                                anchors.centerIn: parent
                                text: root.lobbyT("dialog.cancel")
                                font.family: "Noto Sans"
                                font.pointSize: 12
                                color: lobby.textColor
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    lobbyFilesInputError = ""
                                    lobbyFilesPendingMode = ""
                                    lobbyFilesPendingInput = ""
                                    lobbyFilesMode = ""
                                    lobbyFilesInput = ""
                                    lobbyFilesInputPos = 0
                                    root.lobbyKeepFocus()
                                }
                            }
                        }
                        Rectangle {
                            width: (parent.width - lobby.tabSpacing) / 2
                            height: lobby.actionBtnHeight
                            radius: lobby.btnRadius
                            color: lobby.btnFill
                            border.color: lobby.borderColor
                            border.width: lobby.btnBorderSelected
                            Text {
                                anchors.centerIn: parent
                                text: lobbyFilesMode === "rename"
                                      ? root.lobbyT("dialog.rename")
                                      : root.lobbyT("dialog.create")
                                font.family: "Noto Sans"
                                font.pointSize: 12
                                color: lobby.textColor
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    root.lobbyFilesSubmitInput()
                                    root.lobbyKeepFocus()
                                }
                            }
                        }
                    }

                    Row {
                        width: parent.width
                        spacing: lobby.tabSpacing
                        visible: lobbyShowNoKeyboard

                        Rectangle {
                            width: parent.width
                            height: lobby.actionBtnHeight
                            radius: lobby.btnRadius
                            color: lobby.btnFill
                            border.color: lobby.borderColor
                            border.width: lobby.btnBorder
                            Text {
                                anchors.centerIn: parent
                                text: root.lobbyT("dialog.cancel")
                                font.family: "Noto Sans"
                                font.pointSize: 12
                                color: lobby.textColor
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    root.lobbyDismissNoKeyboard()
                                    root.lobbyKeepFocus()
                                }
                            }
                        }
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
                    text: lobbyUi.str("home.sleeping")
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
