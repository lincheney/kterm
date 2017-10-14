#include <QTabWidget>
#include <QScrollBar>
#include <QTabBar>
#include <KParts/ReadOnlyPart>
#include <KService>
#include <kde_terminal_interface.h>

#ifndef TERMINAL_H
#define TERMINAL_H

class Terminal : public QWidget {
    Q_OBJECT

public:
    Terminal(KParts::ReadOnlyPart*, const QStringList&);
    ~Terminal();
    static Terminal* make_term(const QString&, const QStringList&);
    static KService::Ptr konsole_service();

    QObject* session_controller() { return m_session_controller; };
    QScrollBar* scrollbar() { return m_scrollbar; };
    TerminalInterface* terminalInterface() { return qobject_cast<TerminalInterface*>(m_part); };

    QTabWidget* tabwidget = NULL;
    QString title;
    bool has_activity = false;
    bool has_silence = false;

public slots:
    void slotTermOverrideShortcut(QKeyEvent*, bool&);
    void setWindowCaption(QString);
    void activityDetected();
    void silenceDetected();
    void closeTerminal();

private:
    KParts::ReadOnlyPart* m_part;
    QWidget* m_widget;
    QObject* m_session_controller;
    QScrollBar* m_scrollbar;
};

#endif
