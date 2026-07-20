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
                                    text: "Writerdeck"
                                    color: "black"
                                    font.pointSize: 26
                                    font.family: "Noto Mono"
                                    width: parent.width
                                }
                                Text {
                                    text: "A text editor for use with a physical keyboard.\nWith Markdown support."
                                    color: "black"
                                    font.pointSize: 12
                                    font.family: "Noto Sans"
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                }
                                Text {
                                    text: lobbyVersionText !== "" ? lobbyVersionText
                                          : "Writerdeck version \u2026 (checking GitHub for updates\u2026)"
                                    color: "black"
                                    font.pointSize: 11
                                    font.family: "Noto Sans"
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                }
                                Text {
                                    text: (lobbyNoteCount === 1 ? "1 note" : lobbyNoteCount + " notes") + " on this device."
                                    color: "black"
                                    font.pointSize: 11
                                    font.family: "Noto Sans"
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                }
                                Text {
                                    text: lobbyUi.str("home.tip")
                                    color: lobby.textColor
                                    font.pointSize: 10
                                    font.family: "Noto Sans"
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                }
                                Text {
                                    text: "Open sourced at github.com/bjornte/Writerdeck-for-reMarkable"
                                    color: "black"
                                    font.pointSize: 9
                                    font.family: "Noto Mono"
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }
                    }
