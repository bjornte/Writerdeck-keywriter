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
            if (key === Qt.Key_Right) { moveCursorTo(lineEndPos(pos, text), false); return }
            if (key === Qt.Key_Left) { moveCursorTo(lineStartPos(pos, text), false); return }
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
            if (key === Qt.Key_Right) {
                query.select(pos, macLineEndPos(pos, text))
                cursorStrong = true
                cursorTimer.stop()
                return
            }
            if (key === Qt.Key_Left) {
                query.select(macLineStartPos(pos, text), pos)
                cursorStrong = true
                cursorTimer.stop()
                return
            }
            if (key === Qt.Key_Down || key === Qt.Key_End) {
                query.select(pos, text.length)
                cursorStrong = true
                cursorTimer.stop()
                return
            }
            if (key === Qt.Key_Up || key === Qt.Key_Home) {
                query.select(0, pos)
                cursorStrong = true
                cursorTimer.stop()
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

    function isSpaceChar(c) {
        return c === " " || c === "\t" || c === "\n"
    }

    function lineStartPos(pos, text) {
        var prev = text.lastIndexOf("\n", pos - 1)
        return prev === -1 ? 0 : prev + 1
    }

    function lineEndPos(pos, text) {
        var nl = text.indexOf("\n", pos)
        return nl === -1 ? text.length : nl
    }

    function lineCharCount(lineStart, text) {
        var end = lineEndPos(lineStart, text)
        return end - lineStart
    }

    function rememberGoalX(pos) {
        goalX = query.positionToRectangle(pos).x
    }

    function goalXFor(pos) {
        if (goalX >= 0) return goalX
        return query.positionToRectangle(pos).x
    }

    function deleteWordLeftPos(pos, text) {
        if (pos <= 0) return 0
        var start = wordLeftPos(pos, text)
        if (start > 0) {
            var s = start - 1
            while (s >= 0 && isSpaceChar(text.charAt(s))) s--
            start = s + 1
        }
        return start
    }

    function deleteLineLeftPos(pos, text) {
        if (pos <= 0) return 0
        var start = lineStartPos(pos, text)
        if (pos > start) return start
        if (start === 0) return 0
        return lineStartPos(start - 1, text)
    }

    function wordLeftPos(pos, text) {
        if (pos <= 0) return 0
        if (pos > text.length) pos = text.length
        pos--
        while (pos > 0 && isSpaceChar(text.charAt(pos))) pos--
        while (pos > 0 && !isSpaceChar(text.charAt(pos - 1))) pos--
        if (pos < text.length && isSpaceChar(text.charAt(pos))) pos++
        return pos
    }

    function wordRightPos(pos, text) {
        var len = text.length
        if (pos >= len) return len
        while (pos < len && !isSpaceChar(text.charAt(pos))) pos++
        while (pos < len && isSpaceChar(text.charAt(pos))) pos++
        return pos
    }

    function paragraphUpPos(pos, text) {
        var lineStart = lineStartPos(pos, text)
        if (lineStart === 0) return 0
        var i = lineStart - 1
        while (i > 0) {
            if (text.charAt(i) === "\n" && text.charAt(i - 1) === "\n")
                return i + 1
            i--
        }
        return 0
    }

    function paragraphDownPos(pos, text) {
        var len = text.length
        var lineEnd = lineEndPos(pos, text)
        if (lineEnd >= len) return len
        for (var i = lineEnd; i < len - 1; i++) {
            if (text.charAt(i) === "\n" && text.charAt(i + 1) === "\n")
                return i + 2
        }
        return lineEnd + 1
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
            query.cursorPosition = newPos
            if (!keepGoalColumn) rememberGoalX(newPos)
            return
        }
        applyShiftSelection(newPos)
    }

    function extendSelectionHorizontal(newPos) {
        applyShiftSelection(newPos)
    }

    function selectionExtendFrom(key) {
        if (shiftHead >= 0) return shiftHead
        var pos = query.cursorPosition
        if (query.selectionStart === query.selectionEnd) return pos
        if (key === Qt.Key_Left || key === Qt.Key_Up)
            return Math.min(query.selectionStart, query.selectionEnd)
        if (key === Qt.Key_Right || key === Qt.Key_Down)
            return Math.max(query.selectionStart, query.selectionEnd)
        return pos
    }

    function visualLineDownPos(pos, gx) {
        var len = query.text.length
        if (pos >= len) return len
        var curRect = query.positionToRectangle(pos)
        var nextLineY = curRect.y + curRect.height - 0.1
        var goalXUse = (gx !== undefined && gx >= 0) ? gx : curRect.x
        var best = -1
        var bestDist = 1e12
        for (var p = pos + 1; p < len; p++) {
            var r = query.positionToRectangle(p)
            if (r.y + 0.1 < nextLineY) continue
            if (best >= 0 && r.y > query.positionToRectangle(best).y + 0.1) break
            var dist = Math.abs(r.x - goalXUse)
            if (best < 0 || dist < bestDist) { best = p; bestDist = dist }
        }
        return best >= 0 ? best : len
    }

    function visualLineUpPos(pos, gx) {
        if (pos <= 0) return 0
        var curRect = query.positionToRectangle(pos)
        var rowTop = curRect.y - curRect.height + 0.1
        var goalXUse = (gx !== undefined && gx >= 0) ? gx : curRect.x
        var best = -1
        var bestDist = 1e12
        var targetY = -1
        for (var p = pos - 1; p >= 0; p--) {
            var r = query.positionToRectangle(p)
            if (r.y > rowTop) continue
            if (targetY < 0) targetY = r.y
            if (Math.abs(r.y - targetY) > 0.5) break
            var dist = Math.abs(r.x - goalXUse)
            if (best < 0 || dist < bestDist) { best = p; bestDist = dist }
        }
        return best >= 0 ? best : 0
    }

    function visualLineStartPos(pos) {
        if (pos <= 0) return 0
        var curY = query.positionToRectangle(pos).y
        var best = pos
        for (var p = pos - 1; p >= 0; p--) {
            if (Math.abs(query.positionToRectangle(p).y - curY) < 0.5) best = p
            else break
        }
        return best
    }

    function visualLineEndPos(pos) {
        var len = query.text.length
        if (pos >= len) return len
        var curY = query.positionToRectangle(pos).y
        var best = pos
        for (var p = pos + 1; p <= len; p++) {
            if (Math.abs(query.positionToRectangle(p).y - curY) < 0.5) best = p
            else break
        }
        return best
    }

    function onWrappedLine(pos, text) {
        var s = lineStartPos(pos, text)
        var nl = text.indexOf("\n", s)
        return nl === -1 || nl > lineEndPos(pos, text)
    }

    function macLineStartPos(pos, text) {
        return onWrappedLine(pos, text) ? visualLineStartPos(pos)
            : lineStartPos(pos, text)
    }

    function macLineEndPos(pos, text) {
        return onWrappedLine(pos, text) ? visualLineEndPos(pos)
            : lineEndPos(pos, text)
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
        // Snap to logical line end/start only when vertical motion crossed a
        // newline. Same-logical-line landings are visual wraps — keep goal x
        // (Mac/Qt). Old unconditional snap jumped wrapped Shift+Down to EOF.
        if (lineStartPos(newHead, text) !== lineStartPos(head, text)) {
            if (down && newHead > head && head > lineStartPos(head, text))
                newHead = lineEndPos(newHead, text)
            if (!down && newHead < head && head < lineEndPos(head, text))
                newHead = lineStartPos(newHead, text)
        }
        if (down && newHead === head && head < text.length
                && text.indexOf("\n", head) === -1 && onWrappedLine(head, text)) {
            var vis = visualLineDownPos(head, goalXFor(head))
            if (vis > head) newHead = vis
        }
        if (down && newHead === head && head === text.length && head > 0
                && shiftAnchor === shiftHead)
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
        if (curText.length <= prevText.length) return null
        var extra = curText.length - prevText.length
        for (var p = 0; p <= prevText.length; p++) {
            if (curText.slice(0, p) === prevText.slice(0, p)
                    && curText.slice(p + extra) === prevText.slice(p))
                return {pos: p, len: extra}
        }
        return null
    }

    function beginTextEdit() {
        if (editUndoCapture || editSkipTextUndoPush || harnessPrepareLock || mode != 1)
            return
        pushEditUndoWithMerge(captureEditState())
        editSkipTextUndoPush = true
    }

    function captureEditState() {
        return {
            text: query.text,
            cursor: query.cursorPosition,
            selStart: query.selectionStart,
            selEnd: query.selectionEnd
        }
    }

    function syncEditUndoSnapshot() {
        _prevTextSnapshot = query.text
        _prevTextLen = query.text.length
        _prevCursor = query.cursorPosition
        _prevSelStart = query.selectionStart
        _prevSelEnd = query.selectionEnd
    }

    function clearEditUndoStacks() {
        editUndoStack = []
        editRedoStack = []
    }

    function isOneCharInsert(prevText, curText) {
        if (curText.length !== prevText.length + 1) return -1
        for (var i = 0; i < curText.length; i++) {
            if (curText.slice(0, i) + curText.slice(i + 1) === prevText) return i
        }
        return -1
    }

    function pushEditUndoWithMerge(prevState) {
        var curText = query.text
        var prevText = prevState.text
        var insPos = isOneCharInsert(prevText, curText)
        if (insPos >= 0 && editUndoStack.length > 0) {
            var top = editUndoStack[editUndoStack.length - 1]
            var fromTop = insertTextDelta(top.text, curText)
            if (fromTop && fromTop.pos === top.cursor
                    && fromTop.pos + fromTop.len === query.cursorPosition) {
                editRedoStack = []
                return
            }
        }
        editUndoStack.push(prevState)
        editRedoStack = []
    }

    function restoreEditState(st) {
        editUndoCapture = true
        query.text = st.text
        query.deselect()
        query.cursorPosition = st.cursor
        doc = query.text
        syncEditUndoSnapshot()
        editUndoCapture = false
    }

    function editUndo() {
        if (editUndoStack.length === 0) return false
        var prevState = editUndoStack[editUndoStack.length - 1]
        var redoState = captureEditState()
        var ins = insertTextDelta(prevState.text, redoState.text)
        if (ins && ins.pos === prevState.cursor) {
            redoState.cursor = ins.pos + ins.len
            redoState.selStart = redoState.cursor
            redoState.selEnd = redoState.cursor
        }
        editRedoStack.push(redoState)
        restoreEditState(editUndoStack.pop())
        return true
    }

    function editRedo() {
        if (editRedoStack.length === 0) return false
        editUndoStack.push(captureEditState())
        restoreEditState(editRedoStack.pop())
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
        var mods = event.modifiers
        if (mods === Qt.NoModifier) {
            if (event.key === Qt.Key_Left || event.key === Qt.Key_Right) {
                if (query.selectionStart !== query.selectionEnd) {
                    var c = (event.key === Qt.Key_Left)
                        ? Math.min(query.selectionStart, query.selectionEnd)
                        : Math.max(query.selectionStart, query.selectionEnd)
                    clearShiftSelection()
                    query.deselect()
                    query.cursorPosition = c
                } else {
                    var textLR = query.text
                    var posLR = query.cursorPosition
                    var nextLR = (event.key === Qt.Key_Left)
                        ? Math.max(0, posLR - 1)
                        : Math.min(textLR.length, posLR + 1)
                    moveCursorTo(nextLR, false)
                }
                cursorStrong = true
                cursorTimer.stop()
                event.accepted = true
                return true
            }
        }
        var shift = mods & Qt.ShiftModifier
        var cmd = mods & Qt.ControlModifier
        var alt = mods & Qt.AltModifier
        var text = query.text
        var pos = query.cursorPosition
        var newPos = pos
        if ((event.key === Qt.Key_Home || event.key === Qt.Key_End) && !(shift && !cmd && !alt)) {
            if (alt) return false
            newPos = (event.key === Qt.Key_Home)
                ? (cmd ? 0 : macLineStartPos(pos, text))
                : (cmd ? text.length : macLineEndPos(pos, text))
        } else if (shift && !cmd && !alt) {
            if (event.key === Qt.Key_Left || event.key === Qt.Key_Right) {
                // Persistent shiftAnchor/shiftHead: Qt keeps cursor at selectionEnd,
                // so cursorPosition cannot be the moving end when selecting left/up.
                var headH = (shiftHead >= 0) ? shiftHead : query.cursorPosition
                var newHead = (event.key === Qt.Key_Left)
                    ? Math.max(0, headH - 1)
                    : Math.min(text.length, headH + 1)
                cursorStrong = true
                cursorTimer.stop()
                applyShiftSelection(newHead)
                lastShiftHorizKey = event.key
                event.accepted = true
                return true
            } else if (event.key === Qt.Key_Up || event.key === Qt.Key_Down) {
                cursorStrong = true
                cursorTimer.stop()
                extendSelectionVertical(event.key === Qt.Key_Down)
                event.accepted = true
                return true
            } else if (event.key === Qt.Key_Home) {
                if (shiftAnchor < 0)
                    shiftAnchor = (query.selectionStart === query.selectionEnd)
                        ? pos : Math.max(query.selectionStart, query.selectionEnd)
                newPos = macLineStartPos((shiftHead >= 0) ? shiftHead : pos, text)
                cursorStrong = true
                cursorTimer.stop()
                applyShiftSelection(newPos)
                event.accepted = true
                return true
            } else if (event.key === Qt.Key_End) {
                if (shiftAnchor < 0)
                    shiftAnchor = (query.selectionStart === query.selectionEnd)
                        ? pos : Math.min(query.selectionStart, query.selectionEnd)
                newPos = macLineEndPos((shiftHead >= 0) ? shiftHead : pos, text)
                cursorStrong = true
                cursorTimer.stop()
                applyShiftSelection(newPos)
                event.accepted = true
                return true
            } else {
                return false
            }
        } else if (!cmd && !alt && !shift) {
            if (event.key === Qt.Key_Up || event.key === Qt.Key_Down) {
                cursorStrong = true
                cursorTimer.stop()
                moveCursorVertical(event.key === Qt.Key_Down)
                event.accepted = true
                return true
            }
            if (event.key === Qt.Key_Left || event.key === Qt.Key_Right) {
                if (query.selectionStart !== query.selectionEnd) {
                    var selLo = Math.min(query.selectionStart, query.selectionEnd)
                    var selHi = Math.max(query.selectionStart, query.selectionEnd)
                    newPos = (event.key === Qt.Key_Left) ? selLo : selHi
                    moveCursorTo(newPos, false, false)
                    cursorStrong = true
                    cursorTimer.stop()
                    event.accepted = true
                    return true
                }
            }
            return false
        } else if (event.key === Qt.Key_Left) {
            if (cmd && shift) newPos = macLineStartPos(pos, text)
            else if (cmd) newPos = lineStartPos(pos, text)
            else if (alt) newPos = wordLeftPos(selectionExtendFrom(Qt.Key_Left), text)
            else newPos = lineStartPos(pos, text)
        } else if (event.key === Qt.Key_Right) {
            if (cmd && shift) newPos = macLineEndPos(pos, text)
            else if (cmd) newPos = lineEndPos(pos, text)
            else if (alt) newPos = wordRightPos(selectionExtendFrom(Qt.Key_Right), text)
            else newPos = lineEndPos(pos, text)
        } else if (event.key === Qt.Key_Up) {
            if (cmd) newPos = 0
            else if (alt && shift) newPos = paragraphUpPos(selectionExtendFrom(Qt.Key_Up), text)
            else newPos = paragraphUpPos(pos, text)
        } else if (event.key === Qt.Key_Down) {
            if (cmd) newPos = text.length
            else if (alt && shift) newPos = paragraphDownPos(selectionExtendFrom(Qt.Key_Down), text)
            else newPos = paragraphDownPos(pos, text)
        } else {
            return false
        }
        cursorStrong = true
        cursorTimer.stop()
        if (shift)
            extendSelectionHorizontal(newPos)
        else
            moveCursorTo(newPos, false)
        event.accepted = true
        return true
    }

    function handleMacEditKeys(event) {
        if (mode != 1) return false
        var mods = event.modifiers
        var cmd = mods & Qt.ControlModifier
        if (cmd && !(mods & Qt.AltModifier) && event.key === Qt.Key_A) {
            var len = query.text.length
            query.select(0, len)
            cursorStrong = true
            cursorTimer.stop()
            event.accepted = true
            doc = query.text
            return true
        }
        if (event.key !== Qt.Key_Return || mods !== Qt.NoModifier) return false
        var text = query.text
        var pos = query.cursorPosition
        beginTextEdit()
        query.text = text.slice(0, pos) + "\n" + text.slice(pos)
        query.cursorPosition = pos + 1
        query.deselect()
        cursorStrong = true
        cursorTimer.stop()
        event.accepted = true
        doc = query.text
        return true
    }

    function handleMacBackspace(event) {
        if (mode != 1) return false
        if (event.key !== Qt.Key_Backspace && event.key !== Qt.Key_Delete) return false
        var mods = event.modifiers
        var cmd = mods & Qt.ControlModifier
        var alt = mods & Qt.AltModifier
        var text = query.text
        var pos = query.cursorPosition
        if (!cmd && !alt) {
            if (event.key === Qt.Key_Backspace || event.key === Qt.Key_Delete) {
                beginTextEdit()
                if (event.key === Qt.Key_Backspace) {
                    if (query.selectionStart !== query.selectionEnd) {
                        var a = Math.min(query.selectionStart, query.selectionEnd)
                        var b = Math.max(query.selectionStart, query.selectionEnd)
                        query.text = text.slice(0, a) + text.slice(b)
                        query.cursorPosition = a
                    } else if (pos > 0) {
                        query.text = text.slice(0, pos - 1) + text.slice(pos)
                        query.cursorPosition = pos - 1
                    }
                } else if (event.key === Qt.Key_Delete) {
                    if (query.selectionStart !== query.selectionEnd) {
                        var sa = Math.min(query.selectionStart, query.selectionEnd)
                        var sb = Math.max(query.selectionStart, query.selectionEnd)
                        query.text = text.slice(0, sa) + text.slice(sb)
                        query.cursorPosition = sa
                    } else if (pos < text.length) {
                        query.text = text.slice(0, pos) + text.slice(pos + 1)
                        query.cursorPosition = pos
                    }
                }
                query.deselect()
            }
            doc = query.text
            cursorStrong = true
            cursorTimer.stop()
            event.accepted = true
            return true
        }
        if (pos <= 0 && text.length === 0) {
            event.accepted = true
            return true
        }
        if (query.selectionStart !== query.selectionEnd) {
            beginTextEdit()
            var selA = Math.min(query.selectionStart, query.selectionEnd)
            var selB = Math.max(query.selectionStart, query.selectionEnd)
            query.text = text.slice(0, selA) + text.slice(selB)
            query.cursorPosition = selA
            query.deselect()
            cursorStrong = true
            cursorTimer.stop()
            event.accepted = true
            return true
        }
        var start = cmd ? deleteLineLeftPos(pos, text) : deleteWordLeftPos(pos, text)
        var end = pos
        if (start < end) {
            beginTextEdit()
            query.text = text.slice(0, start) + text.slice(end)
            query.cursorPosition = start
            query.deselect()
        }
        cursorStrong = true
        cursorTimer.stop()
        event.accepted = true
        return true
    }
