#include <QStyleOptionTab>
#include <QStylePainter>
#include <QShortcut>
#include <QDebug>
#include <QDrag>
#include <QMimeData>

#include "tabwindow.h"

TabWindow::TabWindow() : QTabWidget()
{
    connect(this, &TabWindow::currentChanged, this, &TabWindow::changed_tab);

    QTabBar* bar = new TabBar();
    setTabBar(bar);
    // bar->setMovable(true);
    bar->setDocumentMode(true);
    // setAcceptDrops(true);

    QShortcut* shortcut;
    // new tab
    shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_T), this);
    QObject::connect(shortcut, &QShortcut::activated,
            [=](){ new_tab(-1, NULL); });

    // new window
    shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_N), this);
    QObject::connect(shortcut, &QShortcut::activated, qApp,
            [=](){ qApp->new_window(NULL); });

    // next tab
    shortcut = new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Right), this);
    QObject::connect(shortcut, &QShortcut::activated,
            [=]() { setCurrentIndex((currentIndex() + 1) % count()); });

    // prev tab
    shortcut = new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Left), this);
    QObject::connect(shortcut, &QShortcut::activated,
            [=]() { setCurrentIndex((currentIndex() - 1 + count()) % count()); });
}

int TabWindow::new_tab(int index, TermPart* part)
{
    part = part ? part : qApp->make_term();
    if (! part) {
        qApp->quit();
        return -1;
    }

    part->setProperty("tabwidget", QVariant::fromValue(this));
    QWidget* widget = part->widget();
    widget->setProperty("kpart", QVariant::fromValue(part));
    index = insertTab(index, widget, "");
    tabBar()->setTabData(index, QVariant::fromValue(part));
    setCurrentIndex(index);
    return index;
}

void TabWindow::changed_tab(int index)
{
    if (index != -1) {
        QWidget* w = widget(index);
        w->setFocus();

        QObject* part = w->property("kpart").value<QObject*>();
        part->setProperty("has_activity", QVariant(false));
        tabBar()->update();
    } else if (! qApp->dragged_part) {
        close();
    }
}

void TabBar::paintEvent(QPaintEvent*)
{
    // QTabBar::paintEvent(ev);
    QStylePainter p(this);
    for (int i = 0; i < count(); i++) {
        QStyleOptionTab tab;
        initStyleOption(&tab, i);

        TermPart* part = tabData(i).value<TermPart*>();
        if (part->property("has_activity").toBool()) {
            tab.state |= QStyle::State_HasFocus;
        }
        tab.text = part->property("term_title").toString();
        // only :pressed if dragging
        if (qApp->dragged_part && qApp->dragged_part == tabData(i).value<TermPart*>()) {
            tab.state |= QStyle::State_Sunken;
        } else {
            tab.state &= ~QStyle::State_Sunken;
        }

        p.drawControl(QStyle::CE_TabBarTab, tab);
    }
}
