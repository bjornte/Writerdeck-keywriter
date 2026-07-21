                    // 2 Keyboard
                    Item {
                        visible: lobbyPage === 1
                        anchors.fill: parent
                        // Live presence while this tab is open (USB plug / phone page).
                        Timer {
                            interval: 2000
                            running: lobbyPage === 1 && isLobby
                            repeat: true
                            onTriggered: writerdeck.requestLobbyInfo()
                        }
                        Flickable {
                            anchors.fill: parent
                            contentWidth: width
                            contentHeight: kbCol.height
                            clip: true
                            focus: false
                            interactive: true
                            flickableDirection: Flickable.VerticalFlick
                            Column {
                                id: kbCol
                                width: parent.width
                                spacing: lobby.contentSpacing

                                // ---- Bluetooth keyboard (phone bridge) ----
                                Rectangle {
                                    width: parent.width
                                    height: btKbInner.height + 24
                                    color: lobby.dialogBg
                                    border.color: lobby.borderColor
                                    border.width: lobbyPhoneConnected ? lobby.btnBorderSelected : lobby.btnBorder
                                    radius: lobby.btnRadius
                                    Column {
                                        id: btKbInner
                                        anchors.left: parent.left
                                        anchors.right: parent.right
                                        anchors.top: parent.top
                                        anchors.margins: 12
                                        spacing: 10
                                        Text {
                                            text: lobbyUi.str("keyboard.btTitle")
                                                  + (lobbyPhoneConnected
                                                     ? lobbyUi.str("keyboard.connected")
                                                     : lobbyUi.str("keyboard.notConnected"))
                                            font.pointSize: lobby.rowPointSize
                                            font.family: "Noto Sans"
                                            color: lobby.textColor
                                            width: parent.width
                                            wrapMode: Text.WordWrap
                                        }
                                        Text {
                                            text: lobbyUi.str("keyboard.btBody")
                                            font.pointSize: lobby.labelPointSize
                                            font.family: "Noto Sans"
                                            color: lobby.textColor
                                            width: parent.width
                                            wrapMode: Text.WordWrap
                                        }
                                        Text {
                                            text: root.lobbyPhoneUrl()
                                            font.pointSize: 13
                                            font.family: "Noto Mono"
                                            color: lobby.textColor
                                            width: parent.width
                                            wrapMode: Text.WrapAnywhere
                                            horizontalAlignment: Text.AlignHCenter
                                        }
                                        Text {
                                            visible: lobbyPIN !== ""
                                            text: lobbyUi.str("keyboard.pinPrefix") + lobbyPIN
                                            font.pointSize: lobby.sectionPointSize
                                            font.family: "Noto Sans"
                                            color: lobby.textColor
                                            width: parent.width
                                            horizontalAlignment: Text.AlignHCenter
                                        }
                                        Image {
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            width: Math.min(parent.width * 0.55, 280)
                                            height: width
                                            fillMode: Image.PreserveAspectFit
                                            visible: lobbyQrPath !== ""
                                            source: lobbyQrPath !== "" ? ("file://" + lobbyQrPath) : ""
                                            cache: false
                                        }
                                    }
                                }

                                // ---- USB keyboard ----
                                Rectangle {
                                    width: parent.width
                                    height: usbKbInner.height + 24
                                    color: lobby.dialogBg
                                    border.color: lobby.borderColor
                                    border.width: lobbyUsbKeyboard ? lobby.btnBorderSelected : lobby.btnBorder
                                    radius: lobby.btnRadius
                                    Column {
                                        id: usbKbInner
                                        anchors.left: parent.left
                                        anchors.right: parent.right
                                        anchors.top: parent.top
                                        anchors.margins: 12
                                        spacing: 10
                                        Text {
                                            text: lobbyUi.str("keyboard.usbTitle")
                                                  + (lobbyUsbKeyboard
                                                     ? lobbyUi.str("keyboard.connected")
                                                     : lobbyUi.str("keyboard.notConnected"))
                                            font.pointSize: lobby.rowPointSize
                                            font.family: "Noto Sans"
                                            color: lobby.textColor
                                            width: parent.width
                                            wrapMode: Text.WordWrap
                                        }
                                        Text {
                                            text: lobbyUi.str("keyboard.usbBody")
                                            font.pointSize: lobby.labelPointSize
                                            font.family: "Noto Sans"
                                            color: lobby.textColor
                                            width: parent.width
                                            wrapMode: Text.WordWrap
                                        }
                                        Text {
                                            text: lobbyUi.str("keyboard.layout")
                                            font.pointSize: lobby.sectionPointSize
                                            font.family: "Noto Sans"
                                            color: lobby.textColor
                                            width: parent.width
                                        }
                                        Grid {
                                            id: kbLayoutGrid
                                            width: parent.width
                                            columns: 2
                                            spacing: lobby.tabSpacing
                                            Repeater {
                                                model: [
                                                    { id: "us", labelKey: "keyboard.us", action: "keyboard.us" },
                                                    { id: "no", labelKey: "keyboard.no", action: "keyboard.no" },
                                                    { id: "es", labelKey: "keyboard.es", action: "keyboard.es" },
                                                    { id: "de", labelKey: "keyboard.de", action: "keyboard.de" },
                                                    { id: "fr", labelKey: "keyboard.fr", action: "keyboard.fr" }
                                                ]
                                                delegate: Rectangle {
                                                    width: (kbLayoutGrid.width - lobby.tabSpacing) / 2
                                                    height: lobby.actionBtnHeight
                                                    radius: lobby.btnRadius
                                                    property bool selected: lobbyKeyboardLayout === modelData.id
                                                    color: selected ? lobby.btnFillSelected : lobby.btnFill
                                                    border.color: lobby.borderColor
                                                    border.width: selected ? lobby.btnBorderSelected : lobby.btnBorder
                                                    Loader {
                                                        anchors.fill: parent
                                                        property string labelText: lobbyUi.str(modelData.labelKey)
                                                        property string shortcutKey: lobbyUi.shortcutBadge(modelData.action)
                                                        property int pointSize: lobby.labelPointSize
                                                        sourceComponent: lobbyBtnLabelComp
                                                    }
                                                    MouseArea {
                                                        anchors.fill: parent
                                                        onClicked: {
                                                            writerdeck.setKeyboardLayout(modelData.id)
                                                            root.lobbyKeepFocus()
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
