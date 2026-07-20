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
                                    color: "white"
                                    border.color: lobby.borderColor
                                    border.width: lobbyPhoneConnected ? lobby.btnBorderSelected : lobby.btnBorder
                                    radius: 6
                                    Column {
                                        id: btKbInner
                                        anchors.left: parent.left
                                        anchors.right: parent.right
                                        anchors.top: parent.top
                                        anchors.margins: 12
                                        spacing: 10
                                        Text {
                                            text: "Bluetooth keyboard" + (lobbyPhoneConnected ? " (connected)" : " (not connected)")
                                            font.pointSize: 14
                                            font.family: "Noto Sans"
                                            color: "black"
                                            width: parent.width
                                            wrapMode: Text.WordWrap
                                        }
                                        Text {
                                            text: "Pair the keyboard to your phone, then open the address below (or scan the code). Typing is forwarded over Wi-Fi."
                                            font.pointSize: 11
                                            font.family: "Noto Sans"
                                            color: "black"
                                            width: parent.width
                                            wrapMode: Text.WordWrap
                                        }
                                        Text {
                                            text: root.lobbyPhoneUrl()
                                            font.pointSize: 13
                                            font.family: "Noto Mono"
                                            color: "black"
                                            width: parent.width
                                            wrapMode: Text.WrapAnywhere
                                            horizontalAlignment: Text.AlignHCenter
                                        }
                                        Text {
                                            visible: lobbyPIN !== ""
                                            text: "PIN: " + lobbyPIN
                                            font.pointSize: 12
                                            font.family: "Noto Sans"
                                            color: "black"
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
                                    color: "white"
                                    border.color: lobby.borderColor
                                    border.width: lobbyUsbKeyboard ? lobby.btnBorderSelected : lobby.btnBorder
                                    radius: 6
                                    Column {
                                        id: usbKbInner
                                        anchors.left: parent.left
                                        anchors.right: parent.right
                                        anchors.top: parent.top
                                        anchors.margins: 12
                                        spacing: 10
                                        Text {
                                            text: "USB keyboard" + (lobbyUsbKeyboard ? " (connected)" : " (not connected)")
                                            font.pointSize: 14
                                            font.family: "Noto Sans"
                                            color: "black"
                                            width: parent.width
                                            wrapMode: Text.WordWrap
                                        }
                                        Text {
                                            text: "Connect with a USB OTG cable.\nChanging layout restarts Writerdeck."
                                            font.pointSize: 11
                                            font.family: "Noto Sans"
                                            color: "black"
                                            width: parent.width
                                            wrapMode: Text.WordWrap
                                        }
                                        Text {
                                            text: "Layout"
                                            font.pointSize: 12
                                            font.family: "Noto Sans"
                                            color: "black"
                                            width: parent.width
                                        }
                                        Row {
                                            id: kbLayoutRow
                                            width: parent.width
                                            spacing: lobby.tabSpacing
                                            Repeater {
                                                model: [
                                                    { id: "us", label: "US QWERTY", action: "keyboard.us" },
                                                    { id: "no", label: "Norwegian", action: "keyboard.no" }
                                                ]
                                                delegate: Rectangle {
                                                    width: (kbLayoutRow.width - lobby.tabSpacing) / 2
                                                    height: lobby.actionBtnHeight
                                                    radius: 6
                                                    property bool selected: lobbyKeyboardLayout === modelData.id
                                                    color: selected ? "#e8e8e8" : "#f0f0f0"
                                                    border.color: lobby.borderColor
                                                    border.width: selected ? lobby.btnBorderSelected : lobby.btnBorder
                                                    Loader {
                                                        anchors.fill: parent
                                                        property string labelText: modelData.label
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
