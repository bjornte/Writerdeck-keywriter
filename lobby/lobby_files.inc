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
                                color: "white"
                                border.color: "black"
                                border.width: 2
                                radius: 8
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
                                        color: "black"
                                        text: lobbyVaultError
                                    }

                                    Text {
                                        width: parent.width
                                        horizontalAlignment: Text.AlignHCenter
                                        font.family: "Noto Sans"
                                        font.pointSize: 11
                                        color: "#555555"
                                        text: "Tap to dismiss"
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
                                height: visible ? 48 : 0
                                spacing: lobby.tabSpacing
                                visible: lobbyFilesMode === "" && lobbyNotesModel.count > lobbyFilesPageSize

                                property bool canPrev: root.lobbyFilesPageIndex() > 0
                                property bool canNext: root.lobbyFilesPageIndex() + 1 < root.lobbyFilesPageCount()

                                Rectangle {
                                    width: (lobbyFilesPageStrip.width - lobby.tabSpacing * 2) / 4
                                    height: 48
                                    radius: 6
                                    color: "#f0f0f0"
                                    border.color: lobbyFilesPageStrip.canPrev ? "#bbb" : "#ddd"
                                    border.width: 1
                                    opacity: lobbyFilesPageStrip.canPrev ? 1 : 0.45
                                    Text {
                                        anchors.centerIn: parent
                                        text: "Prev"
                                        font.family: "Noto Sans"
                                        font.pointSize: 11
                                        color: "black"
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
                                    height: 48
                                    Text {
                                        anchors.centerIn: parent
                                        text: "Page " + (root.lobbyFilesPageIndex() + 1)
                                              + "/" + root.lobbyFilesPageCount()
                                        font.family: "Noto Sans"
                                        font.pointSize: 11
                                        color: "black"
                                    }
                                }
                                Rectangle {
                                    width: (lobbyFilesPageStrip.width - lobby.tabSpacing * 2) / 4
                                    height: 48
                                    radius: 6
                                    color: "#f0f0f0"
                                    border.color: lobbyFilesPageStrip.canNext ? "#bbb" : "#ddd"
                                    border.width: 1
                                    opacity: lobbyFilesPageStrip.canNext ? 1 : 0.45
                                    Text {
                                        anchors.centerIn: parent
                                        text: "Next"
                                        font.family: "Noto Sans"
                                        font.pointSize: 11
                                        color: "black"
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
                                color: "#999"
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
                                        { label: "New", key: "n" },
                                        { label: "Edit", key: "" },
                                        { label: "Read", key: "v" },
                                        { label: "Rename", key: "r" },
                                        { label: "Delete", key: "d" },
                                        { label: "Download", key: "g" }
                                    ]
                                    delegate: Rectangle {
                                        width: (lobbyFilesBar.width - lobby.tabSpacing * 5) / 6
                                        height: lobby.actionBtnHeight
                                        radius: 6
                                        color: "#f0f0f0"
                                        border.color: "#bbb"
                                        border.width: 1
                                        Loader {
                                            anchors.fill: parent
                                            property string labelText: modelData.label
                                            property string shortcutKey: modelData.key
                                            property int pointSize: 10
                                            sourceComponent: lobbyBtnLabelComp
                                        }
                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                if (modelData.label === "New") root.lobbyFilesBeginNew()
                                                else if (modelData.label === "Edit") root.lobbyOpenSelected()
                                                else if (modelData.label === "Read") root.lobbyReadSelected()
                                                else if (modelData.label === "Rename") root.lobbyFilesBeginRename()
                                                else if (modelData.label === "Delete") root.lobbyFilesBeginDelete()
                                                else if (modelData.label === "Download") root.lobbyFilesBeginDownload()
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
                                    radius: 6
                                    color: "#f0f0f0"
                                    border.color: "#bbb"
                                    border.width: 1
                                    Loader {
                                        anchors.fill: parent
                                        property string labelText: "Encrypt"
                                        property string shortcutKey: "x"
                                        property int pointSize: 11
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
                                    radius: 6
                                    color: "#f0f0f0"
                                    border.color: "#bbb"
                                    border.width: 1
                                    Loader {
                                        anchors.fill: parent
                                        property string labelText: "New encrypted"
                                        property string shortcutKey: "e"
                                        property int pointSize: 11
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
                                    radius: 6
                                    color: "#f0f0f0"
                                    border.color: "#bbb"
                                    border.width: 1
                                    Loader {
                                        anchors.fill: parent
                                        property string labelText: "Decrypt"
                                        property string shortcutKey: "y"
                                        property int pointSize: 11
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
                                            anchors.leftMargin: 8
                                            anchors.right: parent.right
                                            anchors.rightMargin: 8
                                            anchors.verticalCenter: parent.verticalCenter
                                            text: (noteIndex === lobbyFilesIndex ? "\u25B6  " : "   ")
                                                  + lobbyFilesStripSuffix(noteRow ? noteRow.name : "")
                                                  + (noteRow && noteRow.encrypted ? " [private]" : "")
                                            font.family: "Noto Sans"
                                            font.pointSize: 14
                                            color: "black"
                                            elide: Text.ElideRight
                                        }
                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                if (noteIndex === lobbyFilesIndex)
                                                    root.lobbyOpenSelected()
                                                else
                                                    root.lobbyFilesSetIndex(noteIndex)
                                                root.lobbyKeepFocus()
                                            }
                                            onDoubleClicked: root.lobbyOpenSelected()
                                        }
                                    }
                                }
                            }
                        }
                    }
