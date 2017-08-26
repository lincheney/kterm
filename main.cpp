#include <QApplication>
#include <QPushButton>
#include <QTabWidget>
#include <QVariant>
#include <QTabBar>
#include "main.h"
// #include <kde_terminal_interface.h>

KService::Ptr konsole_service;

Tabs::Tabs() : QTabWidget()
{
    m_service = KService::serviceByDesktopName("konsolepart");
    if (! m_service) {
        qApp->quit();
        return;
    }

    connect(this, &Tabs::currentChanged, this, &Tabs::changed_tab);

    QTabBar* bar = tabBar();
    bar->setDocumentMode(true);
}

void Tabs::add_tab()
{
    KParts::ReadOnlyPart* term = m_service->createInstance<KParts::ReadOnlyPart>();
    if (! term) {
        qApp->quit();
        return;
    }

    QWidget* widget = term->widget();
    widget->setProperty("kpart", QVariant::fromValue(term));
    connect(term, &QObject::destroyed, this, &Tabs::slotTermDestroyed);
    addTab(widget, "XYZ");
}

void Tabs::slotTermDestroyed(QObject* widget)
{
    int i = indexOf(qobject_cast<QWidget*>(widget));
    removeTab(i);
}

void Tabs::changed_tab(int index)
{
    if (index == -1) {
        close();
    } else {
        widget(index)->setFocus();
    }
}

int main (int argc, char **argv)
{
    QApplication app(argc, argv);

    Tabs* tabs = new Tabs();
    tabs->show();
    tabs->add_tab();
    tabs->add_tab();

    return app.exec();
}
