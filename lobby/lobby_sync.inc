                    // 3 Sync
                    Item {
                        visible: lobbyPage === 2
                        anchors.fill: parent
                        Flickable {
                            anchors.fill: parent
                            contentWidth: width
                            contentHeight: syncCol.height
                            clip: true
                            focus: false
                            interactive: true
                            flickableDirection: Flickable.VerticalFlick
                            Column {
                                id: syncCol
                                width: parent.width
                                spacing: lobby.contentSpacing
                                Text {
                                    text: root.lobbyT("sync.title")
                                    font.pointSize: lobby.rowPointSize
                                    font.family: "Noto Sans"
                                    color: lobby.textColor
                                    width: parent.width
                                }
                                Text {
                                    visible: !(lobbySyncOn && lobbySyncRepo !== "" && !lobbySyncReady)
                                        && !(lobbySyncOn && lobbySyncRepo !== "" && lobbySyncReady && lobbySyncError !== "")
                                    text: lobbySyncOn && lobbySyncRepo !== ""
                                        ? (root.lobbyLastSyncLabel() !== ""
                                            ? root.lobbyTf("sync.lastSync", root.lobbyLastSyncLabel(), lobbySyncRepo)
                                            : root.lobbyTf("sync.syncingTo", lobbySyncRepo))
                                        : root.lobbyTf("sync.notConfigured", lobbyIP, "" + lobbyPort)
                                    font.pointSize: lobby.labelPointSize
                                    font.family: "Noto Sans"
                                    color: lobby.textColor
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                }
                                Rectangle {
                                    visible: lobbySyncOn && lobbySyncRepo !== "" && lobbySyncReady && lobbySyncError !== ""
                                    width: parent.width
                                    height: syncErrCol.height + 20
                                    color: lobby.dialogBg
                                    border.color: lobby.borderColor
                                    border.width: lobby.btnBorder
                                    radius: lobby.bannerRadius
                                    Column {
                                        id: syncErrCol
                                        anchors.centerIn: parent
                                        width: parent.width - 20
                                        spacing: 10
                                        Text {
                                            text: !lobbyWifi ? root.lobbyT("sync.offline") : root.lobbyT("sync.failed")
                                            font.pointSize: lobby.bannerPointSize
                                            font.bold: true
                                            font.family: "Noto Sans"
                                            color: lobby.textColor
                                            width: parent.width
                                        }
                                        Text {
                                            text: lobbySyncError
                                            font.pointSize: 13
                                            font.family: "Noto Sans"
                                            color: lobby.textColor
                                            width: parent.width
                                            wrapMode: Text.WordWrap
                                            lineHeight: 1.25
                                        }
                                    }
                                }
                                Rectangle {
                                    visible: lobbySyncOn && lobbySyncRepo !== "" && !lobbySyncReady
                                    width: parent.width
                                    height: tokenWarnCol.height + 20
                                    color: lobby.dialogBg
                                    border.color: lobby.borderColor
                                    border.width: lobby.btnBorder
                                    radius: lobby.bannerRadius
                                    Column {
                                        id: tokenWarnCol
                                        anchors.centerIn: parent
                                        width: parent.width - 20
                                        spacing: 10
                                        Text {
                                            text: root.lobbyT("sync.tokenNeeded")
                                            font.pointSize: lobby.bannerPointSize
                                            font.bold: true
                                            font.family: "Noto Sans"
                                            color: lobby.textColor
                                            width: parent.width
                                        }
                                        Text {
                                            text: root.lobbyTf("sync.tokenBody", lobbyIP, "" + lobbyPort, lobbySyncRepo)
                                            font.pointSize: 13
                                            font.family: "Noto Sans"
                                            color: lobby.textColor
                                            width: parent.width
                                            wrapMode: Text.WordWrap
                                            lineHeight: 1.25
                                        }
                                    }
                                }
                                Rectangle {
                                    visible: lobbySyncOn && lobbySyncRepo !== ""
                                    width: parent.width
                                    height: lobby.actionBtnHeight
                                    radius: lobby.btnRadius
                                    color: (lobbySyncReady && !lobbySyncing) ? lobby.btnFill : lobby.dialogBg
                                    border.color: lobby.borderColor
                                    border.width: lobbySyncReady ? lobby.btnBorder : lobby.btnBorderSelected
                                    Loader {
                                        anchors.fill: parent
                                        property string labelText: !lobbySyncReady ? root.lobbyT("sync.tokenBtn")
                                            : (lobbySyncing ? root.lobbyT("sync.syncing") : root.lobbyT("sync.now"))
                                        property string shortcutKey: (lobbySyncReady && !lobbySyncing)
                                            ? lobbyUi.shortcutBadge("sync.now") : ""
                                        property int pointSize: !lobbySyncReady ? 14 : 12
                                        property bool labelBold: !lobbySyncReady
                                        property color labelColor: lobby.textColor
                                        sourceComponent: lobbyBtnLabelComp
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        enabled: lobbySyncReady && !lobbySyncing
                                        onClicked: {
                                            writerdeck.syncNow()
                                            root.lobbyKeepFocus()
                                        }
                                    }
                                }
                                Text {
                                    text: root.lobbyT("sync.footnote")
                                    font.pointSize: lobby.helpPointSize
                                    font.family: "Noto Sans"
                                    color: lobby.textColor
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }
                    }
