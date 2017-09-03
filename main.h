#include <QApplication>
#include <QKeyEvent>
#include <KParts/ReadOnlyPart>
#include <KService>

#ifndef MAIN_H
#define MAIN_H

typedef KParts::ReadOnlyPart TermPart;

class TermApp: public QApplication
{
    Q_OBJECT
public:
    TermApp(int &argc, char **argv) : QApplication(argc, argv) {};
    KService::Ptr konsole_service();
    TermPart* make_term();
    TermPart* dragged_part = NULL;

public Q_SLOTS:
    void new_window(TermPart*);
    void slotTermActivityDetected();
    void slotTermSilenceDetected();
    void slotTermSetWindowCaption(QString);
    void slotTermOverrideShortcut(QKeyEvent*, bool&);

protected:
    void updateTermTitle(QObject*);
    bool eventFilter(QObject*, QEvent*);

private:
    void drag_tabs(QPoint, bool);
    KService::Ptr m_service;
    QPoint m_drag_start;
};

#define qApp (static_cast<TermApp*>(QCoreApplication::instance()))

#endif
