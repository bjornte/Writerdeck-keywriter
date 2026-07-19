        // Shared Lobby dialog chrome: grey scrim + white floating box (black type).
        // Kind comes from lobbyShowNoKeyboard / lobbyFilesMode — keep content in one piece.
        Rectangle {
            id: lobbyDialogScrim
            anchors.fill: parent
            color: "#dddddd"
            visible: root.lobbyDialogIsOpen()
            z: 24

            Rectangle {
                id: lobbyDialogBox
                anchors.centerIn: parent
                width: Math.min(parent.width * 0.85, parent.width - 48)
                height: lobbyDialogCol.height + 48
                color: "white"
                border.color: "black"
                border.width: lobby.btnBorderSelected
                radius: 8

                Column {
                    id: lobbyDialogCol
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: 24
                    width: parent.width - 48
                    spacing: 16

                    Text {
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        font.family: "Noto Sans"
                        font.pointSize: 16
                        color: "black"
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
                        color: "black"
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
                        color: "black"
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
                        color: "black"
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
                        color: "black"
                        text: "USB: plug in with an OTG cable.\n\nBluetooth: pair to your phone, then open the address below (or scan the code)."
                    }

                    Text {
                        width: parent.width
                        visible: lobbyShowNoKeyboard
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        font.family: "Noto Mono"
                        font.pointSize: 13
                        color: "black"
                        text: root.lobbyPhoneUrl()
                    }

                    Text {
                        width: parent.width
                        visible: lobbyShowNoKeyboard && lobbyPIN !== ""
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        font.family: "Noto Sans"
                        font.pointSize: 12
                        color: "black"
                        text: "PIN: " + lobbyPIN
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

                    // ---- buttons ----
                    Row {
                        width: parent.width
                        spacing: lobby.tabSpacing
                        visible: !lobbyShowNoKeyboard && lobbyFilesMode === "confirm-delete"

                        Rectangle {
                            width: (parent.width - lobby.tabSpacing) / 2
                            height: lobby.actionBtnHeight
                            radius: 6
                            color: "white"
                            border.color: "black"
                            border.width: lobby.btnBorder
                            Text {
                                anchors.centerIn: parent
                                text: "Cancel"
                                font.family: "Noto Sans"
                                font.pointSize: 12
                                color: "black"
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
                            radius: 6
                            color: "white"
                            border.color: "black"
                            border.width: lobby.btnBorderSelected
                            Text {
                                anchors.centerIn: parent
                                text: "Delete"
                                font.family: "Noto Sans"
                                font.pointSize: 12
                                color: "black"
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
                        visible: !lobbyShowNoKeyboard
                                 && (lobbyFilesMode === "new"
                                     || lobbyFilesMode === "rename"
                                     || lobbyFilesMode === "new-encrypted")

                        Rectangle {
                            width: (parent.width - lobby.tabSpacing) / 2
                            height: lobby.actionBtnHeight
                            radius: 6
                            color: "white"
                            border.color: "black"
                            border.width: lobby.btnBorder
                            Text {
                                anchors.centerIn: parent
                                text: "Cancel"
                                font.family: "Noto Sans"
                                font.pointSize: 12
                                color: "black"
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
                            radius: 6
                            color: "white"
                            border.color: "black"
                            border.width: lobby.btnBorderSelected
                            Text {
                                anchors.centerIn: parent
                                text: lobbyFilesMode === "rename" ? "Rename" : "Create"
                                font.family: "Noto Sans"
                                font.pointSize: 12
                                color: "black"
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
                            radius: 6
                            color: "white"
                            border.color: "black"
                            border.width: lobby.btnBorder
                            Text {
                                anchors.centerIn: parent
                                text: "Cancel"
                                font.family: "Noto Sans"
                                font.pointSize: 12
                                color: "black"
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
