#include "lobby_ui_config.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTimer>

namespace {

// ASCII-only embedded fallback (device still runs if lobby-ui.json / packs are missing).
static const char kDefaultJson[] =
    "{\"language\":\"en\",\"visual\":{\"btnBorder\":2,\"btnBorderSelected\":4,\"shortcutBadgeMargin\":8,\"pageMargin\":24,\"tabBtnHeight\":64,\"rowHeight\":72,\"actionBtnHeight\":72,\"tabSpacing\":16,\"contentSpacing\":12,\"labelPointSize\":11,\"badgePointSize\":9,\"titlePointSize\":26,\"sectionPointSize\":12,\"rowPointSize\":14,\"dialogTitlePointSize\":16,\"bannerPointSize\":16,\"helpPointSize\":10,\"textColor\":\"#000000\",\"borderColor\":\"#000000\",\"badgeTextColor\":\"#000000\",\"badgeBorderColor\":\"#000000\",\"btnFill\":\"#f0f0f0\",\"btnFillSelected\":\"#e8e8e8\",\"tabFill\":\"#f5f5f5\",\"tabFillSelected\":\"#e0e0e0\",\"pageBg\":\"#ffffff\",\"dialogBg\":\"#ffffff\",\"dialogScrim\":\"#dddddd\",\"vaultWash\":\"#f8f8f8\",\"btnRadius\":6,\"dialogRadius\":8,\"badgeRadius\":3,\"bannerRadius\":4,\"pageStripHeight\":48,\"listRowInset\":8,\"tabRowExtraHeight\":8,\"dialogWidthFraction\":0.85,\"dialogPadding\":48,\"settingsLandscapeScrollGutter\":144,\"fontPickerNamePointSize\":16,\"fontPickerSamplePointSize\":14,\"fontPickerRowExtra\":96,\"dialogCancelWidthFraction\":0.55},\"strings\":{\"tabs.files\":\"Documents\",\"tabs.keyboard\":\"Keyboard\",\"tabs.sync\":\"Sync\",\"tabs.settings\":\"Settings\",\"tabs.shortcuts\":\"Shortcuts\",\"tabs.about\":\"About\",\"files.prev\":\"Prev\",\"files.next\":\"Next\",\"files.page\":\"Page %1/%2\",\"files.new\":\"New\",\"files.edit\":\"Edit\",\"files.read\":\"Read\",\"files.rename\":\"Rename\",\"files.delete\":\"Delete\",\"files.download\":\"Download\",\"files.encrypt\":\"Encrypt\",\"files.newEncrypted\":\"New encrypted\",\"files.decrypt\":\"Decrypt\",\"files.marker\":\"\\u25b6  \",\"files.markerPad\":\"   \",\"files.privateSuffix\":\" [private]\",\"files.tapDismiss\":\"Tap to dismiss\",\"files.editBadge\":\"\\u21b5\",\"files.nameExists\":\"A document with that name already exists.\",\"files.operationFailed\":\"Operation failed\",\"files.openFailed\":\"Could not open document\",\"files.decryptCorrupt\":\"Cannot decrypt: wrong vault key or corrupted file\",\"files.downloadNeedPhone\":\"Open the phone page, then try Download again.\",\"home.brand\":\"Writerdeck\",\"home.tagline\":\"A text editor for use with a physical keyboard.\\nWith Markdown support.\",\"home.docsOne\":\"1 document on this device.\",\"home.docsMany\":\"%1 documents on this device.\",\"home.versionChecking\":\"Writerdeck version \\u2026 (checking GitHub for updates\\u2026)\",\"home.versionUnknown\":\"Writerdeck version unknown\",\"home.opensource\":\"Open sourced at github.com/bjornte/Writerdeck-for-reMarkable\",\"home.tip\":\"Open the Documents tab.\\nUse Tab or Left / Right to switch pages.\",\"home.sleeping\":\"Writerdeck is sleeping.\\nWi-Fi is off. Press power to wake.\",\"keyboard.btTitle\":\"Bluetooth keyboard\",\"keyboard.usbTitle\":\"USB keyboard\",\"keyboard.connected\":\" (connected)\",\"keyboard.notConnected\":\" (not connected)\",\"keyboard.btBody\":\"Pair the keyboard to your phone, then open the address below (or scan the code). Typing is forwarded over Wi-Fi.\",\"keyboard.usbBody\":\"Connect with a USB OTG cable.\\nChanging layout restarts Writerdeck.\",\"keyboard.layout\":\"Layout\",\"keyboard.pinPrefix\":\"PIN: \",\"keyboard.us\":\"US QWERTY\",\"keyboard.no\":\"Norwegian\",\"sync.title\":\"GitHub sync\",\"sync.lastSync\":\"Last sync was %1.\\nDocuments sync to github.com/%2\",\"sync.syncingTo\":\"Documents sync to github.com/%1\",\"sync.notConfigured\":\"Sync not configured.\\nSet up in phone Sync setup:\\nhttp://%1:%2\",\"sync.offline\":\"SYNC OFFLINE\",\"sync.failed\":\"SYNC FAILED\",\"sync.tokenNeeded\":\"TOKEN NEEDED\",\"sync.tokenBody\":\"GitHub token is not on the tablet.\\nOpen phone Sync setup and tap Save:\\nhttp://%1:%2\\nRepo: github.com/%3\",\"sync.now\":\"Sync now\",\"sync.syncing\":\"Syncing\\u2026\",\"sync.tokenBtn\":\"Token needed \\u2014 phone Sync setup\",\"sync.footnote\":\"Sync also runs automatically on save, Home, and every few minutes.\",\"settings.title\":\"Settings\",\"settings.fontSection\":\"\\nReading font\",\"settings.fontHelp\":\"Ctrl-A - cycle through fonts.\",\"settings.privateSection\":\"\\nPrivate documents\",\"settings.privateOn\":\"On - encrypted documents require PIN to open, read, or edit\",\"settings.privateOff\":\"Off - optional encryption with a separate 6-digit PIN. Recovery via GitHub secret/pin when sync is on.\",\"settings.enable\":\"Enable\",\"settings.changePin\":\"Change PIN\",\"settings.pinSection\":\"\\nPIN for phone pairing\",\"settings.pinHelp\":\"Ctrl-M - cycle PIN length. Adding a PIN ensures that only intended devices can access your documents.\",\"settings.pin6\":\"6 digits\",\"settings.pin4\":\"4 digits\",\"settings.pinNone\":\"No PIN\",\"settings.pinNoneWarn\":\"Anyone on Wi-Fi can read and edit documents\",\"settings.rotationSection\":\"\\nDisplay rotation\",\"settings.rotationHelp\":\"Ctrl-O - cycle rotation.\",\"settings.rot0\":\"0\\u00b0\",\"settings.rot90\":\"90\\u00b0\",\"settings.rot180\":\"180\\u00b0\",\"settings.rot270\":\"270\\u00b0\",\"settings.serviceSection\":\"\\nService\",\"settings.serviceHelp\":\"Stop Writerdeck and return the tablet to the stock reMarkable UI. Reconnect later via SSH or reboot.\",\"settings.exit\":\"Exit Writerdeck\",\"settings.confirmExit\":\"Stop Writerdeck? Enter=yes  Esc=no\",\"dialog.connectTitle\":\"Connect a keyboard\",\"dialog.deleteTitle\":\"Delete this document?\",\"dialog.newTitle\":\"New document\",\"dialog.renameTitle\":\"Rename document\",\"dialog.newEncryptedTitle\":\"New encrypted document\",\"dialog.noKeyboardBody\":\"USB: plug in with an OTG cable.\\n\\nBluetooth: pair to your phone, then open the address below (or scan the code).\",\"dialog.cancel\":\"Cancel\",\"dialog.delete\":\"Delete\",\"dialog.create\":\"Create\",\"dialog.rename\":\"Rename\",\"dialog.pinPrefix\":\"PIN: \",\"vault.setup\":\"Choose a 6-digit private PIN\",\"vault.confirm\":\"Confirm private PIN\",\"vault.changeOld\":\"Enter current private PIN\",\"vault.changeNew\":\"Enter new private PIN\",\"vault.changeConfirm\":\"Confirm new private PIN\",\"vault.enter\":\"Enter private PIN\",\"vault.open\":\"Enter PIN to open this document\",\"vault.edit\":\"Enter PIN to edit encrypted document\",\"vault.read\":\"Enter PIN to read encrypted document\",\"vault.create\":\"Enter PIN to create encrypted document\",\"vault.encrypt\":\"Enter PIN to encrypt document\",\"vault.decrypt\":\"Enter PIN to decrypt document\",\"vault.download\":\"Enter PIN on tablet to allow phone download\",\"vault.downloadTitle\":\"Phone download: %1\",\"vault.downloadFallbackName\":\"encrypted document\",\"vault.wrongPin\":\"Wrong PIN. Try again.\",\"vault.bksp\":\"Bksp\",\"shortcuts.title\":\"Shortcuts\",\"shortcuts.body\":\"Pages\\nTab or Left / Right - next or previous page\\nOptional Ctrl-letter jumps live in lobby-ui.json (tabs.*); none ship by default\\n\\nDocuments\\nUp / Down - move the selection (turns the page at the edge)\\nPage Up / Page Down - previous or next page of documents\\n\\u21b5 - edit the selected document\\nCtrl-V - read \\u00b7 Ctrl-M - new \\u00b7 Ctrl-I - rename \\u00b7 Ctrl-B - delete\\nCtrl-G - download to phone\\nWith private documents on: Ctrl-E - new encrypted \\u00b7 Ctrl-X - encrypt \\u00b7 Ctrl-Y - decrypt\\n\\nKeyboard\\nCtrl-U - US layout \\u00b7 Ctrl-O - Norwegian \\u00b7 Ctrl-E - Spanish \\u00b7 Ctrl-D - German \\u00b7 Ctrl-F - French\\n\\nSync\\n\\u21b5 - sync now\\n\\nSettings\\nCtrl-A - cycle reading font \\u00b7 Ctrl-M - cycle phone PIN length\\nCtrl-O - cycle rotation \\u00b7 Ctrl-E - enable private documents \\u00b7 Ctrl-C - change private PIN\\nCtrl-X - exit Writerdeck (then Enter to confirm)\\n\\nAnywhere\\nCtrl-C / Ctrl-X / Ctrl-V - copy, cut, paste (while editing a document)\\nPhysical Home - from a document, back to Documents; from Lobby, quit to the stock UI\\n(Keyboard Home is caret motion only -- not that button.)\\n\\nPrivate PIN\\nType the six digits on a USB keyboard, or on the phone while it shows the PIN banner.\\n\\nOpen Lobby from the stock UI\\nEsc on a USB keyboard, or both page buttons together.\",\"sync.agoJustNow\":\"just now\",\"sync.agoMinute\":\"1 minute ago\",\"sync.agoMinutes\":\"%1 minutes ago\",\"sync.agoHour\":\"1 hour ago\",\"sync.agoHours\":\"%1 hours ago\",\"sync.agoDay\":\"1 day ago\",\"sync.agoDays\":\"%1 days ago\",\"sync.pendingOne\":\"sync pending\",\"sync.pendingMany\":\"%1 sync ops pending\",\"sync.agoWithPending\":\"%1 (%2)\",\"sync.errNoWifi\":\"No Wi-Fi - cannot reach GitHub\",\"home.versionLatest\":\"Writerdeck version %1 (latest)\",\"home.versionOutdated\":\"Writerdeck version %1. Latest on GitHub is %2.\",\"home.versionAhead\":\"Writerdeck version %1 (newer than GitHub %2)\",\"home.versionMismatch\":\"Writerdeck version %1 (server and editor differ \\u2014 update both)\",\"home.versionMismatchLatest\":\"Writerdeck version %1 (server and editor differ \\u2014 update both). Latest on GitHub is %2.\",\"home.versionOffline\":\"Writerdeck version %1 (couldn't reach GitHub to check for updates)\",\"keyboard.es\":\"Spanish\",\"keyboard.de\":\"German\",\"keyboard.fr\":\"French\"},\"shortcuts\":{\"tabs.files\":\"\",\"tabs.keyboard\":\"\",\"tabs.sync\":\"\",\"tabs.settings\":\"\",\"tabs.shortcuts\":\"\",\"tabs.about\":\"\",\"global.toLobby\":\"hardware_home\",\"global.quit\":\"hardware_home\",\"files.edit\":\"enter\",\"files.new\":\"m\",\"files.read\":\"v\",\"files.rename\":\"i\",\"files.delete\":\"b\",\"files.download\":\"g\",\"files.encrypt\":\"x\",\"files.decrypt\":\"y\",\"files.newEncrypted\":\"e\",\"keyboard.us\":\"u\",\"keyboard.no\":\"o\",\"sync.now\":\"enter\",\"settings.language\":\"l\",\"settings.font\":\"a\",\"settings.pin\":\"m\",\"settings.rotation\":\"o\",\"settings.enableVault\":\"e\",\"settings.changePin\":\"c\",\"settings.exit\":\"x\",\"keyboard.es\":\"e\",\"keyboard.de\":\"d\",\"keyboard.fr\":\"f\"}}";

static int jsonInt(const QJsonObject &o, const char *key, int fallback)
{
    const QJsonValue v = o.value(QLatin1String(key));
    if (v.isDouble())
        return v.toInt();
    return fallback;
}

static double jsonDouble(const QJsonObject &o, const char *key, double fallback)
{
    const QJsonValue v = o.value(QLatin1String(key));
    if (v.isDouble())
        return v.toDouble();
    return fallback;
}

static QString jsonStr(const QJsonObject &o, const char *key, const QString &fallback)
{
    const QJsonValue v = o.value(QLatin1String(key));
    if (v.isString())
        return v.toString();
    return fallback;
}

} // namespace

