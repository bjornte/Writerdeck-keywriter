        Rectangle {
            id: lobby
            anchors.fill: parent
            color: "white"
            visible: isLobby
            z: 5

            readonly property int pageMargin: 24
            readonly property int tabBtnHeight: 64
            readonly property int rowHeight: 72
            readonly property int actionBtnHeight: 72
            readonly property int tabSpacing: 12
            readonly property int contentSpacing: 12
            // Inner keycap for single-letter / digit chords. Opacity-only hide
            // when no keyboard so the label does not jump. Square hugs the
            // outer button (tight outer margin), not the glyph inside.
            readonly property int shortcutBadgeMargin: 2

            // Button caption + optional keycap. Loader sets labelText / shortcutKey / pointSize
            // (and optional labelBold / labelColor).
            Component {
                id: lobbyBtnLabelComp
                Item {
                    anchors.fill: parent
                    readonly property string labelText: parent.labelText
                    readonly property string shortcutKey: parent.shortcutKey
                    readonly property int pointSize: parent.pointSize
                    readonly property bool labelBold: !!parent.labelBold
                    readonly property color labelColor: parent.labelColor || "black"
                    readonly property bool showBadge: shortcutKey !== ""

                    Text {
                        anchors.left: parent.left
                        anchors.right: keyBadge.left
                        anchors.leftMargin: 6
                        anchors.rightMargin: showBadge ? 2 : 6
                        anchors.verticalCenter: parent.verticalCenter
                        text: labelText
                        font.family: "Noto Sans"
                        font.pointSize: pointSize
                        font.bold: labelBold
                        color: labelColor
                        horizontalAlignment: Text.AlignHCenter
                        elide: Text.ElideRight
                    }
                    Rectangle {
                        id: keyBadge
                        anchors.right: parent.right
                        anchors.rightMargin: lobby.shortcutBadgeMargin
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.topMargin: lobby.shortcutBadgeMargin
                        anchors.bottomMargin: lobby.shortcutBadgeMargin
                        width: showBadge ? height : 0
                        radius: 3
                        color: "white"
                        border.color: "black"
                        border.width: showBadge ? 1 : 0
                        // Keep layout width when keyboard drops; only fade the keycap.
                        opacity: (showBadge && root.lobbyKeyboardReady()) ? 1 : 0
                        Text {
                            anchors.centerIn: parent
                            visible: showBadge
                            text: shortcutKey
                            font.family: "Noto Sans"
                            // Quieter than the button label — never bolder.
                            font.pointSize: Math.max(8, pointSize - 2)
                            font.bold: false
                            color: "black"
                        }
                    }
                }
            }

            FocusScope {
                id: lobbyFocus
                anchors.fill: parent
                focus: isLobby && !isOmni
                Keys.enabled: isLobby && !isOmni
                Keys.onPressed: {
                    handleKeyDown(event)
                    if (lobbyHandleKey(event))
                        event.accepted = true
                }
                Keys.onReleased: {
                    handleKeyUp(event)
                    handleKey(event)
                }

                Connections {
                    target: root
                    function onIsLobbyChanged() {
                        if (isLobby) Qt.callLater(function() { lobbyFocus.forceActiveFocus() })
                    }
                    // Touch on tabs/buttons/Flickable can steal focus; keys must stay on Lobby.
                    function onActiveFocusItemChanged() {
                        if (!isLobby || isOmni) return
                        if (root.activeFocusItem === lobbyFocus) return
                        Qt.callLater(function() {
                            if (isLobby && !isOmni)
                                lobbyFocus.forceActiveFocus()
                        })
                    }
                }

                // ---- tab bar (touch + keyboard 1-6 / Tab / arrows) ----
                Row {
                    id: lobbyTabRow
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: lobby.pageMargin
                    anchors.rightMargin: lobby.pageMargin
                    anchors.topMargin: lobby.pageMargin
                    height: lobby.tabBtnHeight + 8
                    spacing: lobby.tabSpacing

                    Repeater {
                        model: lobbyTabLabels
                        delegate: Rectangle {
                            width: Math.max(88, (lobby.width - lobby.pageMargin * 2 - lobby.tabSpacing * (lobbyTabLabels.length - 1)) / lobbyTabLabels.length)
                            height: lobby.tabBtnHeight
                            radius: 6
                            color: lobbyPage === index ? "#e0e0e0" : "#f5f5f5"
                            border.color: "black"
                            border.width: lobbyPage === index ? 2 : 1

                            Loader {
                                anchors.fill: parent
                                property string labelText: modelData
                                property string shortcutKey: "" + (index + 1)
                                property int pointSize: 11
                                sourceComponent: lobbyBtnLabelComp
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    root.lobbyGoPage(index)
                                    root.lobbyKeepFocus()
                                }
                            }
                        }
                    }
                }

                // ---- page stack ----
                Item {
                    anchors.top: lobbyTabRow.bottom
                    anchors.topMargin: lobby.contentSpacing
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: lobby.pageMargin
                    anchors.leftMargin: lobby.pageMargin
                    anchors.rightMargin: lobby.pageMargin

