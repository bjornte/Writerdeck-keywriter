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
