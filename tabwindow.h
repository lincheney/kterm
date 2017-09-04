#include <QTabBar>
#include <QPaintEvent>
#include <QTabWidget>
#include <QPoint>

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
    TabWindow();
    ~TabWindow() {};
    int new_tab(int, TermPart*);

private:
    void changed_tab(int);
    int offset_index(int);
};

#endif