LobbyUiConfig::LobbyUiConfig(QObject *parent)
    : QObject(parent)
{
    applyDefaults();
    connect(&m_watch, &QFileSystemWatcher::fileChanged,
            this, &LobbyUiConfig::onFileChanged);
    connect(&m_watch, &QFileSystemWatcher::directoryChanged,
            this, &LobbyUiConfig::onFileChanged);
    m_poll.setInterval(1500);
    connect(&m_poll, &QTimer::timeout, this, &LobbyUiConfig::pollDisk);
}

void LobbyUiConfig::setPath(const QString &path)
{
    m_path = path;
    reload();
    if (!m_poll.isActive())
        m_poll.start();
}

void LobbyUiConfig::reload()
{
    const QJsonDocument embedded = QJsonDocument::fromJson(QByteArray(kDefaultJson));

    // Prefer on-disk JSON. Corrupt/missing after a good load keeps the last good
    // values so a bad SSH edit does not blank the Lobby.
    if (!m_path.isEmpty()) {
        QFile f(m_path);
        if (f.open(QIODevice::ReadOnly)) {
            const QByteArray raw = f.readAll();
            f.close();
            QJsonParseError err;
            const QJsonDocument doc = QJsonDocument::fromJson(raw, &err);
            if (err.error == QJsonParseError::NoError && doc.isObject()) {
                applyDefaults();
                if (embedded.isObject())
                    parseObject(embedded.object(), true);
                // Disk main file: visual + language + shortcuts; strings overlay last.
                const QJsonObject root = doc.object();
                QString lang = jsonStr(root, "language", m_language).trimmed().toLower();
                if (lang.isEmpty())
                    lang = QStringLiteral("en");
                m_language = lang;
                parseObject(root, false);
                loadLanguagePack(m_language);
                // Per-device string overrides in lobby-ui.json win last.
                if (root.contains(QLatin1String("strings")))
                    mergeStringsObject(root.value(QLatin1String("strings")).toObject());
                noteDiskStamp();
                watchPath();
                ++m_revision;
                qWarning("lobby-ui: loaded %s lang=%s (rev %d)",
                         qPrintable(m_path), qPrintable(m_language), m_revision);
                emit changed();
                return;
            }
            qWarning("lobby-ui.json parse error: %s", qPrintable(err.errorString()));
            if (m_revision > 0) {
                noteDiskStamp();
                watchPath();
                return;
            }
        } else if (m_revision > 0) {
            qWarning("lobby-ui: keep last good load (%s unreadable)", qPrintable(m_path));
            noteDiskStamp();
            watchPath();
            return;
        }
    }

    applyDefaults();
    if (embedded.isObject())
        parseObject(embedded.object(), true);
    if (!m_path.isEmpty())
        qWarning("lobby-ui: using embedded defaults (%s missing or invalid)",
                 qPrintable(m_path));
    noteDiskStamp();
    watchPath();
    ++m_revision;
    emit changed();
}

