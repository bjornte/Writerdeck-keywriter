#ifndef LOBBY_UI_CONFIG_H
#define LOBBY_UI_CONFIG_H

#include <QHash>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariantMap>
#include <QFileSystemWatcher>

// On-disk Lobby look / copy / language / shortcut chords.
// Main file: /home/root/.Writerdeck/lobby-ui.json
// Packs: /home/root/.Writerdeck/lobby-ui-i18n/<lang>.json
// Hot-reloads via QFileSystemWatcher plus a short mtime poll.
class LobbyUiConfig : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int revision READ revision NOTIFY changed)
    Q_PROPERTY(QString language READ language NOTIFY changed)
    Q_PROPERTY(int btnBorder READ btnBorder NOTIFY changed)
    Q_PROPERTY(int btnBorderSelected READ btnBorderSelected NOTIFY changed)
    Q_PROPERTY(int shortcutBadgeMargin READ shortcutBadgeMargin NOTIFY changed)
    Q_PROPERTY(int pageMargin READ pageMargin NOTIFY changed)
    Q_PROPERTY(int tabBtnHeight READ tabBtnHeight NOTIFY changed)
    Q_PROPERTY(int rowHeight READ rowHeight NOTIFY changed)
    Q_PROPERTY(int actionBtnHeight READ actionBtnHeight NOTIFY changed)
    Q_PROPERTY(int tabSpacing READ tabSpacing NOTIFY changed)
    Q_PROPERTY(int contentSpacing READ contentSpacing NOTIFY changed)
    Q_PROPERTY(int labelPointSize READ labelPointSize NOTIFY changed)
    Q_PROPERTY(int badgePointSize READ badgePointSize NOTIFY changed)
    Q_PROPERTY(int titlePointSize READ titlePointSize NOTIFY changed)
    Q_PROPERTY(int sectionPointSize READ sectionPointSize NOTIFY changed)
    Q_PROPERTY(int rowPointSize READ rowPointSize NOTIFY changed)
    Q_PROPERTY(int dialogTitlePointSize READ dialogTitlePointSize NOTIFY changed)
    Q_PROPERTY(int bannerPointSize READ bannerPointSize NOTIFY changed)
    Q_PROPERTY(int helpPointSize READ helpPointSize NOTIFY changed)
    Q_PROPERTY(QString textColor READ textColor NOTIFY changed)
    Q_PROPERTY(QString borderColor READ borderColor NOTIFY changed)
    Q_PROPERTY(QString badgeTextColor READ badgeTextColor NOTIFY changed)
    Q_PROPERTY(QString badgeBorderColor READ badgeBorderColor NOTIFY changed)
    Q_PROPERTY(QString btnFill READ btnFill NOTIFY changed)
    Q_PROPERTY(QString btnFillSelected READ btnFillSelected NOTIFY changed)
    Q_PROPERTY(QString tabFill READ tabFill NOTIFY changed)
    Q_PROPERTY(QString tabFillSelected READ tabFillSelected NOTIFY changed)
    Q_PROPERTY(QString pageBg READ pageBg NOTIFY changed)
    Q_PROPERTY(QString dialogBg READ dialogBg NOTIFY changed)
    Q_PROPERTY(QString dialogScrim READ dialogScrim NOTIFY changed)
    Q_PROPERTY(QString vaultWash READ vaultWash NOTIFY changed)
    Q_PROPERTY(int btnRadius READ btnRadius NOTIFY changed)
    Q_PROPERTY(int dialogRadius READ dialogRadius NOTIFY changed)
    Q_PROPERTY(int badgeRadius READ badgeRadius NOTIFY changed)
    Q_PROPERTY(int bannerRadius READ bannerRadius NOTIFY changed)
    Q_PROPERTY(int pageStripHeight READ pageStripHeight NOTIFY changed)
    Q_PROPERTY(int listRowInset READ listRowInset NOTIFY changed)
    Q_PROPERTY(int tabRowExtraHeight READ tabRowExtraHeight NOTIFY changed)
    Q_PROPERTY(double dialogWidthFraction READ dialogWidthFraction NOTIFY changed)
    Q_PROPERTY(int dialogPadding READ dialogPadding NOTIFY changed)
    Q_PROPERTY(int settingsLandscapeScrollGutter READ settingsLandscapeScrollGutter NOTIFY changed)
    Q_PROPERTY(QVariantMap shortcuts READ shortcutsMap NOTIFY changed)
    Q_PROPERTY(QVariantMap strings READ stringsMap NOTIFY changed)

