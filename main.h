#include <QApplication>
#include <QKeyEvent>
#include <QSettings>

#include "terminal.h"

#ifndef MAIN_H
#define MAIN_H

class TermApp: public QApplication
{
    Q_OBJECT
public:
    TermApp(int &argc, char **argv) : QApplication(argc, argv) {};
    Terminal* dragged_part = NULL;
    QSettings* settings() { return m_settings; }

public Q_SLOTS:
    void new_window(Terminal* = NULL, const QString& = QString(), const QStringList& = QStringList());
    void new_window(const QString& pwd, const QStringList& args){ new_window(NULL, pwd, args); };
    void load_settings();

protected:
    void updateTermTitle(QObject*);
    bool eventFilter(QObject*, QEvent*);

private:
    void drag_tabs(QPoint, bool);
    QPoint m_drag_start;
    QSettings* m_settings;
};

#define qApp (static_cast<TermApp*>(QCoreApplication::instance()))

#endif
