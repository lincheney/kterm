#include <KParts/MainWindow>
#include <KParts/ReadOnlyPart>

class MainWindow : public KParts::MainWindow
{
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow() {};

public Q_SLOTS:
    void slotConsoleDestroyed();

private:
    KParts::ReadOnlyPart *m_part;
};
