#include <QStyleOptionTab>
#include <QStylePainter>
#include <QFont>
#include <QDebug>
#include <QToolBar>
#include <QAction>
#include <QIcon>
#include <unistd.h>
#include <sys/param.h>

#include "tabwindow.h"

TabWindow::TabWindow() : QTabWidget()
{
    connect(this, &TabWindow::currentChanged, this, &TabWindow::changed_tab);

    QTabBar* bar = new TabBar();
    setTabBar(bar);
    bar->setDocumentMode(true);

    QToolBar* toolbar = new QToolBar();
    setCornerWidget(toolbar);

    QAction* action;
#define MAKE_ACTION(text, shortcut, slot) \
    action = new QAction(text, this); \
    action->setShortcut(shortcut); \
    connect(action, &QAction::triggered, slot); \
    addAction(action)

    MAKE_ACTION("New window", Qt::CTRL + Qt::SHIFT + Qt::Key_N, [=](){ qApp->new_window(NULL, current_dir()); });
    action->setIcon(QIcon::fromTheme("window-new"));
    toolbar->addAction(action);

    MAKE_ACTION("New tab", Qt::CTRL + Qt::SHIFT + Qt::Key_T, [=](){ new_tab(-1, NULL, current_dir()); });
    action->setIcon(QIcon::fromTheme("list-add"));
    toolbar->addAction(action);

    MAKE_ACTION("Next tab", Qt::SHIFT + Qt::Key_Right, [=](){ setCurrentIndex(offset_index(1)); });
    MAKE_ACTION("Prev tab", Qt::SHIFT + Qt::Key_Left, [=](){ setCurrentIndex(offset_index(-1)); });
    MAKE_ACTION("Move tab forward", Qt::CTRL + Qt::SHIFT + Qt::Key_Right, [=](){ tabBar()->moveTab(currentIndex(), offset_index(1)); });
    MAKE_ACTION("Move tab backward", Qt::CTRL + Qt::SHIFT + Qt::Key_Left, [=](){ tabBar()->moveTab(currentIndex(), offset_index(-1)); });

    MAKE_ACTION("Go to tab 1", Qt::ALT + Qt::Key_1, [=](){ setCurrentIndex(0); });
    MAKE_ACTION("Go to tab 2", Qt::ALT + Qt::Key_2, [=](){ setCurrentIndex(qMin(1, count()-1)); });
    MAKE_ACTION("Go to tab 3", Qt::ALT + Qt::Key_3, [=](){ setCurrentIndex(qMin(2, count()-1)); });
    MAKE_ACTION("Go to tab 4", Qt::ALT + Qt::Key_4, [=](){ setCurrentIndex(qMin(3, count()-1)); });
    MAKE_ACTION("Go to tab 5", Qt::ALT + Qt::Key_5, [=](){ setCurrentIndex(qMin(4, count()-1)); });
    MAKE_ACTION("Go to tab 6", Qt::ALT + Qt::Key_6, [=](){ setCurrentIndex(qMin(5, count()-1)); });
    MAKE_ACTION("Go to tab 7", Qt::ALT + Qt::Key_7, [=](){ setCurrentIndex(qMin(6, count()-1)); });
    MAKE_ACTION("Go to tab 8", Qt::ALT + Qt::Key_8, [=](){ setCurrentIndex(qMin(7, count()-1)); });
    MAKE_ACTION("Go to tab 9", Qt::ALT + Qt::Key_9, [=](){ setCurrentIndex(qMin(8, count()-1)); });
    MAKE_ACTION("Go to last tab", Qt::ALT + Qt::Key_0, [=](){ setCurrentIndex(count()-1); });

    MAKE_ACTION("Find", Qt::CTRL + Qt::SHIFT + Qt::Key_F, [=](){
        QObject* part = tabBar()->tabData(currentIndex()).value<TermPart*>();
        QObject* session = part->property("session_controller").value<QObject*>();
        QMetaObject::invokeMethod(session, "searchBarEvent", Qt::DirectConnection);
    });
}

int TabWindow::offset_index(int offset)
{
    return (currentIndex() + offset + count()) % count();
}

int TabWindow::new_tab(int index, TermPart* part, QString pwd)
{
    part = part ? part : qApp->make_term(pwd);
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

QString TabWindow::current_dir()
{
    TerminalInterface* term = qobject_cast<TerminalInterface*>(currentWidget()->property("kpart").value<QObject*>());
    int pid = term->terminalProcessId();

    char buffer[MAXPATHLEN+1], fname[100];
    snprintf(fname, 100, "/proc/%i/cwd", pid);
    int length = readlink(fname, buffer, MAXPATHLEN);
    if (length == -1) {
        return term->currentWorkingDirectory();
    }

    buffer[length] = '\0';
    return buffer;
}

void TabBar::paintEvent(QPaintEvent*)
{
    // QTabBar::paintEvent(ev);
    char buffer[2] = " ";
    QStylePainter p(this);
    int label_width = height();
    QRect label_rect(0, 0, label_width, label_width);

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

        label_rect.moveTo(tab.rect.x(), tab.rect.y());
        tab.rect.adjust(label_width, 0, 0, 0);
        p.drawControl(QStyle::CE_TabBarTab, tab);

        p.save();
        p.setBrush(m_labelbg);
        p.setPen(m_labelbg);
        p.drawRect(label_rect);

        QFont font = p.font();
        font.setBold(true);
        p.setFont(font);
        p.setPen(m_labelfg);
        buffer[0] = '1' + i;
        p.drawText(label_rect, Qt::AlignCenter, buffer);
        p.restore();
    }
}
