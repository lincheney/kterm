#include <QVariant>
#include <QDBusConnection>
#include <QScrollBar>
#include <QDebug>
#include <QX11Info>

#include "kterm_interface.h"
#include "kterm_adaptor.h"

#include "main.h"
#include "tabwindow.h"
#include "x.h"

void TermApp::new_window(Terminal* term, QString pwd=QString())
{
    TabWindow* tabs = new TabWindow();
    tabs->tabBar()->installEventFilter(this);
    tabs->show();
    tabs->new_tab(-1, term, pwd);
}

void TermApp::load_settings() {
    m_settings = new QSettings();

    QString path = m_settings->value("stylesheet").toString();
    if (! path.isNull()) {
        if (path.startsWith('~')) {
            path = QDir::home().path() % path.rightRef(path.length() - 1);
        }

        QFile stylesheet(path);
        if (stylesheet.open(QFile::ReadOnly))
            setStyleSheet(stylesheet.readAll());
        else
            qWarning() << "Could not open" << path;
    }

    TabWindow* window;
    foreach(QObject* obj, topLevelWidgets()) {
        window = qobject_cast<TabWindow*>(obj);
        if (window) window->load_settings(m_settings);
    }
}

void TermApp::drag_tabs(QPoint pos, bool split)
{
    QTabBar* bar;
    TabWindow* _new_window = qobject_cast<TabWindow*>(topLevelAt(pos));
    QTabWidget* _old_window = dragged_part->tabwidget;

    if (!_new_window) {
        if (split && _old_window->count() > 1) {
            _old_window->removeTab(_old_window->currentIndex());
            new_window(dragged_part);
        } else {
            _old_window->tabBar()->update();
        }
        return;
    }

    bar = _new_window->tabBar();
    int i = bar->tabAt(bar->mapFromGlobal(pos));

    if (_old_window == _new_window) {
        // same tabbar , just move it about
        int current = _old_window->currentIndex();
        if (i == -1) i = current;
        else if (current != i) bar->moveTab(current, i);
    } else {
        // diff tabwindow
        _old_window->removeTab(_old_window->currentIndex());
        i = _new_window->new_tab(i, dragged_part);
    }
    bar->update();
}

bool TermApp::eventFilter(QObject* obj, QEvent* event)
{
    QTabBar* bar = qobject_cast<QTabBar*>(obj);
    TabWindow* tabs = bar ? qobject_cast<TabWindow*>(bar->parent()) : NULL;
    if (tabs) {
        QMouseEvent* mevent;
        switch (event->type()) {
            case QEvent::MouseButtonPress:
                mevent = static_cast<QMouseEvent*>(event);
                if (mevent->button() == Qt::LeftButton && tabs->currentIndex() != -1) {
                    m_drag_start = mevent->globalPos();
                }
                break;

            case QEvent::MouseMove:
                mevent = static_cast<QMouseEvent*>(event);
                if (mevent->buttons() & Qt::LeftButton) {
                    int current = tabs->currentIndex();
                    if (!dragged_part && current != -1 && (mevent->globalPos() - m_drag_start).manhattanLength() > QApplication::startDragDistance()) {
                        // start dragging
                        setOverrideCursor(Qt::DragCopyCursor);

                        dragged_part = bar->tabData(current).value<Terminal*>();
                        bar->update();
                    }

                    if (dragged_part) {
                        drag_tabs(mevent->globalPos(), false);
                    }
                }
                break;

            case QEvent::MouseButtonRelease:
                mevent = static_cast<QMouseEvent*>(event);
                if (mevent->button() == Qt::LeftButton && dragged_part) {
                    // stop dragging
                    setOverrideCursor(Qt::ArrowCursor);

                    drag_tabs(mevent->globalPos(), true);
                    dragged_part = NULL;
                    // kill any left over empty windows
                    TabWindow* window;
                    foreach(QObject* obj, topLevelWidgets()) {
                        window = qobject_cast<TabWindow*>(obj);
                        if (window && window->count() == 0) {
                            window->close();
                        }
                    }
                }
                break;

            default: break;
        }
    }
    return false;
}

int main (int argc, char **argv)
{
    TermApp app(argc, argv);
    app.setOrganizationDomain(app.applicationName());
    bool no_dbus = false;

    for (int i = 1; i < argc; ++i)
        if (! qstrcmp(argv[i], "--standalone"))
            no_dbus = true;

    if (!no_dbus) {

#define DBUS_SERVICE "org.kterm"
#define DBUS_PATH "/"

        Display *d = QX11Info::display();
        // skip the :
        QString dbus_service = QString(DBUS_SERVICE ".x" GIT_REF ".x%1").arg(x_display(d)+1);

        QDBusConnection dbus = QDBusConnection::sessionBus();
        if (dbus.isConnected()) {
            if (! dbus.registerService(dbus_service)) {
                // app already exists, launch new window in existing one
                org::kterm* iface = new org::kterm(dbus_service, DBUS_PATH, dbus);
                iface->new_window(QDir::currentPath()).waitForFinished();
                return 0;
            }

            // expose on dbus
            KtermAdaptor* a = new KtermAdaptor(&app);
            dbus.registerObject(DBUS_PATH, a, QDBusConnection::ExportAllSlots);
        }
    }

    app.load_settings();
    app.setWindowIcon(QIcon::fromTheme("utilities-terminal"));

    if (! Terminal::konsole_service() ) {
        qCritical() << "Unable to start Konsole::Part service";
        app.quit();
        return 1;
    }
    app.new_window(NULL);

    return app.exec();
}
