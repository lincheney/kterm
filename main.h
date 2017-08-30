#include <QTabWidget>
#include <QApplication>
#include <QTabBar>
#include <QStyleOptionTab>

#include <KParts/ReadOnlyPart>
#include <KService>

typedef KParts::ReadOnlyPart TermPart;

class TermApp: public QApplication
{
    Q_OBJECT
public:
    TermApp(int &argc, char **argv) : QApplication(argc, argv) {};
    KService::Ptr konsole_service();
    TermPart* make_term();

public Q_SLOTS:
    void slotTermActivityDetected();
    void slotTermSetWindowCaption(QString);

protected:
    void updateTermTitle(QObject*);

private:
    KService::Ptr m_service;
};

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

typedef struct {
    KParts::ReadOnlyPart* term;
} Term;
