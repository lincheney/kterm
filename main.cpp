#include <QVariant>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>
#include <kde_terminal_interface.h>

#include "kterm_interface.h"
#include "kterm_adaptor.h"

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
    connect(part, SIGNAL(overrideShortcut(QKeyEvent*, bool&)), SLOT(slotTermOverrideShortcut(QKeyEvent*, bool&)) );
    part->widget()->setProperty("kpart", QVariant::fromValue(part));

    return part;
}

void TermApp::new_window(TermPart* part)
{
    TabWindow* tabs = new TabWindow();
    tabs->show();
    tabs->new_tab(part);
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

void TermApp::slotTermOverrideShortcut(QKeyEvent*, bool &override)
{
    override = false;
}

int main (int argc, char **argv)
{
    TermApp app(argc, argv);
    if (! app.konsole_service()) {
        app.quit();
        return 1;
    }

#define DBUS_SERVICE "org.kterm"
#define DBUS_PATH "/"

    QDBusConnection dbus = QDBusConnection::sessionBus();
    if (dbus.isConnected()) {
        if (! dbus.registerService(DBUS_SERVICE)) {
            // app already exists, launch new window in existing one
            org::kterm* iface = new org::kterm(DBUS_SERVICE, DBUS_PATH, dbus);
            iface->new_window().waitForFinished();
            return 0;
        }

        // expose on dbus
        KtermAdaptor* a = new KtermAdaptor(&app);
        dbus.registerObject(DBUS_PATH, a, QDBusConnection::ExportAllSlots);
    }

    app.new_window(NULL);

    return app.exec();
}
