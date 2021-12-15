#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QToolBar>
#include <QAction>
#include <QList>

#include "constants.h"


class QWidget;

class ToolBar : public QToolBar
{
public:
    ToolBar(QWidget* parent, const QList<QAction*> &imageActions,
                             const QList<QAction*> &toolActions);

private:
    void createActions();

    QList<QAction*> imageActions;
    QList<QAction*> toolActions;

    ToolBar(const ToolBar&);
    ToolBar& operator=(const ToolBar&);
    QWidget *main;
    QSlider *penSize;
};

#endif // TOOLBAR_H
