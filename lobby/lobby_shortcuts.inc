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
                                    text: "Pages\n"
                                          + "Tab or Left / Right — next or previous page\n"
                                          + "1 to 6 — jump straight to a page\n"
                                          + "\n"
                                          + "Files\n"
                                          + "Up / Down — move the selection (turns the page at the edge)\n"
                                          + "Page Up / Page Down — previous or next page of notes\n"
                                          + "Enter or \u21B5 — edit the selected note\n"
                                          + "v or Ctrl-V — read · n or Ctrl-N — new · r — rename · d or Ctrl-D — delete\n"
                                          + "g or Ctrl-G — download to phone\n"
                                          + "With private notes on: e / Ctrl-E — new encrypted · x / Ctrl-X — encrypt · y / Ctrl-Y — decrypt\n"
                                          + "(Ctrl-R still rotates; Ctrl-K opens the quick picker; Ctrl-Q quits.)\n"
                                          + "\n"
                                          + "Keyboard\n"
                                          + "u or Ctrl-U — US layout · o or Ctrl-O — Norwegian\n"
                                          + "\n"
                                          + "Sync\n"
                                          + "Enter or s / Ctrl-S — sync now\n"
                                          + "\n"
                                          + "Settings\n"
                                          + "f / Ctrl-F — cycle reading font · p / Ctrl-P — cycle phone PIN length\n"
                                          + "t / Ctrl-T — cycle rotation · e / Ctrl-E — enable private notes · c / Ctrl-C — change private PIN\n"
                                          + "x / Ctrl-X — exit Writerdeck (then Enter to confirm)\n"
                                          + "\n"
                                          + "Anywhere\n"
                                          + "Ctrl-K — quick file picker\n"
                                          + "Ctrl-C / Ctrl-X / Ctrl-V — copy, cut, paste\n"
                                          + "Ctrl-R — rotate · Ctrl-Q — quit\n"
                                          + "Home — from a note, back to Files; from Lobby, quit to the stock UI\n"
                                          + "\n"
                                          + "Private PIN\n"
                                          + "Type the six digits on a USB keyboard, or on the phone while it shows the PIN banner.\n"
                                          + "\n"
                                          + "Open Lobby from the stock UI\n"
                                          + "Esc on a USB keyboard, or both page buttons together."
                                    font.pointSize: 11
                                    font.family: "Noto Sans"
                                    color: "#555555"
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                    lineHeight: 1.25
                                }
                            }
                        }
                    }
