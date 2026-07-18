                }
                Text {
                    id: lobbyHint
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: lobby.pageMargin
                    anchors.rightMargin: lobby.pageMargin
                    anchors.bottomMargin: lobby.pageMargin
                    // Hide on Files: action buttons already cover those chords, and a hint line
                    // was colliding with the bottom button row.
                    visible: !(lobbyPage === 0 && lobbyFilesMode === "")
                    height: visible ? 36 : 0
                    verticalAlignment: Text.AlignVCenter
                    font.family: "Noto Mono"
                    font.pointSize: 9
                    color: "#888888"
                    text: "Tab next page  1-6 jump  Ctrl-K files"
                }
            }
        }