void LobbyUiConfig::applyDefaults()
{
    m_language = QStringLiteral("en");
    m_btnBorder = 2;
    m_btnBorderSelected = 4;
    m_shortcutBadgeMargin = 8;
    m_pageMargin = 24;
    m_tabBtnHeight = 64;
    m_rowHeight = 72;
    m_actionBtnHeight = 72;
    m_tabSpacing = 16;
    m_contentSpacing = 12;
    m_labelPointSize = 11;
    m_badgePointSize = 9;
    m_titlePointSize = 26;
    m_sectionPointSize = 12;
    m_rowPointSize = 14;
    m_dialogTitlePointSize = 16;
    m_bannerPointSize = 16;
    m_helpPointSize = 10;
    m_textColor = QStringLiteral("#000000");
    m_borderColor = QStringLiteral("#000000");
    m_badgeTextColor = QStringLiteral("#000000");
    m_badgeBorderColor = QStringLiteral("#000000");
    m_btnFill = QStringLiteral("#f0f0f0");
    m_btnFillSelected = QStringLiteral("#e8e8e8");
    m_tabFill = QStringLiteral("#f5f5f5");
    m_tabFillSelected = QStringLiteral("#e0e0e0");
    m_pageBg = QStringLiteral("#ffffff");
    m_dialogBg = QStringLiteral("#ffffff");
    m_dialogScrim = QStringLiteral("#dddddd");
    m_vaultWash = QStringLiteral("#f8f8f8");
    m_btnRadius = 6;
    m_dialogRadius = 8;
    m_badgeRadius = 3;
    m_bannerRadius = 4;
    m_pageStripHeight = 48;
    m_listRowInset = 8;
    m_tabRowExtraHeight = 8;
    m_dialogWidthFraction = 0.85;
    m_dialogPadding = 48;
    m_settingsLandscapeScrollGutter = 144;
    m_fontPickerNamePointSize = 16;
    m_fontPickerSamplePointSize = 14;
    m_fontPickerRowExtra = 96;
    m_dialogCancelWidthFraction = 0.55;
    m_strings.clear();
    m_shortcuts.clear();
    m_letterToAction.clear();
}

