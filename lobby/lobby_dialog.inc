        // Shared Lobby dialog chrome: grey scrim + white floating box (black type).
        // Kind comes from lobbyShowNoKeyboard / lobbyFilesMode — keep content in one piece.
        Rectangle {
            id: lobbyDialogScrim
            anchors.fill: parent
            color: lobby.dialogScrim
            visible: root.lobbyDialogIsOpen()
            z: 24

            Rectangle {
                id: lobbyDialogBox
                anchors.centerIn: parent
                width: Math.min(parent.width * lobby.dialogWidthFraction, parent.width - lobby.dialogPadding)
                height: lobbyDialogCol.height + lobby.dialogPadding
                color: lobby.dialogBg
                border.color: lobby.borderColor
                border.width: lobby.btnBorderSelected
                radius: lobby.dialogRadius

                Column {
                    id: lobbyDialogCol
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: 24
                    width: parent.width - lobby.dialogPadding
                    spacing: 16

                    Text {
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        font.family: "Noto Sans"
                        font.pointSize: lobby.dialogTitlePointSize
                        color: lobby.textColor
                        text: root.lobbyDialogTitle()
                    }

                    // ---- confirm-delete body ----
                    Text {
                        width: parent.width
                        visible: !lobbyShowNoKeyboard && lobbyFilesMode === "confirm-delete"
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        elide: Text.ElideMiddle
                        font.family: "Noto Sans"
                        font.pointSize: 13
                        color: lobby.textColor
                        text: root.lobbyDialogSelectedNoteLabel()
                    }

                    // ---- new / rename / new-encrypted body ----
                    Text {
                        width: parent.width
                        visible: !lobbyShowNoKeyboard
                                 && (lobbyFilesMode === "new"
                                     || lobbyFilesMode === "rename"
                                     || lobbyFilesMode === "new-encrypted")
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WrapAnywhere
                        font.family: "Noto Mono"
                        font.pointSize: 14
                        color: lobby.textColor
                        text: root.lobbyFilesInputDisplay()
                    }

                    Text {
                        width: parent.width
                        visible: !lobbyShowNoKeyboard
                                 && lobbyFilesInputError !== ""
                                 && (lobbyFilesMode === "new"
                                     || lobbyFilesMode === "rename"
                                     || lobbyFilesMode === "new-encrypted")
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        font.family: "Noto Sans"
                        font.pointSize: 12
                        color: lobby.textColor
                        text: lobbyFilesInputError
                    }

                    // ---- no-keyboard body ----
                    Text {
                        width: parent.width
                        visible: lobbyShowNoKeyboard
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        font.family: "Noto Sans"
                        font.pointSize: 12
                        color: lobby.textColor
                        text: root.lobbyT("dialog.noKeyboardBody")
                    }

                    Text {
                        width: parent.width
                        visible: lobbyShowNoKeyboard
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        font.family: "Noto Mono"
                        font.pointSize: 13
                        color: lobby.textColor
                        text: root.lobbyPhoneUrl()
                    }

                    Text {
                        width: parent.width
                        visible: lobbyShowNoKeyboard && lobbyPIN !== ""
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        font.family: "Noto Sans"
                        font.pointSize: 12
                        color: lobby.textColor
                        text: root.lobbyT("dialog.pinPrefix") + lobbyPIN
                    }

                    Image {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: Math.min(parent.width * 0.55, 280)
                        height: width
                        fillMode: Image.PreserveAspectFit
                        visible: lobbyShowNoKeyboard && lobbyQrPath !== ""
                        source: (lobbyShowNoKeyboard && lobbyQrPath !== "") ? ("file://" + lobbyQrPath) : ""
                        cache: false
                    }

                    // ---- Settings pickers (language / font / PIN) ----
                    Column {
                        width: parent.width
                        spacing: lobby.tabSpacing
                        visible: !lobbyShowNoKeyboard && lobbySettingsMode === "pick-language"
                        Repeater {
                            model: root.lobbySettingsLanguageOptions()
                            delegate: Rectangle {
                                width: parent.width
                                height: lobby.actionBtnHeight
                                radius: lobby.btnRadius
                                property bool selected: index === lobbySettingsPickIndex
                                color: selected ? lobby.btnFillSelected : lobby.btnFill
                                border.color: lobby.borderColor
                                border.width: selected ? lobby.btnBorderSelected : lobby.btnBorder
                                Text {
                                    anchors.centerIn: parent
                                    text: modelData.label
                                    font.family: "Noto Sans"
                                    font.pointSize: lobby.labelPointSize
                                    color: lobby.textColor
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: root.lobbySettingsApplyPick(index)
                                }
                            }
                        }
                    }

                    Column {
                        width: parent.width
                        spacing: lobby.tabSpacing
                        visible: !lobbyShowNoKeyboard && lobbySettingsMode === "pick-font"
                        Repeater {
                            model: root.lobbySettingsFontOptions()
                            delegate: Rectangle {
                                width: parent.width
                                height: lobby.actionBtnHeight + lobby.fontPickerRowExtra
                                radius: lobby.btnRadius
                                property bool selected: index === lobbySettingsPickIndex
                                color: selected ? lobby.btnFillSelected : lobby.btnFill
                                border.color: lobby.borderColor
                                border.width: selected ? lobby.btnBorderSelected : lobby.btnBorder
                                Column {
                                    anchors.centerIn: parent
                                    width: parent.width - 16
                                    spacing: 4
                                    Text {
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        text: modelData.label
                                        font.family: modelData.id
                                        font.pointSize: lobby.fontPickerNamePointSize
                                        color: lobby.textColor
                                    }
                                    Text {
                                        width: parent.width
                                        horizontalAlignment: Text.AlignHCenter
                                        wrapMode: Text.WordWrap
                                        text: root.lobbyFontSample
                                        font.family: modelData.id
                                        font.pointSize: lobby.fontPickerSamplePointSize
                                        color: lobby.textColor
                                    }
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: root.lobbySettingsApplyPick(index)
                                }
                            }
                        }
                    }

                    Column {
                        width: parent.width
                        spacing: lobby.tabSpacing
                        visible: !lobbyShowNoKeyboard && lobbySettingsMode === "pick-pin"
                        Repeater {
                            model: root.lobbySettingsPinOptions()
                            delegate: Rectangle {
                                width: parent.width
                                height: modelData.warnKey ? lobby.actionBtnHeight + 28 : lobby.actionBtnHeight
                                radius: lobby.btnRadius
                                property bool selected: index === lobbySettingsPickIndex
                                color: selected ? lobby.btnFillSelected : lobby.btnFill
                                border.color: lobby.borderColor
                                border.width: selected ? lobby.btnBorderSelected : lobby.btnBorder
                                Column {
                                    anchors.centerIn: parent
                                    width: parent.width - 16
                                    spacing: 2
                                    Text {
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        text: root.lobbyT(modelData.labelKey)
                                        font.family: "Noto Sans"
                                        font.pointSize: lobby.labelPointSize
                                        color: lobby.textColor
                                    }
                                    Text {
                                        visible: !!modelData.warnKey
                                        width: parent.width
                                        horizontalAlignment: Text.AlignHCenter
                                        wrapMode: Text.WordWrap
                                        text: modelData.warnKey ? root.lobbyT(modelData.warnKey) : ""
                                        font.family: "Noto Sans"
                                        font.pointSize: 9
                                        color: lobby.textColor
                                    }
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: root.lobbySettingsApplyPick(index)
                                }
                            }
                        }
                    }

                    // ---- buttons ----
                    Row {
                        width: parent.width
                        spacing: lobby.tabSpacing
                        visible: !lobbyShowNoKeyboard && lobbyFilesMode === "confirm-delete"

                        Rectangle {
                            width: (parent.width - lobby.tabSpacing) / 2
                            height: lobby.actionBtnHeight
                            radius: lobby.btnRadius
                            color: lobby.btnFill
                            border.color: lobby.borderColor
                            border.width: lobby.btnBorder
                            Text {
                                anchors.centerIn: parent
                                text: root.lobbyT("dialog.cancel")
                                font.family: "Noto Sans"
                                font.pointSize: 12
                                color: lobby.textColor
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    lobbyFilesMode = ""
                                    root.lobbyKeepFocus()
                                }
                            }
                        }
                        Rectangle {
                            width: (parent.width - lobby.tabSpacing) / 2
                            height: lobby.actionBtnHeight
                            radius: lobby.btnRadius
                            color: lobby.btnFill
                            border.color: lobby.borderColor
                            border.width: lobby.btnBorderSelected
                            Text {
                                anchors.centerIn: parent
                                text: root.lobbyT("dialog.delete")
                                font.family: "Noto Sans"
                                font.pointSize: 12
                                color: lobby.textColor
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    root.lobbyFilesDoDelete()
                                    root.lobbyKeepFocus()
                                }
                            }
                        }
                    }

                    Row {
                        width: parent.width
                        spacing: lobby.tabSpacing
                        visible: !lobbyShowNoKeyboard && lobbySettingsMode === "confirm-exit"

                        Rectangle {
                            width: (parent.width - lobby.tabSpacing) / 2
                            height: lobby.actionBtnHeight
                            radius: lobby.btnRadius
                            color: lobby.btnFill
                            border.color: lobby.borderColor
                            border.width: lobby.btnBorder
                            Text {
                                anchors.centerIn: parent
                                text: root.lobbyT("dialog.cancel")
                                font.family: "Noto Sans"
                                font.pointSize: 12
                                color: lobby.textColor
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: root.lobbySettingsCancelPick()
                            }
                        }
                        Rectangle {
                            width: (parent.width - lobby.tabSpacing) / 2
                            height: lobby.actionBtnHeight
                            radius: lobby.btnRadius
                            color: lobby.btnFill
                            border.color: lobby.borderColor
                            border.width: lobby.btnBorderSelected
                            Text {
                                anchors.centerIn: parent
                                text: root.lobbyT("settings.exit")
                                font.family: "Noto Sans"
                                font.pointSize: 12
                                color: lobby.textColor
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: root.lobbySettingsApplyPick()
                            }
                        }
                    }

                    // Separator + narrower Cancel — not another full-width choice.
                    Rectangle {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width * 0.4
                        height: 2
                        color: lobby.borderColor
                        visible: !lobbyShowNoKeyboard
                                 && (lobbySettingsMode === "pick-language"
                                     || lobbySettingsMode === "pick-font"
                                     || lobbySettingsMode === "pick-pin")
                    }

                    Item {
                        width: parent.width
                        height: lobby.actionBtnHeight
                        visible: !lobbyShowNoKeyboard
                                 && (lobbySettingsMode === "pick-language"
                                     || lobbySettingsMode === "pick-font"
                                     || lobbySettingsMode === "pick-pin")

                        Rectangle {
                            width: Math.max(120, parent.width * lobby.dialogCancelWidthFraction)
                            height: lobby.actionBtnHeight
                            anchors.horizontalCenter: parent.horizontalCenter
                            radius: lobby.btnRadius
                            color: lobby.btnFill
                            border.color: lobby.borderColor
                            border.width: lobby.btnBorder
                            Text {
                                anchors.centerIn: parent
                                text: root.lobbyT("dialog.cancel")
                                font.family: "Noto Sans"
                                font.pointSize: 12
                                color: lobby.textColor
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: root.lobbySettingsCancelPick()
                            }
                        }
                    }

                    Row {
                        width: parent.width
                        spacing: lobby.tabSpacing
                        visible: !lobbyShowNoKeyboard
                                 && (lobbyFilesMode === "new"
                                     || lobbyFilesMode === "rename"
                                     || lobbyFilesMode === "new-encrypted")

                        Rectangle {
                            width: (parent.width - lobby.tabSpacing) / 2
                            height: lobby.actionBtnHeight
                            radius: lobby.btnRadius
                            color: lobby.btnFill
                            border.color: lobby.borderColor
                            border.width: lobby.btnBorder
                            Text {
                                anchors.centerIn: parent
                                text: root.lobbyT("dialog.cancel")
                                font.family: "Noto Sans"
                                font.pointSize: 12
                                color: lobby.textColor
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    lobbyFilesInputError = ""
                                    lobbyFilesPendingMode = ""
                                    lobbyFilesPendingInput = ""
                                    lobbyFilesMode = ""
                                    lobbyFilesInput = ""
                                    lobbyFilesInputPos = 0
                                    root.lobbyKeepFocus()
                                }
                            }
                        }
                        Rectangle {
                            width: (parent.width - lobby.tabSpacing) / 2
                            height: lobby.actionBtnHeight
                            radius: lobby.btnRadius
                            color: lobby.btnFill
                            border.color: lobby.borderColor
                            border.width: lobby.btnBorderSelected
                            Text {
                                anchors.centerIn: parent
                                text: lobbyFilesMode === "rename"
                                      ? root.lobbyT("dialog.rename")
                                      : root.lobbyT("dialog.create")
                                font.family: "Noto Sans"
                                font.pointSize: 12
                                color: lobby.textColor
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    root.lobbyFilesSubmitInput()
                                    root.lobbyKeepFocus()
                                }
                            }
                        }
                    }

                    Row {
                        width: parent.width
                        spacing: lobby.tabSpacing
                        visible: lobbyShowNoKeyboard

                        Rectangle {
                            width: parent.width
                            height: lobby.actionBtnHeight
                            radius: lobby.btnRadius
                            color: lobby.btnFill
                            border.color: lobby.borderColor
                            border.width: lobby.btnBorder
                            Text {
                                anchors.centerIn: parent
                                text: root.lobbyT("dialog.cancel")
                                font.family: "Noto Sans"
                                font.pointSize: 12
                                color: lobby.textColor
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    root.lobbyDismissNoKeyboard()
                                    root.lobbyKeepFocus()
                                }
                            }
                        }
                    }
                }
            }
        }
