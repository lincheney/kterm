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
    void new_window();

public Q_SLOTS:
    void slotTermActivityDetected();
    void slotTermSetWindowCaption(QString);
    void slotTermOverrideShortcut(QKeyEvent*, bool&);

protected:
    void updateTermTitle(QObject*);

private:
    KService::Ptr m_service;
};

#endif