void LobbyUiConfig::mergeStringsObject(const QJsonObject &strings)
{
    for (auto it = strings.begin(); it != strings.end(); ++it) {
        if (it.value().isString())
            m_strings.insert(it.key(), it.value().toString());
    }
}

bool LobbyUiConfig::loadLanguagePack(const QString &lang)
{
    if (m_path.isEmpty() || lang.isEmpty())
        return false;
    const QString packPath = QFileInfo(m_path).absolutePath()
            + QStringLiteral("/lobby-ui-i18n/") + lang + QStringLiteral(".json");
    QFile f(packPath);
    if (!f.open(QIODevice::ReadOnly)) {
        if (lang != QLatin1String("en"))
            qWarning("lobby-ui: language pack missing %s", qPrintable(packPath));
        return false;
    }
    const QByteArray raw = f.readAll();
    f.close();
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(raw, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning("lobby-ui i18n parse error (%s): %s",
                 qPrintable(packPath), qPrintable(err.errorString()));
        return false;
    }
    mergeStringsObject(doc.object());
    // Watch the pack file too so SSH edits reload.
    if (!m_watch.files().contains(packPath) && QFile::exists(packPath))
        m_watch.addPath(packPath);
    const QString i18nDir = QFileInfo(packPath).absolutePath();
    if (!i18nDir.isEmpty() && QDir(i18nDir).exists()
            && !m_watch.directories().contains(i18nDir))
        m_watch.addPath(i18nDir);
    return true;
}

