#include <QVariant>
#include <QDebug>
#include <kde_terminal_interface.h>

#include "main.h"
#include "tabwindow.h"

KService::Ptr TermApp::konsole_service()
{
    if (! m_service) {
        m_service = KService::serviceByDesktopName("konsolepart");
    }
    return m_service;
}

TermPart* TermApp::make_term()
{
    TermPart* part = konsole_service()->createInstance<KParts::ReadOnlyPart>();
    if (! part) return NULL;

    QMetaObject::invokeMethod(part, "setMonitorActivityEnabled", Qt::DirectConnection, Q_ARG(bool, true));
    // connect(part, &QObject::destroyed, this, &TermApp::slotTermDestroyed);
    connect(part, SIGNAL(activityDetected()), SLOT(slotTermActivityDetected()));
    connect(part, &KParts::Part::setWindowCaption, this, &TermApp::slotTermSetWindowCaption);
    part->widget()->setProperty("kpart", QVariant::fromValue(part));

    return part;
}

void TermApp::slotTermActivityDetected()
{
    TermPart* part = (TermPart*)QObject::sender();
    if (! part->widget()->hasFocus() && ! part->property("has_activity").toBool()) {
        part->setProperty("has_activity", QVariant(true));
        part->property("tabwidget").value<QTabWidget*>()->tabBar()->update();
    }
}

void TermApp::slotTermSetWindowCaption(QString caption)
{
    QObject* part = QObject::sender();
    part->setProperty("term_title", QVariant(caption));
    part->property("tabwidget").value<QTabWidget*>()->tabBar()->update();
}

int main (int argc, char **argv)
{
    TermApp app(argc, argv);
    if (! app.konsole_service()) {
        app.quit();
        return 1;
    }

    TabWindow* tabs = new TabWindow();
    tabs->show();
    tabs->add_tab();
    tabs->add_tab();

    return app.exec();
}
