#include <QPushButton>
#include <QTabWidget>
#include <QVariant>
#include <QTabBar>
#include <QDebug>
#include <QStylePainter>
#include <QMetaMethod>
#include <kde_terminal_interface.h>

#include "main.h"

Tabs::Tabs() : QTabWidget()
{
    connect(this, &Tabs::currentChanged, this, &Tabs::changed_tab);
    QTabBar* bar = new TabBar();
    setTabBar(bar);
    bar->setDocumentMode(true);
}

void Tabs::add_tab()
{
    TermPart* part = ((TermApp*)qApp)->make_term();
    if (! part) {
        qApp->quit();
        return;
    }

    part->setProperty("tabwidget", QVariant::fromValue(this));
    QWidget* widget = part->widget();
    int index = addTab(widget, "");
    tabBar()->setTabData(index, QVariant::fromValue(part));
}

void Tabs::changed_tab(int index)
{
    if (index == -1) {
        close();
    } else {
        QWidget* w = widget(index);
        w->setFocus();

        QObject* part = w->property("kpart").value<QObject*>();
        part->setProperty("has_activity", QVariant(false));
        tabBar()->update();
    }
}

void TabBar::paintEvent(QPaintEvent* ev)
{
    QTabBar::paintEvent(ev);
    QStylePainter p(this);
    for (int i = 0; i < count(); i++) {
        QStyleOptionTab tab;
        initStyleOption(&tab, i);

        TermPart* part = tabData(i).value<TermPart*>();
        if (part->property("has_activity").toBool()) {
            tab.state |= QStyle::State_HasFocus;
        }
        tab.text = part->property("term_title").toString();

        p.drawControl(QStyle::CE_TabBarTab, tab);
    }
}

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

    Tabs* tabs = new Tabs();
    tabs->show();
    tabs->add_tab();
    tabs->add_tab();

    return app.exec();
}