void LobbyUiConfig::parseObject(const QJsonObject &root, bool mergeStrings)
{
    if (root.contains(QLatin1String("language"))) {
        const QString lang = jsonStr(root, "language", m_language).trimmed().toLower();
        if (!lang.isEmpty())
            m_language = lang;
    }

    const QJsonObject visual = root.value(QLatin1String("visual")).toObject();
    if (!visual.isEmpty()) {
        m_btnBorder = jsonInt(visual, "btnBorder", m_btnBorder);
        m_btnBorderSelected = jsonInt(visual, "btnBorderSelected", m_btnBorderSelected);
        m_shortcutBadgeMargin = jsonInt(visual, "shortcutBadgeMargin", m_shortcutBadgeMargin);
        m_pageMargin = jsonInt(visual, "pageMargin", m_pageMargin);
        m_tabBtnHeight = jsonInt(visual, "tabBtnHeight", m_tabBtnHeight);
        m_rowHeight = jsonInt(visual, "rowHeight", m_rowHeight);
        m_actionBtnHeight = jsonInt(visual, "actionBtnHeight", m_actionBtnHeight);
        m_tabSpacing = jsonInt(visual, "tabSpacing", m_tabSpacing);
        m_contentSpacing = jsonInt(visual, "contentSpacing", m_contentSpacing);
        m_labelPointSize = jsonInt(visual, "labelPointSize", m_labelPointSize);
        m_badgePointSize = jsonInt(visual, "badgePointSize", m_badgePointSize);
        m_titlePointSize = jsonInt(visual, "titlePointSize", m_titlePointSize);
        m_sectionPointSize = jsonInt(visual, "sectionPointSize", m_sectionPointSize);
        m_rowPointSize = jsonInt(visual, "rowPointSize", m_rowPointSize);
        m_dialogTitlePointSize = jsonInt(visual, "dialogTitlePointSize", m_dialogTitlePointSize);
        m_bannerPointSize = jsonInt(visual, "bannerPointSize", m_bannerPointSize);
        m_helpPointSize = jsonInt(visual, "helpPointSize", m_helpPointSize);
        m_textColor = jsonStr(visual, "textColor", m_textColor);
        m_borderColor = jsonStr(visual, "borderColor", m_borderColor);
        m_badgeTextColor = jsonStr(visual, "badgeTextColor", m_badgeTextColor);
        m_badgeBorderColor = jsonStr(visual, "badgeBorderColor", m_badgeBorderColor);
        m_btnFill = jsonStr(visual, "btnFill", m_btnFill);
        m_btnFillSelected = jsonStr(visual, "btnFillSelected", m_btnFillSelected);
        m_tabFill = jsonStr(visual, "tabFill", m_tabFill);
        m_tabFillSelected = jsonStr(visual, "tabFillSelected", m_tabFillSelected);
        m_pageBg = jsonStr(visual, "pageBg", m_pageBg);
        m_dialogBg = jsonStr(visual, "dialogBg", m_dialogBg);
        m_dialogScrim = jsonStr(visual, "dialogScrim", m_dialogScrim);
        m_vaultWash = jsonStr(visual, "vaultWash", m_vaultWash);
        m_btnRadius = jsonInt(visual, "btnRadius", m_btnRadius);
        m_dialogRadius = jsonInt(visual, "dialogRadius", m_dialogRadius);
        m_badgeRadius = jsonInt(visual, "badgeRadius", m_badgeRadius);
        m_bannerRadius = jsonInt(visual, "bannerRadius", m_bannerRadius);
        m_pageStripHeight = jsonInt(visual, "pageStripHeight", m_pageStripHeight);
        m_listRowInset = jsonInt(visual, "listRowInset", m_listRowInset);
        m_tabRowExtraHeight = jsonInt(visual, "tabRowExtraHeight", m_tabRowExtraHeight);
        m_dialogWidthFraction = jsonDouble(visual, "dialogWidthFraction", m_dialogWidthFraction);
        m_dialogPadding = jsonInt(visual, "dialogPadding", m_dialogPadding);
        m_settingsLandscapeScrollGutter = jsonInt(visual, "settingsLandscapeScrollGutter",
                                                 m_settingsLandscapeScrollGutter);
        m_fontPickerNamePointSize = jsonInt(visual, "fontPickerNamePointSize",
                                            m_fontPickerNamePointSize);
        m_fontPickerSamplePointSize = jsonInt(visual, "fontPickerSamplePointSize",
                                              m_fontPickerSamplePointSize);
        m_fontPickerRowExtra = jsonInt(visual, "fontPickerRowExtra", m_fontPickerRowExtra);
        m_dialogCancelWidthFraction = jsonDouble(visual, "dialogCancelWidthFraction",
                                                 m_dialogCancelWidthFraction);
    }

    if (mergeStrings && root.contains(QLatin1String("strings")))
        mergeStringsObject(root.value(QLatin1String("strings")).toObject());

    const QJsonObject shortcuts = root.value(QLatin1String("shortcuts")).toObject();
    if (root.contains(QLatin1String("shortcuts"))) {
        for (auto it = shortcuts.begin(); it != shortcuts.end(); ++it) {
            if (!it.value().isString())
                continue;
            QString value = it.value().toString().trimmed().toLower();
            if (value.isEmpty()) {
                m_shortcuts.insert(it.key(), QString());
                continue;
            }
            if (value == QLatin1String("enter")
                    || value == QLatin1String("hardware_home")) {
                m_shortcuts.insert(it.key(), value);
                continue;
            }
            if (value.size() == 1 && value[0] >= QLatin1Char('a')
                    && value[0] <= QLatin1Char('z')) {
                m_shortcuts.insert(it.key(), value);
                continue;
            }
        }
    }
    rebuildLetterIndex();
}

