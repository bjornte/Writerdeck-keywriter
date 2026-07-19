                    // 1 Files — bottom chrome is anchored; the list fills what is left.
                    // Do not put feedback and bars in one Column with a hand-reserved
                    // list height: extra lines then push the buttons off the screen.
                    Item {
                        visible: lobbyPage === 0
                        anchors.fill: parent

                        Rectangle {
                            id: lobbyFilesFeedbackBox
                            anchors.top: parent.top
                            anchors.left: parent.left
                            anchors.right: parent.right
                            visible: lobbyVaultError !== "" && lobbyFilesMode === ""
                            height: visible ? (lobbyFilesFeedbackCol.height + 24) : 0
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

                        Column {
                            id: lobbyFilesBottom
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            spacing: lobby.contentSpacing

                            Row {
                                id: lobbyFilesVaultBar
                                width: parent.width
                                height: lobby.actionBtnHeight + 4
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
                                    Text {
                                        anchors.centerIn: parent
                                        text: "Encrypt"
                                        font.family: "Noto Sans"
                                        font.pointSize: 11
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
                                    Text {
                                        anchors.centerIn: parent
                                        text: "New encrypted"
                                        font.family: "Noto Sans"
                                        font.pointSize: 11
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
                                    Text {
                                        anchors.centerIn: parent
                                        text: "Decrypt"
                                        font.family: "Noto Sans"
                                        font.pointSize: 11
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
                                                root.lobbyKeepFocus()
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        Text {
                            id: lobbyFilesPageLabel
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: lobbyFilesBottom.top
                            anchors.bottomMargin: lobby.contentSpacing
                            visible: lobbyFilesMode === "" && lobbyNotesModel.count > lobbyFilesPageSize
                            height: visible ? implicitHeight : 0
                            text: "Page " + (root.lobbyFilesPageIndex() + 1)
                                  + "/" + root.lobbyFilesPageCount()
                                  + "  Up/Down  PgUp/PgDn"
                            font.family: "Noto Mono"
                            font.pointSize: 9
                            color: "#888888"
                        }

                        // Fixed page of rows — no flick scroll (e-ink).
                        Item {
                            id: lobbyFilesList
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.top: lobbyFilesFeedbackBox.bottom
                            anchors.topMargin: lobbyFilesFeedbackBox.visible ? lobby.contentSpacing : 0
                            anchors.bottom: lobbyFilesPageLabel.visible
                                            ? lobbyFilesPageLabel.top
                                            : lobbyFilesBottom.top
                            anchors.bottomMargin: lobby.contentSpacing
                            visible: lobbyFilesMode === ""
                            clip: true

                            property int pageSize: Math.max(1, Math.floor(height / lobby.rowHeight))
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
                                                    lobbyFilesIndex = noteIndex
                                                root.lobbyKeepFocus()
                                            }
                                            onDoubleClicked: root.lobbyOpenSelected()
                                        }
                                    }
                                }
                            }
                        }
                    }
