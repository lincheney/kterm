#include <QStyleOptionTab>
#include <QStylePainter>
#include <QShortcut>
#include <QDebug>
#include <QDrag>
#include <QAction>
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

    QAction* action;

#define MAKE_ACTION(text, shortcut, slot) \
    action = new QAction(text, this); \
    action->setShortcut(shortcut); \
    connect(action, &QAction::triggered, slot); \
    addAction(action)

    MAKE_ACTION("New tab", Qt::CTRL + Qt::SHIFT + Qt::Key_T, [=](){ new_tab(-1, NULL); });
    MAKE_ACTION("New window", Qt::CTRL + Qt::SHIFT + Qt::Key_N, [=](){ qApp->new_window(NULL); });
    MAKE_ACTION("Next tab", Qt::SHIFT + Qt::Key_Right, [=](){ setCurrentIndex(offset_index(1)); });
    MAKE_ACTION("Prev tab", Qt::SHIFT + Qt::Key_Left, [=](){ setCurrentIndex(offset_index(-1)); });
    MAKE_ACTION("Move tab forward", Qt::CTRL + Qt::SHIFT + Qt::Key_Right, [=](){ tabBar()->moveTab(currentIndex(), offset_index(1)); });
    MAKE_ACTION("Move tab backward", Qt::CTRL + Qt::SHIFT + Qt::Key_Left, [=](){ tabBar()->moveTab(currentIndex(), offset_index(-1)); });
}

int TabWindow::offset_index(int offset)
{
    return (currentIndex() + offset + count()) % count();
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
        part->setProperty("has_silence", QVariant(false));
        tabBar()->update();
        setWindowTitle(part->property("term_title").toString());
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
        tab.text = part->property("term_title").toString();

        if (part->property("has_activity").toBool()) {
            tab.state |= QStyle::State_On;
        }

        if (part->property("has_silence").toBool()) {
            tab.state |= QStyle::State_Off;
        }

        // only :pressed if dragging
        if (qApp->dragged_part && qApp->dragged_part == tabData(i).value<TermPart*>()) {
            tab.state |= QStyle::State_Sunken;
        } else {
            tab.state &= ~QStyle::State_Sunken;
        }

        p.drawControl(QStyle::CE_TabBarTab, tab);
    }
}