void LobbyUiConfig::rebuildLetterIndex()
{
    m_letterToAction.clear();
    for (auto it = m_shortcuts.constBegin(); it != m_shortcuts.constEnd(); ++it) {
        const QString &v = it.value();
        if (v.size() == 1 && v[0] >= QLatin1Char('a') && v[0] <= QLatin1Char('z'))
            m_letterToAction.insert(v, it.key());
    }
}

void LobbyUiConfig::watchPath()
{
    const QStringList curFiles = m_watch.files();
    if (!curFiles.isEmpty())
        m_watch.removePaths(curFiles);
    const QStringList curDirs = m_watch.directories();
    if (!curDirs.isEmpty())
        m_watch.removePaths(curDirs);
    if (m_path.isEmpty())
        return;
    if (QFile::exists(m_path))
        m_watch.addPath(m_path);
    const QString dir = QFileInfo(m_path).absolutePath();
    if (!dir.isEmpty() && QDir(dir).exists())
        m_watch.addPath(dir);
    const QString i18nDir = dir + QStringLiteral("/lobby-ui-i18n");
    if (QDir(i18nDir).exists())
        m_watch.addPath(i18nDir);
    const QString pack = i18nDir + QLatin1Char('/') + m_language + QStringLiteral(".json");
    if (QFile::exists(pack))
        m_watch.addPath(pack);
}

