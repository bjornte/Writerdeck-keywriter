                    // 4 Settings
                    Item {
                        visible: lobbyPage === 3
                        anchors.fill: parent
                        Flickable {
                            anchors.fill: parent
                            contentWidth: width
                            contentHeight: setCol.height
                            clip: true
                            Column {
                                id: setCol
                                width: parent.width
                                spacing: lobby.contentSpacing
                                Text {
                                    text: lobbySettingsMode === "confirm-exit"
                                          ? "Stop Writerdeck? Enter=yes  Esc=no"
                                          : "Settings"
                                    font.pointSize: 14
                                    font.family: "Noto Sans"
                                    color: "black"
                                    width: parent.width
                                }

                                Column {
                                    width: parent.width
                                    spacing: lobby.contentSpacing
                                    visible: lobbySettingsMode === ""

                                    Text {
                                        text: "\nReading font"
                                        font.pointSize: 12
                                        font.family: "Noto Sans"
                                        color: "black"
                                        width: parent.width
                                    }
                                    Grid {
                                        id: fontGrid
                                        width: parent.width
                                        columns: 2
                                        rowSpacing: lobby.tabSpacing
                                        columnSpacing: lobby.tabSpacing
                                        property var fontOptions: [
                                            { id: "Inter", label: "Inter" },
                                            { id: "Literata", label: "Literata" },
                                            { id: "EB Garamond", label: "EB Garamond" },
                                            { id: "DejaVu Sans", label: "DejaVu Sans" }
                                        ]
                                        Repeater {
                                            model: fontGrid.fontOptions
                                            delegate: Rectangle {
                                                width: (fontGrid.width - lobby.tabSpacing) / 2
                                                height: lobby.actionBtnHeight
                                                radius: 6
                                                property bool selected: readFont === modelData.id
                                                color: selected ? "#e8e8e8" : "#f0f0f0"
                                                border.color: selected ? "black" : "#bbb"
                                                border.width: selected ? 2 : 1
                                                Text {
                                                    anchors.centerIn: parent
                                                    text: modelData.label
                                                    font.family: "Noto Sans"
                                                    font.pointSize: 11
                                                }
                                                MouseArea {
                                                    anchors.fill: parent
                                                    onClicked: writerdeck.setReadFont(modelData.id)
                                                }
                                            }
                                        }
                                    }

                                    Text {
                                        text: "\nPrivate notes"
                                        font.pointSize: 12
                                        font.family: "Noto Sans"
                                        color: "black"
                                        width: parent.width
                                    }
                                    Text {
                                        text: lobbyEncryptionEnabled
                                            ? "On — encrypted notes require PIN to open, read, or edit"
                                            : "Off — optional encryption with a separate 6-digit PIN. Recovery via GitHub secret/pin when sync is on."
                                        font.pointSize: 10
                                        font.family: "Noto Sans"
                                        color: "#666666"
                                        width: parent.width
                                        wrapMode: Text.WordWrap
                                    }
                                    Row {
                                        width: parent.width
                                        spacing: lobby.tabSpacing
                                        visible: !lobbyEncryptionEnabled
                                        Rectangle {
                                            width: (parent.width - lobby.tabSpacing) / 2
                                            height: lobby.actionBtnHeight
                                            radius: 6
                                            color: "#f0f0f0"
                                            border.color: "#bbb"
                                            border.width: 1
                                            Text {
                                                anchors.centerIn: parent
                                                text: "Enable"
                                                font.family: "Noto Sans"
                                                font.pointSize: 11
                                            }
                                            MouseArea {
                                                anchors.fill: parent
                                                onClicked: root.vaultBeginSetup()
                                            }
                                        }
                                    }
                                    Row {
                                        width: parent.width
                                        spacing: lobby.tabSpacing
                                        visible: lobbyEncryptionEnabled
                                        Rectangle {
                                            width: parent.width
                                            height: lobby.actionBtnHeight
                                            radius: 6
                                            color: "#f0f0f0"
                                            border.color: "#bbb"
                                            border.width: 1
                                            Text {
                                                anchors.centerIn: parent
                                                text: "Change PIN"
                                                font.family: "Noto Sans"
                                                font.pointSize: 11
                                            }
                                            MouseArea {
                                                anchors.fill: parent
                                                onClicked: root.vaultBeginChangePIN()
                                            }
                                        }
                                    }

                                    Text {
                                        text: "\nPIN for phone pairing"
                                        font.pointSize: 12
                                        font.family: "Noto Sans"
                                        color: "black"
                                        width: parent.width
                                    }
                                    Text {
                                        text: "Adding a PIN ensures that only intended devices can access your notes"
                                        font.pointSize: 10
                                        font.family: "Noto Sans"
                                        color: "#666666"
                                        width: parent.width
                                        wrapMode: Text.WordWrap
                                    }
                                    Row {
                                        id: pinDigitsRow
                                        width: parent.width
                                        spacing: lobby.tabSpacing
                                        Repeater {
                                            model: [
                                                { id: "6", label: "6 digits" },
                                                { id: "4", label: "4 digits" },
                                                { id: "none", label: "No PIN", warn: "Anyone on Wi-Fi can read and edit notes" }
                                            ]
                                            delegate: Rectangle {
                                                // Same height for all three — No PIN warn line sets the size.
                                                width: (pinDigitsRow.width - lobby.tabSpacing * 2) / 3
                                                height: lobby.actionBtnHeight + 36
                                                radius: 6
                                                property bool selected: lobbyPinDigits === modelData.id
                                                color: selected ? "#e8e8e8" : "#f0f0f0"
                                                border.color: selected ? "black" : "#bbb"
                                                border.width: selected ? 2 : 1
                                                Column {
                                                    anchors.centerIn: parent
                                                    width: parent.width - 12
                                                    spacing: 2
                                                    Text {
                                                        anchors.horizontalCenter: parent.horizontalCenter
                                                        text: modelData.label
                                                        font.family: "Noto Sans"
                                                        font.pointSize: 11
                                                        color: "black"
                                                    }
                                                    Text {
                                                        visible: !!modelData.warn
                                                        anchors.horizontalCenter: parent.horizontalCenter
                                                        text: modelData.warn || ""
                                                        font.family: "Noto Sans"
                                                        font.pointSize: 8
                                                        color: "#666666"
                                                        horizontalAlignment: Text.AlignHCenter
                                                        wrapMode: Text.WordWrap
                                                        width: parent.width
                                                    }
                                                }
                                                MouseArea {
                                                    anchors.fill: parent
                                                    onClicked: writerdeck.setPinDigits(modelData.id)
                                                }
                                            }
                                        }
                                    }

                                    Text {
                                        text: "\nDisplay rotation"
                                        font.pointSize: 12
                                        font.family: "Noto Sans"
                                        color: "black"
                                        width: parent.width
                                    }
                                    Text {
                                        text: "Ctrl-R or Ctrl+arrows also rotate."
                                        font.pointSize: 10
                                        font.family: "Noto Sans"
                                        color: "#666666"
                                        width: parent.width
                                        wrapMode: Text.WordWrap
                                    }
                                    Row {
                                        id: rotationRow
                                        width: parent.width
                                        spacing: lobby.tabSpacing
                                        Repeater {
                                            model: [
                                                { deg: 0, label: "0\u00B0" },
                                                { deg: 90, label: "90\u00B0" },
                                                { deg: 180, label: "180\u00B0" },
                                                { deg: 270, label: "270\u00B0" }
                                            ]
                                            delegate: Rectangle {
                                                width: (rotationRow.width - lobby.tabSpacing * 3) / 4
                                                height: lobby.actionBtnHeight
                                                radius: 6
                                                property bool selected: root.rotation === modelData.deg
                                                color: selected ? "#e8e8e8" : "#f0f0f0"
                                                border.color: selected ? "black" : "#bbb"
                                                border.width: selected ? 2 : 1
                                                Text {
                                                    anchors.centerIn: parent
                                                    text: modelData.label
                                                    font.family: "Noto Sans"
                                                    font.pointSize: 12
                                                }
                                                MouseArea {
                                                    anchors.fill: parent
                                                    onClicked: root.setScreenRotation(modelData.deg)
                                                }
                                            }
                                        }
                                    }

                                    Text {
                                        text: "\nService"
                                        font.pointSize: 12
                                        font.family: "Noto Sans"
                                        color: "black"
                                        width: parent.width
                                    }
                                    Text {
                                        text: "Stop Writerdeck and return the tablet to the stock reMarkable UI. Reconnect later via SSH or reboot."
                                        font.pointSize: 10
                                        font.family: "Noto Sans"
                                        color: "#666666"
                                        width: parent.width
                                        wrapMode: Text.WordWrap
                                    }
                                    Rectangle {
                                        width: parent.width
                                        height: lobby.actionBtnHeight
                                        radius: 6
                                        color: "#f0f0f0"
                                        border.color: "#bbb"
                                        border.width: 1
                                        Text {
                                            anchors.centerIn: parent
                                            text: "Exit Writerdeck"
                                            font.family: "Noto Sans"
                                            font.pointSize: 12
                                        }
                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: root.lobbySettingsBeginExit()
                                        }
                                    }
                                }
                            }
                        }
                    }
