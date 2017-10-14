#include <QTabBar>
#include <QPaintEvent>
#include <QTabWidget>
#include <QPoint>
#include <QSettings>

#include "terminal.h"

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
    int new_tab(int, Terminal*, QString = QString());
    void load_settings(QSettings*);
    Terminal* currentTerminal() { return qobject_cast<Terminal*>(currentWidget()); };

protected:
    void closeEvent(QCloseEvent*);

private:
    void changed_tab(int);
    int offset_index(int);
    QString current_dir();
};

#endif
