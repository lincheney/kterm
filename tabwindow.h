#include <QTabBar>
#include <QPaintEvent>
#include <QTabWidget>
#include <QPoint>
#include <QSettings>
#include <kde_terminal_interface.h>

#include "main.h"

#ifndef TABWINDOW_H
#define TABWINDOW_H

class TabBar: public QTabBar
{
    Q_OBJECT
    Q_PROPERTY(QColor labelfg MEMBER m_labelfg DESIGNABLE true)
    Q_PROPERTY(QColor labelbg MEMBER m_labelbg DESIGNABLE true)
protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
private:
    QColor m_labelfg;
    QColor m_labelbg;
};

class TabWindow : public QTabWidget
{
    Q_OBJECT
public:
    TabWindow();
    ~TabWindow() {};
    int new_tab(int, TermPart*, QString = QString());
    void load_settings(QSettings*);

protected:
    void closeEvent(QCloseEvent*);

private:
    void changed_tab(int);
    int offset_index(int);
    QString current_dir();
};

#endif
