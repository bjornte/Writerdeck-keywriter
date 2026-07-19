#include "lobby_ui_config.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTimer>

namespace {

// ASCII-only embedded fallback (device still runs if lobby-ui.json is missing).
static const char kDefaultJson[] =
    "{"
    "\"visual\":{"
    "\"btnBorder\":2,\"btnBorderSelected\":4,\"shortcutBadgeMargin\":4,"
    "\"pageMargin\":24,\"tabBtnHeight\":64,\"rowHeight\":72,\"actionBtnHeight\":72,"
    "\"tabSpacing\":12,\"contentSpacing\":12,\"labelPointSize\":11,\"badgePointSize\":9,"
    "\"textColor\":\"#000000\",\"borderColor\":\"#000000\",\"badgeTextColor\":\"#000000\""
    "},"
    "\"strings\":{"
    "\"files.editBadge\":\"\\u21b5\","
    "\"settings.fontHelp\":\"Ctrl-F - cycle through fonts.\","
    "\"settings.pinHelp\":\"Ctrl-P - cycle PIN length. Adding a PIN ensures that only intended devices can access your notes.\","
    "\"settings.rotationHelp\":\"Ctrl-T - cycle. Ctrl-R or Ctrl+arrows also rotate.\","
    "\"settings.privateOn\":\"On - encrypted notes require PIN to open, read, or edit\","
    "\"settings.privateOff\":\"Off - optional encryption with a separate 6-digit PIN. Recovery via GitHub secret/pin when sync is on.\","
    "\"settings.serviceHelp\":\"Stop Writerdeck and return the tablet to the stock reMarkable UI. Reconnect later via SSH or reboot.\","
    "\"settings.confirmExit\":\"Stop Writerdeck? Enter=yes  Esc=no\","
    "\"dialog.connectTitle\":\"Connect a keyboard\","
    "\"dialog.deleteTitle\":\"Delete this note?\","
    "\"dialog.newTitle\":\"New note\","
    "\"dialog.renameTitle\":\"Rename note\","
    "\"dialog.newEncryptedTitle\":\"New encrypted note\","
    "\"dialog.noKeyboardBody\":\"USB: plug in with an OTG cable.\\n\\nBluetooth: pair to your phone, then open the address below (or scan the code).\","
    "\"dialog.cancel\":\"Cancel\","
    "\"dialog.delete\":\"Delete\","
    "\"dialog.create\":\"Create\","
    "\"dialog.rename\":\"Rename\","
    "\"files.tapDismiss\":\"Tap to dismiss\","
    "\"home.tip\":\"Open the Files tab (1) or press Ctrl-K.\\nUse Tab / arrows / 1-6 to switch pages.\","
    "\"shortcuts.title\":\"Shortcuts\","
    "\"shortcuts.body\":\"Pages\\nTab or Left / Right - next or previous page\\n1 to 6 - jump straight to a page\\n\\nFiles\\nUp / Down - move the selection (turns the page at the edge)\\nPage Up / Page Down - previous or next page of notes\\nEnter - edit the selected note\\nCtrl-V - read · Ctrl-N - new · Ctrl-R - rename · Ctrl-D - delete\\nCtrl-G - download to phone\\nWith private notes on: Ctrl-E - new encrypted · Ctrl-X - encrypt · Ctrl-Y - decrypt\\n\\nKeyboard\\nCtrl-U - US layout · Ctrl-O - Norwegian\\n\\nSync\\nEnter or Ctrl-S - sync now\\n\\nSettings\\nCtrl-F - cycle reading font · Ctrl-P - cycle phone PIN length\\nCtrl-T - cycle rotation · Ctrl-E - enable private notes · Ctrl-C - change private PIN\\nCtrl-X - exit Writerdeck (then Enter to confirm)\\n\\nAnywhere\\nCtrl-K - quick file picker\\nCtrl-C / Ctrl-X / Ctrl-V - copy, cut, paste (while editing a note)\\nCtrl-R - rotate (Lobby pages other than Files) · Ctrl-Q - quit\\nHome - from a note, back to Files; from Lobby, quit to the stock UI\\n\\nPrivate PIN\\nType the six digits on a USB keyboard, or on the phone while it shows the PIN banner.\\n\\nOpen Lobby from the stock UI\\nEsc on a USB keyboard, or both page buttons together.\""
    "},"
    "\"shortcuts\":{"
    "\"files.new\":\"n\",\"files.read\":\"v\",\"files.rename\":\"r\",\"files.delete\":\"d\","
    "\"files.download\":\"g\",\"files.encrypt\":\"x\",\"files.decrypt\":\"y\",\"files.newEncrypted\":\"e\","
    "\"keyboard.us\":\"u\",\"keyboard.no\":\"o\",\"sync.now\":\"s\","
    "\"settings.font\":\"f\",\"settings.pin\":\"p\",\"settings.rotation\":\"t\","
    "\"settings.enableVault\":\"e\",\"settings.changePin\":\"c\",\"settings.exit\":\"x\""
    "}"
    "}";

static int jsonInt(const QJsonObject &o, const char *key, int fallback)
{
    const QJsonValue v = o.value(QLatin1String(key));
    if (v.isDouble())
        return v.toInt();
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
    // Do not start m_poll here: g_lobbyUi is a static constructed before
    // QApplication, and timers started that early never fire.
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
                    parseObject(embedded.object());
                parseObject(doc.object());
                noteDiskStamp();
                watchPath();
                ++m_revision;
                qWarning("lobby-ui: loaded %s (rev %d)", qPrintable(m_path), m_revision);
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
        parseObject(embedded.object());
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
    m_btnBorder = 2;
    m_btnBorderSelected = 4;
    m_shortcutBadgeMargin = 4;
    m_pageMargin = 24;
    m_tabBtnHeight = 64;
    m_rowHeight = 72;
    m_actionBtnHeight = 72;
    m_tabSpacing = 12;
    m_contentSpacing = 12;
    m_labelPointSize = 11;
    m_badgePointSize = 9;
    m_textColor = QStringLiteral("#000000");
    m_borderColor = QStringLiteral("#000000");
    m_badgeTextColor = QStringLiteral("#000000");
    m_strings.clear();
    m_shortcuts.clear();
    m_letterToAction.clear();
}

bool LobbyUiConfig::loadFromDisk()
{
    if (m_path.isEmpty())
        return false;
    QFile f(m_path);
    if (!f.open(QIODevice::ReadOnly))
        return false;
    const QByteArray raw = f.readAll();
    f.close();
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(raw, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning("lobby-ui.json parse error: %s", qPrintable(err.errorString()));
        return false;
    }
    parseObject(doc.object());
    return true;
}

void LobbyUiConfig::parseObject(const QJsonObject &root)
{
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
        m_textColor = jsonStr(visual, "textColor", m_textColor);
        m_borderColor = jsonStr(visual, "borderColor", m_borderColor);
        m_badgeTextColor = jsonStr(visual, "badgeTextColor", m_badgeTextColor);
    }

