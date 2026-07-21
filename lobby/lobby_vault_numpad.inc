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
