#include <QTabBar>
#include <QPaintEvent>
#include <QTabWidget>

#include "main.h"

#ifndef TABWINDOW_H
#define TABWINDOW_H

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
    TabWindow(TermPart* = NULL);
    ~TabWindow() {};
    void new_tab();

private:
    void changed_tab(int);
};

#endif
