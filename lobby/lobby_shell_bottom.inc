                }
                Text {
                    id: lobbyHint
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: lobby.pageMargin
                    anchors.rightMargin: lobby.pageMargin
                    anchors.bottomMargin: lobby.pageMargin
                    // Files chords live on the buttons and Shortcuts tab — keep this line short so it
                    // does not collide with the Files action row.
                    height: 36
                    verticalAlignment: Text.AlignVCenter
                    font.family: "Noto Mono"
                    font.pointSize: 9
                    color: "#888888"
                    text: "Tab next page  1-6 jump  Ctrl-K files"
                }
            }
        }
