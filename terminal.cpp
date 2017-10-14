#include <QStackedLayout>
#include <QDir>

#include "terminal.h"

KService::Ptr _konsole_service;

KService::Ptr Terminal::konsole_service() {
    if (! _konsole_service) {
        _konsole_service = KService::serviceByDesktopName("konsolepart");
    }
    return _konsole_service;
}

QObject* find_child(QObject* parent, const char* classname) {
    foreach(QObject* child, parent->children()) {
        if (child->inherits(classname)) return child;
    }
    return NULL;
}

Terminal* Terminal::make_term(const QString& pwd, const QStringList& args) {
    if (! pwd.isEmpty())
        QDir::setCurrent(pwd);

    KParts::ReadOnlyPart* part = konsole_service()->createInstance<KParts::ReadOnlyPart>();
    if (! part) return NULL;
    return new Terminal(part, args);
}

// typedef QObject ViewProperties;
Terminal::Terminal(KParts::ReadOnlyPart* part, const QStringList& args) : QWidget() {
    m_part = part;
    part->setParent(this);
    new QStackedLayout(this);

    m_widget = part->widget();
    layout()->addWidget(m_widget);
    setFocusProxy(m_widget);

    if (! args.isEmpty())
        terminalInterface()->startProgram(args[0], args);

    // all manner of hacks below

    QMetaObject::invokeMethod(part, "setMonitorActivityEnabled", Qt::DirectConnection, Q_ARG(bool, true));
    QMetaObject::invokeMethod(part, "setMonitorSilenceEnabled", Qt::DirectConnection, Q_ARG(bool, true));

    connect(part, &QObject::destroyed, this, &QObject::deleteLater);
    connect(part, SIGNAL(activityDetected()), SLOT(activityDetected()));
    connect(part, SIGNAL(silenceDetected()), SLOT(silenceDetected()));

    connect(part, &KParts::Part::setWindowCaption, this, &Terminal::setWindowCaption);
    connect(part, SIGNAL(overrideShortcut(QKeyEvent*, bool&)), SLOT(slotTermOverrideShortcut(QKeyEvent*, bool&)) );

    QObject* view_mgr = find_child(part, "Konsole::ViewManager");
    m_session_controller = find_child(view_mgr, "Konsole::SessionController");

    m_scrollbar = m_widget->findChild<QScrollBar*>(QString(), Qt::FindChildrenRecursively);

    // hack initialise title
    QMetaObject::invokeMethod(part, "activeViewTitleChanged", Qt::DirectConnection, QArgument<QObject*>("ViewProperties*", m_session_controller));
}

Terminal::~Terminal() { }

void Terminal::slotTermOverrideShortcut(QKeyEvent*, bool &override) {
    override = false;
}

void Terminal::setWindowCaption(QString caption) {
    title = caption;

    if (tabwidget) {
        tabwidget->tabBar()->update();
        if (tabwidget->currentWidget() == this) {
            tabwidget->setWindowTitle(caption);
        }
    }
}

void Terminal::activityDetected() {
    if (tabwidget && tabwidget->currentWidget() != this && !has_activity) {
        has_activity = true;
        has_silence = false;
        tabwidget->tabBar()->update();
    }
}

void Terminal::silenceDetected() {
    if (tabwidget && tabwidget->currentWidget() != this && !has_silence && has_activity) {
        has_silence = true;
        has_activity = false;
        tabwidget->tabBar()->update();
    }
}
