#include <QTabBar>
#include <QPaintEvent>
#include <QTabWidget>

class TabBar: public QTabBar
{
    Q_OBJECT
protected:
    void paintEvent(QPaintEvent *);
};

class Tabs : public QTabWidget
{
    Q_OBJECT
public:
    Tabs();
    ~Tabs() {};
    void add_tab();

private:
    void changed_tab(int);
};
