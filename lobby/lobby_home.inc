                    // 5 About
                    Item {
                        visible: lobbyPage === 5
                        anchors.fill: parent
                        Flickable {
                            anchors.fill: parent
                            contentWidth: width
                            contentHeight: aboutCol.height
                            clip: true
                            focus: false
                            interactive: true
                            flickableDirection: Flickable.VerticalFlick
                            Column {
                                id: aboutCol
                                width: parent.width
                                spacing: lobby.contentSpacing
                                Text {
                                    text: root.lobbyT("home.brand")
                                    color: lobby.textColor
                                    font.pointSize: lobby.titlePointSize
                                    font.family: "Noto Mono"
                                    width: parent.width
                                }
                                Text {
                                    text: root.lobbyT("home.tagline")
                                    color: lobby.textColor
                                    font.pointSize: lobby.sectionPointSize
                                    font.family: "Noto Sans"
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                }
                                Text {
                                    text: lobbyVersionText !== "" ? lobbyVersionText
                                          : root.lobbyT("home.versionChecking")
                                    color: lobby.textColor
                                    font.pointSize: lobby.labelPointSize
                                    font.family: "Noto Sans"
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                }
                                Text {
                                    text: lobbyNoteCount === 1
                                          ? root.lobbyT("home.docsOne")
                                          : root.lobbyTf("home.docsMany", "" + lobbyNoteCount)
                                    color: lobby.textColor
                                    font.pointSize: lobby.labelPointSize
                                    font.family: "Noto Sans"
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                }
                                Text {
                                    text: root.lobbyT("home.tip")
                                    color: lobby.textColor
                                    font.pointSize: lobby.helpPointSize
                                    font.family: "Noto Sans"
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                }
                                Text {
                                    text: root.lobbyT("home.opensource")
                                    color: lobby.textColor
                                    font.pointSize: 9
                                    font.family: "Noto Mono"
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }
                    }
