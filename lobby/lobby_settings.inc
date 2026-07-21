                    // 4 Settings
                    Item {
                        visible: lobbyPage === 3
                        anchors.fill: parent
                        Flickable {
                            id: settingsFlick
                            anchors.fill: parent
                            // Landscape only: empty right strip so a finger can flick
                            // without hitting full-width buttons (visual.settingsLandscapeScrollGutter).
                            property int scrollGutter: root.rotation % 180 != 0
                                                       ? lobby.settingsLandscapeScrollGutter : 0
                            contentWidth: width
                            contentHeight: setCol.height
                            clip: true
                            focus: false
                            interactive: true
                            flickableDirection: Flickable.VerticalFlick
                            Column {
                                id: setCol
                                width: parent.width - settingsFlick.scrollGutter
                                spacing: lobby.contentSpacing
                                Text {
                                    text: lobbySettingsMode === "confirm-exit"
                                          ? lobbyUi.str("settings.confirmExit")
                                          : lobbyUi.str("settings.title")
                                    font.pointSize: lobby.rowPointSize
                                    font.family: "Noto Sans"
                                    color: lobby.textColor
                                    width: parent.width
                                }

                                Column {
                                    width: parent.width
                                    spacing: lobby.contentSpacing
                                    visible: lobbySettingsMode === ""

                                    Text {
                                        text: lobbyUi.str("settings.fontSection")
                                        font.pointSize: lobby.sectionPointSize
                                        font.family: "Noto Sans"
                                        color: lobby.textColor
                                        width: parent.width
                                    }
                                    Text {
                                        text: lobbyUi.str("settings.fontHelp")
                                        font.pointSize: lobby.helpPointSize
                                        font.family: "Noto Sans"
                                        color: lobby.textColor
                                        width: parent.width
                                        wrapMode: Text.WordWrap
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
                                                radius: lobby.btnRadius
                                                property bool selected: readFont === modelData.id
                                                color: selected ? lobby.btnFillSelected : lobby.btnFill
                                                border.color: lobby.borderColor
                                                border.width: selected ? lobby.btnBorderSelected : lobby.btnBorder
                                                Text {
                                                    anchors.centerIn: parent
                                                    text: modelData.label
                                                    font.family: modelData.id
                                                    font.pointSize: lobby.labelPointSize
                                                    color: lobby.textColor
                                                }
                                                MouseArea {
                                                    anchors.fill: parent
                                                    onClicked: {
                                                        writerdeck.setReadFont(modelData.id)
                                                        root.lobbyKeepFocus()
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    Text {
                                        text: lobbyUi.str("settings.privateSection")
                                        font.pointSize: lobby.sectionPointSize
                                        font.family: "Noto Sans"
                                        color: lobby.textColor
                                        width: parent.width
                                    }
                                    Text {
                                        text: lobbyEncryptionEnabled
                                            ? lobbyUi.str("settings.privateOn")
                                            : lobbyUi.str("settings.privateOff")
                                        font.pointSize: 10
                                        font.family: "Noto Sans"
                                        color: lobby.textColor
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
                                            radius: lobby.btnRadius
                                            color: lobby.btnFill
                                            border.color: lobby.borderColor
                                            border.width: lobby.btnBorder
                                            Loader {
                                                anchors.fill: parent
                                                property string labelText: lobbyUi.str("settings.enable")
                                                property string shortcutKey: lobbyUi.shortcutBadge("settings.enableVault")
                                                property int pointSize: lobby.labelPointSize
                                                sourceComponent: lobbyBtnLabelComp
                                            }
                                            MouseArea {
                                                anchors.fill: parent
                                                onClicked: {
                                                    root.vaultBeginSetup()
                                                    root.lobbyKeepFocus()
                                                }
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
                                            radius: lobby.btnRadius
                                            color: lobby.btnFill
                                            border.color: lobby.borderColor
                                            border.width: lobby.btnBorder
                                            Loader {
                                                anchors.fill: parent
                                                property string labelText: lobbyUi.str("settings.changePin")
                                                property string shortcutKey: lobbyUi.shortcutBadge("settings.changePin")
                                                property int pointSize: lobby.labelPointSize
                                                sourceComponent: lobbyBtnLabelComp
                                            }
                                            MouseArea {
                                                anchors.fill: parent
                                                onClicked: {
                                                    root.vaultBeginChangePIN()
                                                    root.lobbyKeepFocus()
                                                }
                                            }
                                        }
                                    }

                                    Text {
                                        text: lobbyUi.str("settings.pinSection")
                                        font.pointSize: lobby.sectionPointSize
                                        font.family: "Noto Sans"
                                        color: lobby.textColor
                                        width: parent.width
                                    }
                                    Text {
                                        text: lobbyUi.str("settings.pinHelp")
                                        font.pointSize: lobby.helpPointSize
                                        font.family: "Noto Sans"
                                        color: lobby.textColor
                                        width: parent.width
                                        wrapMode: Text.WordWrap
                                    }
                                    Row {
                                        id: pinDigitsRow
                                        width: parent.width
                                        spacing: lobby.tabSpacing
                                        Repeater {
                                            model: [
                                                { id: "6", labelKey: "settings.pin6" },
                                                { id: "4", labelKey: "settings.pin4" },
                                                { id: "none", labelKey: "settings.pinNone", warnKey: "settings.pinNoneWarn" }
                                            ]
                                            delegate: Rectangle {
                                                // Same height for all three — No PIN warn line sets the size.
                                                width: (pinDigitsRow.width - lobby.tabSpacing * 2) / 3
                                                height: lobby.actionBtnHeight + 36
                                                radius: lobby.btnRadius
                                                property bool selected: lobbyPinDigits === modelData.id
                                                color: selected ? lobby.btnFillSelected : lobby.btnFill
                                                border.color: lobby.borderColor
                                                border.width: selected ? lobby.btnBorderSelected : lobby.btnBorder
                                                Column {
                                                    anchors.centerIn: parent
                                                    width: parent.width - 12
                                                    spacing: 2
                                                    Text {
                                                        anchors.horizontalCenter: parent.horizontalCenter
                                                        text: lobbyUi.str(modelData.labelKey)
                                                        font.family: "Noto Sans"
                                                        font.pointSize: lobby.labelPointSize
                                                        color: lobby.textColor
                                                    }
                                                    Text {
                                                        visible: !!modelData.warnKey
                                                        anchors.horizontalCenter: parent.horizontalCenter
                                                        text: modelData.warnKey ? lobbyUi.str(modelData.warnKey) : ""
                                                        font.family: "Noto Sans"
                                                        font.pointSize: 8
                                                        color: lobby.textColor
                                                        horizontalAlignment: Text.AlignHCenter
                                                        wrapMode: Text.WordWrap
                                                        width: parent.width
                                                    }
                                                }
                                                MouseArea {
                                                    anchors.fill: parent
                                                    onClicked: {
                                                        writerdeck.setPinDigits(modelData.id)
                                                        root.lobbyKeepFocus()
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    Text {
                                        text: lobbyUi.str("settings.rotationSection")
                                        font.pointSize: lobby.sectionPointSize
                                        font.family: "Noto Sans"
                                        color: lobby.textColor
                                        width: parent.width
                                    }
                                    Text {
                                        text: lobbyUi.str("settings.rotationHelp")
                                        font.pointSize: lobby.helpPointSize
                                        font.family: "Noto Sans"
                                        color: lobby.textColor
                                        width: parent.width
                                        wrapMode: Text.WordWrap
                                    }
                                    Row {
                                        id: rotationRow
                                        width: parent.width
                                        spacing: lobby.tabSpacing
                                        Repeater {
                                            model: [
                                                { deg: 0, labelKey: "settings.rot0" },
                                                { deg: 90, labelKey: "settings.rot90" },
                                                { deg: 180, labelKey: "settings.rot180" },
                                                { deg: 270, labelKey: "settings.rot270" }
                                            ]
                                            delegate: Rectangle {
                                                width: (rotationRow.width - lobby.tabSpacing * 3) / 4
                                                height: lobby.actionBtnHeight
                                                radius: lobby.btnRadius
                                                property bool selected: root.rotation === modelData.deg
                                                color: selected ? lobby.btnFillSelected : lobby.btnFill
                                                border.color: lobby.borderColor
                                                border.width: selected ? lobby.btnBorderSelected : lobby.btnBorder
                                                Text {
                                                    anchors.centerIn: parent
                                                    text: lobbyUi.str(modelData.labelKey)
                                                    font.family: "Noto Sans"
                                                    font.pointSize: lobby.sectionPointSize
                                                    color: lobby.textColor
                                                }
                                                MouseArea {
                                                    anchors.fill: parent
                                                    onClicked: {
                                                        root.setScreenRotation(modelData.deg)
                                                        root.lobbyKeepFocus()
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    Text {
                                        text: lobbyUi.str("settings.serviceSection")
                                        font.pointSize: lobby.sectionPointSize
                                        font.family: "Noto Sans"
                                        color: lobby.textColor
                                        width: parent.width
                                    }
                                    Text {
                                        text: lobbyUi.str("settings.serviceHelp")
                                        font.pointSize: lobby.helpPointSize
                                        font.family: "Noto Sans"
                                        color: lobby.textColor
                                        width: parent.width
                                        wrapMode: Text.WordWrap
                                    }
                                    Rectangle {
                                        width: parent.width
                                        height: lobby.actionBtnHeight
                                        radius: lobby.btnRadius
                                        color: lobby.btnFill
                                        border.color: lobby.borderColor
                                        border.width: lobby.btnBorder
                                        Loader {
                                            anchors.fill: parent
                                            property string labelText: lobbyUi.str("settings.exit")
                                            property string shortcutKey: lobbyUi.shortcutBadge("settings.exit")
                                            property int pointSize: 12
                                            sourceComponent: lobbyBtnLabelComp
                                        }
                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                root.lobbySettingsBeginExit()
                                                root.lobbyKeepFocus()
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