    const QJsonObject strings = root.value(QLatin1String("strings")).toObject();
    if (root.contains(QLatin1String("strings"))) {
        m_strings.clear();
        for (auto it = strings.begin(); it != strings.end(); ++it) {
            if (it.value().isString())
                m_strings.insert(it.key(), it.value().toString());
        }
    }

    const QJsonObject shortcuts = root.value(QLatin1String("shortcuts")).toObject();
    if (root.contains(QLatin1String("shortcuts"))) {
        m_shortcuts.clear();
        for (auto it = shortcuts.begin(); it != shortcuts.end(); ++it) {
            if (!it.value().isString())
                continue;
            QString letter = it.value().toString().trimmed().toLower();
            if (letter.size() != 1 || letter[0] < QLatin1Char('a') || letter[0] > QLatin1Char('z'))
                continue;
            // Never steal global Ctrl-K / Ctrl-Q.
            if (letter == QLatin1String("k") || letter == QLatin1String("q"))
                continue;
            m_shortcuts.insert(it.key(), letter);
        }
    }
    rebuildLetterIndex();
}

void LobbyUiConfig::rebuildLetterIndex()
{
    m_letterToAction.clear();
    for (auto it = m_shortcuts.constBegin(); it != m_shortcuts.constEnd(); ++it)
        m_letterToAction.insert(it.value(), it.key());
}

void LobbyUiConfig::watchPath()
{
    const QStringList cur = m_watch.files();
    if (!cur.isEmpty())
        m_watch.removePaths(cur);
    if (m_path.isEmpty())
        return;
    if (QFile::exists(m_path))
        m_watch.addPath(m_path);
    // Also watch the directory so a replace/create is noticed.
    const QString dir = QFileInfo(m_path).absolutePath();
    if (!dir.isEmpty() && QDir(dir).exists())
        m_watch.addPath(dir);
}

void LobbyUiConfig::onFileChanged(const QString &path)
{
    Q_UNUSED(path);
    // Rename replaces drop the file watch; re-arm immediately, then reload.
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

QString LobbyUiConfig::str(const QString &key) const
{
    return m_strings.value(key);
}

QString LobbyUiConfig::shortcut(const QString &key) const
{
    return m_shortcuts.value(key);
}

QString LobbyUiConfig::actionForLetter(const QString &letter) const
{
    return m_letterToAction.value(letter.toLower());
}
