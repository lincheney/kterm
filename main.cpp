#include <QApplication>
#include <QPushButton>
#include "main.h"

#include <KService>

MainWindow::MainWindow() : KParts::MainWindow()
{
    KService::Ptr service = KService::serviceByDesktopName("konsolepart");

    if (! service) {
        qApp->quit();
        return;
    }

    m_part = service->createInstance<KParts::ReadOnlyPart>(0);

    if (! m_part) {
        qApp->quit();
        return;
    }

    setCentralWidget(m_part->widget());
    connect(m_part, SIGNAL(destroyed()), this, SLOT(slotConsoleDestroyed()) );
}

void MainWindow::slotConsoleDestroyed()
{
    qApp->quit();
}


int main (int argc, char **argv)
{
    QApplication app(argc, argv);

    MainWindow* window = new MainWindow();
    window->show();

    return app.exec();
}