void LobbyUiConfig::onFileChanged(const QString &path)
{
    Q_UNUSED(path);
    watchPath();
    if (m_reloadPending)
        return;
    m_reloadPending = true;
    QTimer::singleShot(250, this, [this]() {
        m_reloadPending = false;
        reload();
    });
}

void LobbyUiConfig::noteDiskStamp()
{
    if (m_path.isEmpty()) {
        m_diskMtimeMs = -1;
        m_diskSize = -1;
        return;
    }
    const QFileInfo fi(m_path);
    if (!fi.exists()) {
        m_diskMtimeMs = -1;
        m_diskSize = -1;
        return;
    }
    m_diskMtimeMs = fi.lastModified().toMSecsSinceEpoch();
    m_diskSize = fi.size();
}

void LobbyUiConfig::pollDisk()
{
    if (m_path.isEmpty() || m_reloadPending)
        return;
    const QFileInfo fi(m_path);
    if (!fi.exists())
        return;
    const qint64 m = fi.lastModified().toMSecsSinceEpoch();
    const qint64 sz = fi.size();
    if (m == m_diskMtimeMs && sz == m_diskSize)
        return;
    reload();
}

QVariantMap LobbyUiConfig::shortcutsMap() const
{
    QVariantMap m;
    for (auto it = m_shortcuts.constBegin(); it != m_shortcuts.constEnd(); ++it)
        m.insert(it.key(), it.value());
    return m;
}

