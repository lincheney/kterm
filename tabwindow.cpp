#include <QStyleOptionTab>
#include <QStylePainter>

#include "tabwindow.h"
#include "main.h"

TabWindow::TabWindow() : QTabWidget()
{
    connect(this, &TabWindow::currentChanged, this, &TabWindow::changed_tab);
    QTabBar* bar = new TabBar();
    setTabBar(bar);
    bar->setDocumentMode(true);
}

void TabWindow::add_tab()
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

void TabWindow::changed_tab(int index)
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
