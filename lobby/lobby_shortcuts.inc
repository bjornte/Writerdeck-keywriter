                    // 5 Shortcuts
                    Item {
                        visible: lobbyPage === 4
                        anchors.fill: parent
                        Flickable {
                            anchors.fill: parent
                            contentWidth: width
                            contentHeight: scCol.height
                            clip: true
                            focus: false
                            interactive: true
                            flickableDirection: Flickable.VerticalFlick
                            Column {
                                id: scCol
                                width: parent.width
                                spacing: lobby.contentSpacing
                                Text {
                                    text: lobbyUi.str("shortcuts.title")
                                    font.pointSize: 14
                                    font.family: "Noto Sans"
                                    color: lobby.textColor
                                    width: parent.width
                                }
                                Text {
                                    text: lobbyUi.str("shortcuts.body")
                                    font.pointSize: 11
                                    font.family: "Noto Sans"
                                    color: lobby.textColor
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                    lineHeight: 1.25
                                }
                            }
                        }
                    }
