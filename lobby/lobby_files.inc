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
                                // Imperative currentIndex only — a binding breaks when ListView
                                // clears the model (currentIndex=-1) and then ignores lobbyFilesIndex.
                                spacing: 2
                                visible: lobbyFilesMode === "" || lobbyFilesMode === "confirm-delete"
                                // Selection marker is the big triangle — no full-row fill (less e-ink redraw).
                                delegate: Item {
                                    width: lobbyFilesList.width
                                    height: lobby.rowHeight
                                    Row {
                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.left: parent.left
                                        anchors.leftMargin: 4
                                        anchors.right: parent.right
                                        anchors.rightMargin: 8
                                        spacing: 10
                                        Text {
                                            width: 36
                                            horizontalAlignment: Text.AlignHCenter
                                            text: index === lobbyFilesIndex ? "\u25B6" : " "
                                            font.family: "Noto Sans"
                                            font.pointSize: 22
                                            color: "black"
                                        }
                                        Text {
                                            width: parent.width - 46
                                            text: lobbyFilesStripSuffix(model.name) + (model.encrypted ? " [private]" : "")
                                            font.family: "Noto Mono"
                                            font.pointSize: 11
                                            color: "black"
                                            elide: Text.ElideRight
                                        }
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
                                Connections {
                                    target: root
                                    function onLobbyFilesIndexChanged() {
                                        if (lobbyFilesList.currentIndex !== lobbyFilesIndex
                                                && lobbyFilesIndex >= 0
                                                && lobbyFilesIndex < lobbyNotesModel.count) {
                                            lobbyFilesList.currentIndex = lobbyFilesIndex
                                            lobbyFilesList.positionViewAtIndex(lobbyFilesIndex, ListView.Contain)
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
