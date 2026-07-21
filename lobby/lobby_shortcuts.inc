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
                                    font.pointSize: lobby.rowPointSize
                                    font.family: "Noto Sans"
                                    color: lobby.textColor
                                    width: parent.width
                                }
                                Text {
                                    text: lobbyUi.str("shortcuts.body")
                                    font.pointSize: lobby.labelPointSize
                                    font.family: "Noto Sans"
                                    color: lobby.textColor
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                    lineHeight: 1.25
                                }
                            }
                        }
                    }
