#include <QApplication>
#include <QPushButton>
#include <QTabWidget>
#include <QVariant>
#include <QTabBar>
#include <QDebug>
#include <QStylePainter>
// #include <
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

    QTabBar* bar = new TabBar();
    setTabBar(bar);
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

void TabBar::paintEvent(QPaintEvent* ev)
{
    QTabBar::paintEvent(ev);
    QStylePainter p(this);
    for (int i = 0; i < count(); i++) {
        QStyleOptionTab tab;
        initStyleOption(&tab, i);
        tab.state |= QStyle::State_HasFocus;
        // tab.palette.setColor(QPalette::Window, QColor(0xff,0,0));
        // tab.palette.setColor(QPalette::WindowText, QColor(0xff,0,0));
        // tab.palette.setColor(QPalette::Base, QColor(0xff,0,0));
        // tab.palette.setColor(QPalette::AlternateBase, QColor(0xff,0,0));
        // tab.palette.setColor(QPalette::ToolTipBase, QColor(0xff,0,0));
        // tab.palette.setColor(QPalette::ToolTipText, QColor(0xff,0,0));
        // tab.palette.setColor(QPalette::Text, QColor(0xff,0,0));
        tab.palette.setColor(QPalette::Button, QColor(0xff,0,0));
        // tab.palette.setColor(QPalette::ButtonText, QColor(0xff,0,0));
        // tab.palette.setColor(QPalette::BrightText, QColor(0xff,0,0));
        p.drawControl(QStyle::CE_TabBarTab, tab);
    }
    // p.setPen(Qt::blue);
    // qDebug() << rect();
    // p.drawRect(rect());
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
