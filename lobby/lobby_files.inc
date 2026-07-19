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

                        // ---- footer: page label + action bars (pinned to bottom) ----
                        Column {
                            id: lobbyFilesFooter
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            spacing: lobby.contentSpacing
                            z: 2

                            Item {
                                width: parent.width
                                height: lobbyFilesPageLabel.visible ? lobbyFilesPageLabel.implicitHeight : 0

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
                            }

                            Row {
                                id: lobbyFilesBar
                                width: parent.width
                                height: visible ? (lobby.actionBtnHeight + 8) : 0
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
