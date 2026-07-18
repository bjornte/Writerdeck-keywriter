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
                                    text: "Shortcuts"
                                    font.pointSize: 14
                                    font.family: "Noto Sans"
                                    color: "black"
                                    width: parent.width
                                }
                                Text {
                                    text: "Lobby: Tab / arrows / 1-6 switch pages\nFiles: Up/Down  PgUp/PgDn  Enter edit  v read  n d r\nFiles vault: e new encrypted  x encrypt  y decrypt\nKeyboard: u US  o Norwegian\nSync: Enter or s sync now\nSettings: f font  p phone PIN  t rotate  e/c vault  x exit\nStock UI: Esc (USB) or L+R page buttons → Lobby\nCtrl-K: quick file picker\nCtrl-C/X/V: copy cut paste\nCtrl-R: rotate  Ctrl-Q: quit\nHome: exit to reMarkable UI\nPrivate PIN: type digits on USB or phone keyboard"
                                    font.pointSize: 10
                                    font.family: "Noto Mono"
                                    color: "#555555"
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }
                    }
