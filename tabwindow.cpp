#include <QStyleOptionTab>
#include <QStylePainter>
#include <QShortcut>
#include <QDebug>

#include "tabwindow.h"

TabWindow::TabWindow() : QTabWidget()
{
    connect(this, &TabWindow::currentChanged, this, &TabWindow::changed_tab);

    QTabBar* bar = new TabBar();
    setTabBar(bar);
    // bar->setMovable(true);
    bar->setDocumentMode(true);

    QShortcut* shortcut;
    // new tab
    shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_T), this);
    QObject::connect(shortcut, &QShortcut::activated,
            [=](){ new_tab(NULL); });

    // new window
    shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_N), this);
    QObject::connect(shortcut, &QShortcut::activated, (TermApp*)qApp,
            [=](){ qobject_cast<TermApp*>(qApp)->new_window(NULL); });

    // next tab
    shortcut = new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Right), this);
    QObject::connect(shortcut, &QShortcut::activated,
            [=]() { setCurrentIndex((currentIndex() + 1) % count()); });

    // prev tab
    shortcut = new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Left), this);
    QObject::connect(shortcut, &QShortcut::activated,
            [=]() { setCurrentIndex((currentIndex() - 1 + count()) % count()); });
}

void TabWindow::new_tab(TermPart* part)
{
    part = part ? part : qobject_cast<TermApp*>(qApp)->make_term();
    if (! part) {
        qApp->quit();
        return;
    }

    part->setProperty("tabwidget", QVariant::fromValue(this));
    QWidget* widget = part->widget();
    int index = addTab(widget, "");
    tabBar()->setTabData(index, QVariant::fromValue(part));
    setCurrentIndex(index);
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

void TabWindow::split_to_window(int index)
{
    TermPart* part = tabBar()->tabData(index).value<TermPart*>();
    removeTab(index);
    qobject_cast<TermApp*>(qApp)->new_window(part);
}

void TabBar::paintEvent(QPaintEvent* ev)
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
        if (!m_start_drag || i != currentIndex()) {
            tab.state &= ~QStyle::State_Sunken;
        }

        p.drawControl(QStyle::CE_TabBarTab, tab);
    }
}

void TabBar::mouseMoveEvent(QMouseEvent* event)
{
    QTabBar::mouseMoveEvent(event);

    if (event->buttons() & Qt::LeftButton) {
        int current = currentIndex();
        if (! m_start_drag && current != -1 && (event->pos() - m_drag_start).manhattanLength() > QApplication::startDragDistance()) {
            m_start_drag = true;
            repaint(tabRect(current));
        }

        if (m_start_drag) {
            int i = tabAt(event->pos());
            if (i != -1 && current != i) moveTab(current, i);
        }
    }
}

void TabBar::mousePressEvent(QMouseEvent* event)
{
    QTabBar::mousePressEvent(event);

    if (event->button() == Qt::LeftButton && currentIndex() != -1) {
        m_drag_start = event->pos();
    }
}

void TabBar::mouseReleaseEvent(QMouseEvent* event)
{
    QTabBar::mousePressEvent(event);
    if (event->button() == Qt::LeftButton && m_start_drag) {
        m_start_drag = false;

        int current = currentIndex();
        int i = tabAt(event->pos());
        if (i == -1) {
            int y = event->pos().y();
            if (-y > QApplication::startDragDistance() || (y - height()) > QApplication::startDragDistance()) {
                // new window
                qobject_cast<TabWindow*>(parent())->split_to_window(current);
                return;
            }
        } else if (current != i) moveTab(current, i);

        repaint(tabRect(current));
    }
}