QVariantMap LobbyUiConfig::stringsMap() const
{
    QVariantMap m;
    for (auto it = m_strings.constBegin(); it != m_strings.constEnd(); ++it)
        m.insert(it.key(), it.value());
    return m;
}

bool LobbyUiConfig::setLanguage(const QString &langIn)
{
    const QString lang = langIn.trimmed().toLower();
    if (lang != QLatin1String("en") && lang != QLatin1String("no")
            && lang != QLatin1String("es") && lang != QLatin1String("de")
            && lang != QLatin1String("fr")) {
        qWarning("lobby-ui: setLanguage rejected %s", qPrintable(lang));
        return false;
    }
    if (m_path.isEmpty()) {
        qWarning("lobby-ui: setLanguage: no path");
        return false;
    }

    QFile in(m_path);
    if (!in.open(QIODevice::ReadOnly)) {
        qWarning("lobby-ui: setLanguage: cannot read %s", qPrintable(m_path));
        return false;
    }
    const QByteArray raw = in.readAll();
    in.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(raw, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning("lobby-ui: setLanguage: bad JSON in %s", qPrintable(m_path));
        return false;
    }

    QJsonObject root = doc.object();
    root.insert(QStringLiteral("language"), lang);
    const QByteArray outBytes = QJsonDocument(root).toJson(QJsonDocument::Indented);

    // Write beside the live file, then rename (avoids a half-written config).
    const QString tmpPath = m_path + QStringLiteral(".tmp");
    QFile out(tmpPath);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning("lobby-ui: setLanguage: cannot write %s", qPrintable(tmpPath));
        return false;
    }
    if (out.write(outBytes) != outBytes.size()) {
        out.close();
        QFile::remove(tmpPath);
        qWarning("lobby-ui: setLanguage: short write");
        return false;
    }
    out.close();
    if (QFile::exists(m_path) && !QFile::remove(m_path)) {
        QFile::remove(tmpPath);
        qWarning("lobby-ui: setLanguage: cannot replace %s", qPrintable(m_path));
        return false;
    }
    if (!QFile::rename(tmpPath, m_path)) {
        qWarning("lobby-ui: setLanguage: rename failed");
        return false;
    }

    reload();
    return m_language == lang;
}

QString LobbyUiConfig::str(const QString &key) const
{
    return m_strings.value(key);
}

QString LobbyUiConfig::strf(const QString &key, const QString &a1,
                            const QString &a2, const QString &a3) const
{
    const QString s = m_strings.value(key);
    if (!a3.isEmpty())
        return s.arg(a1, a2, a3);
    if (!a2.isEmpty())
        return s.arg(a1, a2);
    return s.arg(a1);
}

QString LobbyUiConfig::shortcut(const QString &key) const
{
    return m_shortcuts.value(key);
}

QString LobbyUiConfig::shortcutBadge(const QString &key) const
{
    const QString v = m_shortcuts.value(key);
    if (v == QLatin1String("enter"))
        return QString(QChar(0x21b5));
    if (v.isEmpty() || v == QLatin1String("hardware_home"))
        return QString();
    return v;
}

QString LobbyUiConfig::actionForLetter(const QString &letter) const
{
    return m_letterToAction.value(letter.toLower());
}
