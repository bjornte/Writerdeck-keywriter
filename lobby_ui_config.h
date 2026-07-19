#ifndef LOBBY_UI_CONFIG_H
#define LOBBY_UI_CONFIG_H

#include <QHash>
#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QFileSystemWatcher>

// On-disk Lobby look / copy / Ctrl-letter chords (/home/root/.Writerdeck/lobby-ui.json).
// Not compiled into resources -- edit on the tablet; hot-reloads via QFileSystemWatcher.
class LobbyUiConfig : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int revision READ revision NOTIFY changed)
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
    Q_PROPERTY(QString textColor READ textColor NOTIFY changed)
    Q_PROPERTY(QString borderColor READ borderColor NOTIFY changed)
    Q_PROPERTY(QString badgeTextColor READ badgeTextColor NOTIFY changed)
    Q_PROPERTY(QVariantMap shortcuts READ shortcutsMap NOTIFY changed)
    Q_PROPERTY(QVariantMap strings READ stringsMap NOTIFY changed)

public:
    explicit LobbyUiConfig(QObject *parent = nullptr);

    void setPath(const QString &path);
    Q_INVOKABLE void reload();

    int revision() const { return m_revision; }
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
    QString textColor() const { return m_textColor; }
    QString borderColor() const { return m_borderColor; }
    QString badgeTextColor() const { return m_badgeTextColor; }
    QVariantMap shortcutsMap() const;
    QVariantMap stringsMap() const;

    Q_INVOKABLE QString str(const QString &key) const;
    Q_INVOKABLE QString shortcut(const QString &key) const;
    Q_INVOKABLE QString actionForLetter(const QString &letter) const;

signals:
    void changed();

private slots:
    void onFileChanged(const QString &path);

private:
    void applyDefaults();
    bool loadFromDisk();
    void parseObject(const QJsonObject &root);
    void rebuildLetterIndex();
    void watchPath();

    QFileSystemWatcher m_watch;
    QString m_path;
    int m_revision = 0;

    int m_btnBorder = 2;
    int m_btnBorderSelected = 4;
    int m_shortcutBadgeMargin = 4;
    int m_pageMargin = 24;
    int m_tabBtnHeight = 64;
    int m_rowHeight = 72;
    int m_actionBtnHeight = 72;
    int m_tabSpacing = 12;
    int m_contentSpacing = 12;
    int m_labelPointSize = 11;
    int m_badgePointSize = 9;
    QString m_textColor = QStringLiteral("#000000");
    QString m_borderColor = QStringLiteral("#000000");
    QString m_badgeTextColor = QStringLiteral("#000000");

    QHash<QString, QString> m_strings;
    QHash<QString, QString> m_shortcuts;
    QHash<QString, QString> m_letterToAction;
};

#endif