public:
    explicit LobbyUiConfig(QObject *parent = nullptr);

    void setPath(const QString &path);
    Q_INVOKABLE void reload();

    int revision() const { return m_revision; }
    QString language() const { return m_language; }
    int btnBorder() const { return m_btnBorder; }
    int btnBorderSelected() const { return m_btnBorderSelected; }
    int shortcutBadgeMargin() const { return m_shortcutBadgeMargin; }
    int pageMargin() const { return m_pageMargin; }
    int tabBtnHeight() const { return m_tabBtnHeight; }
    int rowHeight() const { return m_rowHeight; }
    int actionBtnHeight() const { return m_actionBtnHeight; }
    int tabSpacing() const { return m_tabSpacing; }
    int contentSpacing() const { return m_contentSpacing; }
    int labelPointSize() const { return m_labelPointSize; }
    int badgePointSize() const { return m_badgePointSize; }
    int titlePointSize() const { return m_titlePointSize; }
    int sectionPointSize() const { return m_sectionPointSize; }
    int rowPointSize() const { return m_rowPointSize; }
    int dialogTitlePointSize() const { return m_dialogTitlePointSize; }
    int bannerPointSize() const { return m_bannerPointSize; }
    int helpPointSize() const { return m_helpPointSize; }
    QString textColor() const { return m_textColor; }
    QString borderColor() const { return m_borderColor; }
    QString badgeTextColor() const { return m_badgeTextColor; }
    QString badgeBorderColor() const { return m_badgeBorderColor; }
    QString btnFill() const { return m_btnFill; }
    QString btnFillSelected() const { return m_btnFillSelected; }
    QString tabFill() const { return m_tabFill; }
    QString tabFillSelected() const { return m_tabFillSelected; }
    QString pageBg() const { return m_pageBg; }
    QString dialogBg() const { return m_dialogBg; }
    QString dialogScrim() const { return m_dialogScrim; }
    QString vaultWash() const { return m_vaultWash; }
    int btnRadius() const { return m_btnRadius; }
    int dialogRadius() const { return m_dialogRadius; }
    int badgeRadius() const { return m_badgeRadius; }
    int bannerRadius() const { return m_bannerRadius; }
    int pageStripHeight() const { return m_pageStripHeight; }
    int listRowInset() const { return m_listRowInset; }
    int tabRowExtraHeight() const { return m_tabRowExtraHeight; }
    double dialogWidthFraction() const { return m_dialogWidthFraction; }
    int dialogPadding() const { return m_dialogPadding; }
    int settingsLandscapeScrollGutter() const { return m_settingsLandscapeScrollGutter; }
    QVariantMap shortcutsMap() const;
    QVariantMap stringsMap() const;

    Q_INVOKABLE QString str(const QString &key) const;
    // Replace %1/%2/%3 in the string for key (empty args leave later placeholders alone).
    Q_INVOKABLE QString strf(const QString &key, const QString &a1,
                             const QString &a2 = QString(), const QString &a3 = QString()) const;
    Q_INVOKABLE QString shortcut(const QString &key) const;
    Q_INVOKABLE QString shortcutBadge(const QString &key) const;
    Q_INVOKABLE QString actionForLetter(const QString &letter) const;

signals:
    void changed();

private slots:
    void onFileChanged(const QString &path);
    void pollDisk();

private:
    void applyDefaults();
    void parseObject(const QJsonObject &root, bool mergeStrings);
    void mergeStringsObject(const QJsonObject &strings);
    bool loadLanguagePack(const QString &lang);
    void rebuildLetterIndex();
    void watchPath();
    void noteDiskStamp();

    QFileSystemWatcher m_watch;
    QTimer m_poll;
    QString m_path;
    int m_revision = 0;
    qint64 m_diskMtimeMs = -1;
    qint64 m_diskSize = -1;
    bool m_reloadPending = false;

    QString m_language = QStringLiteral("en");
    int m_btnBorder = 2;
    int m_btnBorderSelected = 4;
    int m_shortcutBadgeMargin = 8;
    int m_pageMargin = 24;
    int m_tabBtnHeight = 64;
    int m_rowHeight = 72;
    int m_actionBtnHeight = 72;
    int m_tabSpacing = 16;
    int m_contentSpacing = 12;
    int m_labelPointSize = 11;
    int m_badgePointSize = 9;
    int m_titlePointSize = 26;
    int m_sectionPointSize = 12;
    int m_rowPointSize = 14;
    int m_dialogTitlePointSize = 16;
    int m_bannerPointSize = 16;
    int m_helpPointSize = 10;
    QString m_textColor = QStringLiteral("#000000");
    QString m_borderColor = QStringLiteral("#000000");
    QString m_badgeTextColor = QStringLiteral("#000000");
    QString m_badgeBorderColor = QStringLiteral("#000000");
    QString m_btnFill = QStringLiteral("#f0f0f0");
    QString m_btnFillSelected = QStringLiteral("#e8e8e8");
    QString m_tabFill = QStringLiteral("#f5f5f5");
    QString m_tabFillSelected = QStringLiteral("#e0e0e0");
    QString m_pageBg = QStringLiteral("#ffffff");
    QString m_dialogBg = QStringLiteral("#ffffff");
    QString m_dialogScrim = QStringLiteral("#dddddd");
    QString m_vaultWash = QStringLiteral("#f8f8f8");
    int m_btnRadius = 6;
    int m_dialogRadius = 8;
    int m_badgeRadius = 3;
    int m_bannerRadius = 4;
    int m_pageStripHeight = 48;
    int m_listRowInset = 8;
    int m_tabRowExtraHeight = 8;
    double m_dialogWidthFraction = 0.85;
    int m_dialogPadding = 48;
    int m_settingsLandscapeScrollGutter = 144;

    QHash<QString, QString> m_strings;
    QHash<QString, QString> m_shortcuts;
    QHash<QString, QString> m_letterToAction;
};

#endif
