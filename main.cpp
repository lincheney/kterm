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

QObject* find_child(QObject* parent, const char* classname)
{
    foreach(QObject* child, parent->children()) {
        if (child->inherits(classname)) return child;
    }
    return NULL;
}

KService::Ptr TermApp::konsole_service()
{
    if (! m_service) {
        m_service = KService::serviceByDesktopName("konsolepart");
    }
    return m_service;
}

TermPart* TermApp::make_term(QString pwd)
{
    QDir::setCurrent(pwd);

    TermPart* part = konsole_service()->createInstance<KParts::ReadOnlyPart>();
    if (! part) return NULL;

    QMetaObject::invokeMethod(part, "setMonitorActivityEnabled", Qt::DirectConnection, Q_ARG(bool, true));
    QMetaObject::invokeMethod(part, "setMonitorSilenceEnabled", Qt::DirectConnection, Q_ARG(bool, true));

    connect(part, SIGNAL(activityDetected()), SLOT(slotTermActivityDetected()));
    connect(part, SIGNAL(silenceDetected()), SLOT(slotTermSilenceDetected()));
    connect(part, &KParts::Part::setWindowCaption, this, &TermApp::slotTermSetWindowCaption);
    connect(part, SIGNAL(overrideShortcut(QKeyEvent*, bool&)), SLOT(slotTermOverrideShortcut(QKeyEvent*, bool&)) );
    part->widget()->setProperty("kpart", QVariant::fromValue(part));

    QObject* view_mgr = find_child(part, "Konsole::ViewManager");
    QObject* session_controller = find_child(view_mgr, "Konsole::SessionController");

    QScrollBar* scrollbar = part->widget()->findChild<QScrollBar*>(QString(), Qt::FindChildrenRecursively);
    part->widget()->setProperty("scrollbar", QVariant::fromValue(scrollbar));

    part->setProperty("session_controller", QVariant::fromValue(session_controller));

    return part;
}

void TermApp::new_window(TermPart* part, QString pwd=QString())
{
    TabWindow* tabs = new TabWindow();
    tabs->tabBar()->installEventFilter(this);
    tabs->show();
    tabs->new_tab(-1, part, pwd);
}

void TermApp::slotTermActivityDetected()
{
    TermPart* part = (TermPart*)QObject::sender();
    TabWindow* tabs = part->property("tabwidget").value<TabWindow*>();

    if (tabs->currentWidget() != part->widget() && ! part->property("has_activity").toBool()) {
        part->setProperty("has_activity", QVariant(true));
        part->setProperty("has_silence", QVariant(false));
        part->property("tabwidget").value<QTabWidget*>()->tabBar()->update();
    }
}

void TermApp::slotTermSilenceDetected()
{
    TermPart* part = (TermPart*)QObject::sender();
    TabWindow* tabs = part->property("tabwidget").value<TabWindow*>();

    if (tabs->currentWidget() != part->widget() && ! part->property("has_silence").toBool() && part->property("has_activity").toBool()) {
        part->setProperty("has_silence", QVariant(true));
        part->setProperty("has_activity", QVariant(false));
        part->property("tabwidget").value<QTabWidget*>()->tabBar()->update();
    }
}

void TermApp::slotTermSetWindowCaption(QString caption)
{
    TermPart* part = qobject_cast<TermPart*>(QObject::sender());
    part->setProperty("term_title", QVariant(caption));
    QTabWidget* tabs = part->property("tabwidget").value<QTabWidget*>();
    tabs->tabBar()->update();
    if (tabs->currentWidget() == part->widget()) {
        tabs->setWindowTitle(caption);
    }
}

void TermApp::slotTermOverrideShortcut(QKeyEvent*, bool &override)
{
    override = false;
}

void TermApp::drag_tabs(QPoint pos, bool split)
{
    QTabBar* bar;
    TabWindow* _new_window = qobject_cast<TabWindow*>(topLevelAt(pos));
    TabWindow* _old_window = dragged_part->property("tabwidget").value<TabWindow*>();

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

                        dragged_part = bar->tabData(current).value<TermPart*>();
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

    QString path = QStandardPaths::locate(QStandardPaths::AppConfigLocation, "/stylesheet.qss");
    if (! path.isNull()) {
        QFile stylesheet(path);
        if (stylesheet.open(QFile::ReadOnly)||1)
            app.setStyleSheet(stylesheet.readAll());
    }

    app.setWindowIcon(QIcon::fromTheme("utilities-terminal"));

    if (! app.konsole_service() ) {
        qCritical() << "Unable to start Konsole::Part service";
        app.quit();
        return 1;
    }
    app.new_window(NULL);

    return app.exec();
}
