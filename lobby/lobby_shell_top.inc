        Rectangle {
            id: lobby
            anchors.fill: parent
            color: lobbyUi.pageBg
            visible: isLobby
            z: 5

            readonly property int pageMargin: lobbyUi.pageMargin
            readonly property int tabBtnHeight: lobbyUi.tabBtnHeight
            readonly property int rowHeight: lobbyUi.rowHeight
            readonly property int actionBtnHeight: lobbyUi.actionBtnHeight
            readonly property int tabSpacing: lobbyUi.tabSpacing
            readonly property int contentSpacing: lobbyUi.contentSpacing
            readonly property int btnBorder: lobbyUi.btnBorder
            readonly property int btnBorderSelected: lobbyUi.btnBorderSelected
            readonly property int shortcutBadgeMargin: lobbyUi.shortcutBadgeMargin
            readonly property color textColor: lobbyUi.textColor
            readonly property color borderColor: lobbyUi.borderColor
            readonly property color badgeTextColor: lobbyUi.badgeTextColor
            readonly property color badgeBorderColor: lobbyUi.badgeBorderColor
            readonly property color btnFill: lobbyUi.btnFill
            readonly property color btnFillSelected: lobbyUi.btnFillSelected
            readonly property color tabFill: lobbyUi.tabFill
            readonly property color tabFillSelected: lobbyUi.tabFillSelected
            readonly property color dialogBg: lobbyUi.dialogBg
            readonly property color dialogScrim: lobbyUi.dialogScrim
            readonly property color vaultWash: lobbyUi.vaultWash
            readonly property int btnRadius: lobbyUi.btnRadius
            readonly property int dialogRadius: lobbyUi.dialogRadius
            readonly property int badgeRadius: lobbyUi.badgeRadius
            readonly property int bannerRadius: lobbyUi.bannerRadius
            readonly property int labelPointSize: lobbyUi.labelPointSize
            readonly property int badgePointSize: lobbyUi.badgePointSize
            readonly property int titlePointSize: lobbyUi.titlePointSize
            readonly property int sectionPointSize: lobbyUi.sectionPointSize
            readonly property int rowPointSize: lobbyUi.rowPointSize
            readonly property int dialogTitlePointSize: lobbyUi.dialogTitlePointSize
            readonly property int bannerPointSize: lobbyUi.bannerPointSize
            readonly property int helpPointSize: lobbyUi.helpPointSize
            readonly property int pageStripHeight: lobbyUi.pageStripHeight
            readonly property int listRowInset: lobbyUi.listRowInset
            readonly property int tabRowExtraHeight: lobbyUi.tabRowExtraHeight
            readonly property double dialogWidthFraction: lobbyUi.dialogWidthFraction
            readonly property int dialogPadding: lobbyUi.dialogPadding
            readonly property int settingsLandscapeScrollGutter: lobbyUi.settingsLandscapeScrollGutter

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
                    readonly property color labelColor: parent.labelColor !== undefined ? parent.labelColor : lobby.textColor
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
                        radius: lobby.badgeRadius
                        color: "white"
                        border.color: lobby.badgeBorderColor
                        border.width: showBadge ? lobby.btnBorder : 0
                        // Keep layout width when keyboard drops; only fade the keycap.
                        opacity: (showBadge && root.lobbyKeyboardReady()) ? 1 : 0
                        Text {
                            anchors.centerIn: parent
                            visible: showBadge
                            text: shortcutKey
                            font.family: "Noto Sans"
                            font.pointSize: lobby.badgePointSize
                            font.bold: false
                            color: lobby.badgeTextColor
                        }
                    }
                }
            }

            FocusScope {
                id: lobbyFocus
                anchors.fill: parent
                focus: isLobby
                Keys.enabled: isLobby
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
                        if (!isLobby) return
                        if (root.activeFocusItem === lobbyFocus) return
                        Qt.callLater(function() {
                            if (isLobby)
                                lobbyFocus.forceActiveFocus()
                        })
                    }
                }

                // ---- tab bar (touch + Tab / arrows / optional tabs.* chords) ----
                Row {
                    id: lobbyTabRow
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: lobby.pageMargin
                    anchors.rightMargin: lobby.pageMargin
                    anchors.topMargin: lobby.pageMargin
                    height: lobby.tabBtnHeight + lobby.tabRowExtraHeight
                    spacing: lobby.tabSpacing

                    Repeater {
                        model: lobbyTabLabels
                        delegate: Rectangle {
                            width: Math.max(88, (lobby.width - lobby.pageMargin * 2 - lobby.tabSpacing * (lobbyTabLabels.length - 1)) / lobbyTabLabels.length)
                            height: lobby.tabBtnHeight
                            radius: lobby.btnRadius
                            color: lobbyPage === index ? lobby.tabFillSelected : lobby.tabFill
                            border.color: lobby.borderColor
                            border.width: lobbyPage === index ? lobby.btnBorderSelected : lobby.btnBorder

                            Loader {
                                anchors.fill: parent
                                property string labelText: modelData
                                property string shortcutKey: lobbyUi.shortcutBadge(lobbyTabShortcutIds[index])
                                property int pointSize: lobby.labelPointSize
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
