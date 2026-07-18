        Rectangle {
            id: noKeyboardOverlay
            anchors.fill: parent
            color: "#f8f8f8"
            visible: lobbyShowNoKeyboard
            z: 26

            Column {
                anchors.centerIn: parent
                width: parent.width * 0.85
                spacing: lobby.contentSpacing

                Text {
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    font.family: "Noto Sans"
                    font.pointSize: 16
                    color: "black"
                    text: "Connect a keyboard"
                }

                Text {
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    font.family: "Noto Sans"
                    font.pointSize: 12
                    color: "#333"
                    text: "USB: plug in with an OTG cable.\n\nBluetooth: pair to your phone, then open the address below (or scan the code)."
                }

                Text {
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    font.family: "Noto Mono"
                    font.pointSize: 13
                    color: "black"
                    text: root.lobbyPhoneUrl()
                }

                Text {
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    font.family: "Noto Sans"
                    font.pointSize: 12
                    color: "#333"
                    visible: lobbyPIN !== ""
                    text: "PIN: " + lobbyPIN
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

                Text {
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    font.family: "Noto Sans"
                    font.pointSize: 11
                    color: "#555"
                    text: "Esc or Cancel to close. Tip closes when a keyboard connects."
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
                        text: "Cancel"
                        font.family: "Noto Sans"
                        font.pointSize: 12
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
