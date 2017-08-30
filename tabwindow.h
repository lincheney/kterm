#include <QTabBar>
#include <QPaintEvent>
#include <QTabWidget>

class TabBar: public QTabBar
{
    Q_OBJECT
protected:
    void paintEvent(QPaintEvent *);
};

class TabWindow : public QTabWidget
{
    Q_OBJECT
public:
    TabWindow();
    ~TabWindow() {};
    void add_tab();

private:
    void changed_tab(int);
};
