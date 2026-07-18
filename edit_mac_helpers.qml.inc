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
        key = parseInt(key)
        mods = parseInt(mods)
        var cmd = (mods & Qt.ControlModifier) !== 0
        var alt = (mods & Qt.AltModifier) !== 0
        var shift = (mods & Qt.ShiftModifier) !== 0
        var text = query.text
        var pos = query.cursorPosition
        if (!shift && cmd && !alt) {
            // Mac: Cmd+Left/Right = line; Cmd+Up/Down and Ctrl+Home/End = document.
            if (key === Qt.Key_Right) { moveCursorTo(macLineEndPos(pos, text), false); return }
            if (key === Qt.Key_Left) { moveCursorTo(macLineStartPos(pos, text), false); return }
            if (key === Qt.Key_Up) { moveCursorTo(0, false); return }
            if (key === Qt.Key_Down) { moveCursorTo(text.length, false); return }
            if (key === Qt.Key_End) { moveCursorTo(text.length, false); return }
            if (key === Qt.Key_Home) { moveCursorTo(0, false); return }
        }
        if (!shift && !cmd && alt) {
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
                return
            }
            if (key === Qt.Key_Left) {
                extendSelectionHorizontal(macLineStartPos(selectionExtendFrom(Qt.Key_Left), text))
                return
            }
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
        if (shiftAnchor < 0) {
            shiftAnchor = query.cursorPosition
            shiftHead = shiftAnchor
        }
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
            var c = r.toMin
                ? Math.min(query.selectionStart, query.selectionEnd)
                : Math.max(query.selectionStart, query.selectionEnd)
            clearShiftSelection()
            query.deselect()
            query.cursorPosition = c
        } else if (action === "moveTo") {
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
        } else if (action === "shiftHorizDelta") {
            if (query.selectionStart === query.selectionEnd
                    && shiftHead >= 0 && shiftHead !== query.cursorPosition)
                clearShiftSelection()
            var headH = (shiftHead >= 0) ? shiftHead : query.cursorPosition
            var newHead = (r.delta < 0)
                ? Math.max(0, headH + r.delta)
                : Math.min(text.length, headH + r.delta)
            applyShiftSelection(newHead)
            lastShiftHorizKey = (r.eventKey !== undefined) ? r.eventKey : eventKey
        } else if (action === "shiftHorizTo") {
            if (query.selectionStart === query.selectionEnd
                    && shiftHead >= 0 && shiftHead !== query.cursorPosition)
                clearShiftSelection()
            if (shiftAnchor < 0)
                shiftAnchor = (query.selectionStart === query.selectionEnd)
                    ? pos : ((r.posKind === "macLineEndShiftHead")
                        ? Math.min(query.selectionStart, query.selectionEnd)
                        : Math.max(query.selectionStart, query.selectionEnd))
            applyShiftSelection(resolveMacPosKind(r.posKind, 0))
        } else if (action === "shiftVert") {
            if (query.selectionStart === query.selectionEnd
                    && shiftHead >= 0 && shiftHead !== query.cursorPosition)
                clearShiftSelection()
            extendSelectionVertical(r.down)
        } else if (action === "moveVert") {
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
        return editHelper.macLineStartPos(pos, text)
    }

    function macLineEndPos(pos, text) {
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
