                    // 1 Files
                    Item {
                        visible: lobbyPage === 0
                        anchors.fill: parent

                        Column {
                            anchors.fill: parent
                            spacing: lobby.contentSpacing

                            Text {
                                visible: lobbyFilesMode === "new" || lobbyFilesMode === "rename"
                                         || lobbyFilesMode === "confirm-delete"
                                         || lobbyFilesMode === "new-encrypted"
                                text: lobbyFilesMode === "new" ? "New note name:"
                                     : lobbyFilesMode === "rename" ? "Rename to:"
                                     : lobbyFilesMode === "new-encrypted" ? "New encrypted note name:"
                                     : lobbyFilesMode === "confirm-delete" ? "Delete this note?"
                                     : ""
                                font.family: "Noto Sans"
                                font.pointSize: 13
                                color: "black"
                                width: parent.width
                            }

                            Text {
                                visible: lobbyFilesMode === "new" || lobbyFilesMode === "rename"
                                         || lobbyFilesMode === "new-encrypted"
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

                            // Fixed page of rows — no flick scroll (e-ink).
                            Item {
                                id: lobbyFilesList
                                width: parent.width
                                height: {
                                    var reserved = lobbyFilesBar.height + lobby.contentSpacing * 2
                                    if (lobbyFilesMode === "confirm-delete")
                                        reserved = lobbyFilesDeleteBar.height + lobby.contentSpacing * 2
                                    if (lobbyEncryptionEnabled && lobbyFilesMode === "" && lobbyNotesModel.count > 0)
                                        reserved += lobbyFilesVaultBar.height + lobby.contentSpacing
                                    if (lobbyFilesPageLabel.visible)
                                        reserved += lobbyFilesPageLabel.height + lobby.contentSpacing
                                    return Math.max(lobby.rowHeight * 2, parent.height - reserved)
                                }
                                visible: lobbyFilesMode === "" || lobbyFilesMode === "confirm-delete"
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

                            Text {
                                id: lobbyFilesPageLabel
                                width: parent.width
                                visible: lobbyFilesMode === "" && lobbyNotesModel.count > lobbyFilesPageSize
                                text: "Page " + (root.lobbyFilesPageIndex() + 1)
                                      + "/" + root.lobbyFilesPageCount()
                                      + "  Up/Down  PgUp/PgDn"
                                font.family: "Noto Mono"
                                font.pointSize: 9
                                color: "#888888"
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

                            Row {
                                id: lobbyFilesDeleteBar
                                width: parent.width
                                height: lobby.actionBtnHeight + 8
                                spacing: lobby.tabSpacing
                                visible: lobbyFilesMode === "confirm-delete"

                                Rectangle {
                                    width: (lobbyFilesDeleteBar.width - lobby.tabSpacing) / 2
                                    height: lobby.actionBtnHeight
                                    radius: 6
                                    color: "#f0f0f0"
                                    border.color: "#bbb"
                                    border.width: 1
                                    Text {
                                        anchors.centerIn: parent
                                        text: "Cancel"
                                        font.family: "Noto Sans"
                                        font.pointSize: 11
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
                                    width: (lobbyFilesDeleteBar.width - lobby.tabSpacing) / 2
                                    height: lobby.actionBtnHeight
                                    radius: 6
                                    color: "#f0f0f0"
                                    border.color: "black"
                                    border.width: 2
                                    Text {
                                        anchors.centerIn: parent
                                        text: "Delete"
                                        font.family: "Noto Sans"
                                        font.pointSize: 11
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
                        }
                    }
