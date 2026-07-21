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
                                    text: root.lobbyT("settings.title")
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
                                        text: root.lobbyT("settings.languageSection")
                                        font.pointSize: lobby.sectionPointSize
                                        font.family: "Noto Sans"
                                        color: lobby.textColor
                                        width: parent.width
                                    }
                                    Text {
                                        text: root.lobbyT("settings.languageHelp")
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
                                            property string labelText: root.lobbySettingsLanguageLabel()
                                            property string shortcutKey: lobbyUi.shortcutBadge("settings.language")
                                            property int pointSize: lobby.labelPointSize
                                            sourceComponent: lobbyBtnLabelComp
                                        }
                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                root.lobbySettingsBeginPickLanguage()
                                                root.lobbyKeepFocus()
                                            }
                                        }
                                    }

                                    Text {
                                        text: root.lobbyT("settings.fontSection")
                                        font.pointSize: lobby.sectionPointSize
                                        font.family: "Noto Sans"
                                        color: lobby.textColor
                                        width: parent.width
                                    }
                                    Text {
                                        text: root.lobbyT("settings.fontHelp")
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
                                            property string labelText: readFont
                                            property string shortcutKey: lobbyUi.shortcutBadge("settings.font")
                                            property int pointSize: lobby.labelPointSize
                                            sourceComponent: lobbyBtnLabelComp
                                        }
                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                root.lobbySettingsBeginPickFont()
                                                root.lobbyKeepFocus()
                                            }
                                        }
                                    }

                                    Text {
                                        text: root.lobbyT("settings.privateSection")
                                        font.pointSize: lobby.sectionPointSize
                                        font.family: "Noto Sans"
                                        color: lobby.textColor
                                        width: parent.width
                                    }
                                    Text {
                                        text: lobbyEncryptionEnabled
                                            ? root.lobbyT("settings.privateOn")
                                            : root.lobbyT("settings.privateOff")
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
                                                property string labelText: root.lobbyT("settings.enable")
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
                                                property string labelText: root.lobbyT("settings.changePin")
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
                                        text: root.lobbyT("settings.pinSection")
                                        font.pointSize: lobby.sectionPointSize
                                        font.family: "Noto Sans"
                                        color: lobby.textColor
                                        width: parent.width
                                    }
                                    Text {
                                        text: root.lobbyT("settings.pinHelp")
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
                                            property string labelText: root.lobbySettingsPinLabel()
                                            property string shortcutKey: lobbyUi.shortcutBadge("settings.pin")
                                            property int pointSize: lobby.labelPointSize
                                            sourceComponent: lobbyBtnLabelComp
                                        }
                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                root.lobbySettingsBeginPickPin()
                                                root.lobbyKeepFocus()
                                            }
                                        }
                                    }

                                    Text {
                                        text: root.lobbyT("settings.rotationSection")
                                        font.pointSize: lobby.sectionPointSize
                                        font.family: "Noto Sans"
                                        color: lobby.textColor
                                        width: parent.width
                                    }
                                    Text {
                                        text: root.lobbyT("settings.rotationHelp")
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
                                                    text: root.lobbyT(modelData.labelKey)
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
                                        text: root.lobbyT("settings.serviceSection")
                                        font.pointSize: lobby.sectionPointSize
                                        font.family: "Noto Sans"
                                        color: lobby.textColor
                                        width: parent.width
                                    }
                                    Text {
                                        text: root.lobbyT("settings.serviceHelp")
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
                                            property string labelText: root.lobbyT("settings.exit")
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
