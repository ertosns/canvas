#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QList>
#include <QAction>
#include <QWidget>
#include <memory>
#include <iostream>

#include "dialog_windows.h"
#include "canvas.h"
#include "toolbar.h"

using namespace std;

class Tool;

class MainWindow: public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = 0, const char* name = 0);
    ~MainWindow();

    void virtual mousePressEvent (QMouseEvent*) override;

public slots:
    void OnNewImage();
    void OnLoadImage();
    void OnSaveImage();
    void OnResizeImage();
    void OnPickColor(int);
    void OnChangeTool(int);

    void OnPenDialog();
    void OnLineDialog();
    void OnEraserDialog();
    void OnRectangleDialog();
    void OnPenSize(int);
private:
    void createMenuActions();
    void createMenuAndToolBar();

    void openToolDialog();

    Canvas* canvas;

    QList<QAction*> imageActions;
    QList<QAction*> toolActions;

    ToolBar* toolbar;

    Tool* currentTool;

    PenDialog* penDialog;
    LineDialog* lineDialog;
    EraserDialog* eraserDialog;
    RectDialog* rectDialog;

    MainWindow(const MainWindow&);
    MainWindow& operator=(const MainWindow&);
};

#endif // MAIN_WINDOW_H
