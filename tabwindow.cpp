#include <QStyleOptionTab>
#include <QStylePainter>
#include <QFont>
#include <QScrollBar>
#include <QDebug>
#include <QToolButton>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QIcon>
#include <unistd.h>
#include <sys/param.h>

#include "tabwindow.h"
#include "main.h"

TabWindow::TabWindow() : QTabWidget()
{
    connect(this, &TabWindow::currentChanged, this, &TabWindow::changed_tab);

    QTabBar* bar = new TabBar();
    setTabBar(bar);
    bar->setDocumentMode(true);
    bar->setFocusPolicy(Qt::NoFocus);
    bar->setElideMode(Qt::ElideLeft);

    QToolButton* menu_button = new QToolButton();
    menu_button->setPopupMode(QToolButton::InstantPopup);
    menu_button->setIcon(QIcon::fromTheme("system-run"));
    setCornerWidget(menu_button);

    QAction* action;
#define MAKE_ACTION(text, shortcut, slot) \
    action = new QAction(text, this); \
    action->setData(shortcut); \
    action->setProperty("key", text); \
    connect(action, &QAction::triggered, slot); \
    addAction(action)

    MAKE_ACTION("New window", Qt::CTRL + Qt::SHIFT + Qt::Key_N, [=](){ qApp->new_window(NULL, current_dir()); });
    action->setIcon(QIcon::fromTheme("window-new"));
    menu_button->addAction(action);

    MAKE_ACTION("New tab", Qt::CTRL + Qt::SHIFT + Qt::Key_T, [=](){ new_tab(-1, NULL, current_dir()); });
    action->setIcon(QIcon::fromTheme("list-add"));
    menu_button->addAction(action);

    MAKE_ACTION("Reload settings", 0, [=](){ qApp->load_settings(); });
    action->setIcon(QIcon::fromTheme("view-refresh"));
    menu_button->addAction(action);

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
            QObject* sc = currentTerminal()->session_controller();
            QMetaObject::invokeMethod(sc, "searchBarEvent", Qt::DirectConnection);
    });

    MAKE_ACTION("Scroll to top", Qt::SHIFT + Qt::Key_Home, [=](){
            currentTerminal()->scrollbar()->setValue(0);
    });
    MAKE_ACTION("Scroll to bottom", Qt::SHIFT + Qt::Key_End, [=](){
            QScrollBar* bar = currentTerminal()->scrollbar();
            bar->setValue(bar->maximum());
    });

    load_settings(new QSettings());
}

void TabWindow::load_settings(QSettings* settings)
{
    foreach(QAction* action, actions()) {
        QVariant keystr = settings->value("shortcuts/" + action->property("key").toString());
        QKeySequence keyseq = keystr.isValid() ? QKeySequence(keystr.toString()) : QKeySequence(action->data().toInt());
        action->setShortcut(keyseq);
    }
}

int TabWindow::offset_index(int offset)
{
    return (currentIndex() + offset + count()) % count();
}

int TabWindow::new_tab(int index, Terminal* term, QString pwd)
{
    term = term ? term : Terminal::make_term();
    if (! term) {
        qApp->quit();
        return -1;
    }

    term->tabwidget = this;
    index = insertTab(index, term, term->title);
    tabBar()->setTabData(index, QVariant::fromValue(term));
    setCurrentIndex(index);
    return index;
}

void TabWindow::changed_tab(int index)
{
    if (index != -1) {
        Terminal* term = qobject_cast<Terminal*>(widget(index));
        term->setFocus();

        term->has_activity = false;
        term->has_silence = false;
        tabBar()->update();
        setWindowTitle(term->title);
    } else if (! qApp->dragged_part) {
        close();
    }
}

QString TabWindow::current_dir()
{
    TerminalInterface* part = currentTerminal()->terminalInterface();
    int pid = part->terminalProcessId();

    char buffer[MAXPATHLEN+1], fname[100];
    snprintf(fname, 100, "/proc/%i/cwd", pid);
    int length = readlink(fname, buffer, MAXPATHLEN);
    if (length == -1) {
        return part->currentWorkingDirectory();
    }

    buffer[length] = '\0';
    return buffer;
}

void TabWindow::closeEvent(QCloseEvent* e)
{
    if (count() == 0) {
        e->accept();
        return;
    }

    QString msg = QString("You have %1 tab(s) open.\nAre you sure you want to quit?").arg(count());
    QMessageBox::StandardButton reply = QMessageBox::question(
            this, "Confirm close", msg, QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        e->accept();
        return;
    }
    e->ignore();
}

void TabBar::resizeEvent(QResizeEvent* e)
{
    QTabWidget* tabs = qobject_cast<QTabWidget*>(parent());
    QToolButton* button = qobject_cast<QToolButton*>(tabs->cornerWidget());
    button->setIconSize(QSize(height(), height()));

    QTabBar::resizeEvent(e);
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
        label_rect.moveTo(tab.rect.x(), tab.rect.y());
        tab.rect.adjust(label_width, 0, 0, 0);

        Terminal* term = tabData(i).value<Terminal*>();
        tab.text = fontMetrics().elidedText(term->title, elideMode(), tab.rect.width());

        if (term->has_activity) {
            tab.state |= QStyle::State_On;
        }

        if (term->has_silence) {
            tab.state |= QStyle::State_Off;
        }

        // only :pressed if dragging
        if (qApp->dragged_part && qApp->dragged_part == tabData(i).value<Terminal*>()) {
            tab.state |= QStyle::State_Sunken;
        } else {
            tab.state &= ~QStyle::State_Sunken;
        }

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
