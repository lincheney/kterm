#include <QTabWidget>

#include <KParts/ReadOnlyPart>
#include <KService>

class Tabs : public QTabWidget
{
    Q_OBJECT
public:
    Tabs();
    ~Tabs() {};
    void add_tab();

public Q_SLOTS:
    void slotTermDestroyed(QObject*);

private:
    void changed_tab(int);
    KService::Ptr m_service;
};
