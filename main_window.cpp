#include <iostream>
using namespace std;

#include <QDesktopWidget>
#include <QMouseEvent>
#include <QFileDialog>
#include <QColorDialog>
#include <QSignalMapper>
#include <QMenuBar>
#include <QMenu>
#include <QGridLayout>

#include "main_window.h"
#include "commands.h"
#include "canvas.h"


/**
 * @brief MainWindow::MainWindow - the main window, parent to every other
 *                                 widget.
 */
MainWindow::MainWindow(QWidget* parent, const char* name)
    :QMainWindow(parent)
{
    penDialog=0;
    eraserDialog=0;
    lineDialog=0;
    rectDialog=0;

    QWidget *window = new QWidget(parent);
    canvas = new Canvas(window); //TODO (res)
    canvas->grid = new QGridLayout;
    canvas->vtkwidget = new QWidget(window);
    canvas->vtkwidget->setLayout(canvas->grid);
    canvas->grid->addWidget(canvas, 0, 0);
    canvas->grid->addWidget(canvas->vtkwidget, 0, 1);
    window->setLayout(canvas->grid);

    canvas->setStyleSheet("background-color:transparent");

    // get default tool
    currentTool = canvas->getCurrentTool();

    // create the menu and toolbar
    createMenuAndToolBar();

    // adjust window size, name, & stop context menu
    setWindowTitle(name);
    resize(QDesktopWidget().availableGeometry(this).size()*.6);
    setContextMenuPolicy(Qt::PreventContextMenu);
    setCentralWidget(window);
}

MainWindow::~MainWindow()
{
    imageActions.clear();
    toolActions.clear();
}

/**
 * @brief MainWindow::mousePressEvent - On mouse right click, open dialog menu.
 *
 */

void MainWindow::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::RightButton) {
        openToolDialog();
    }
}

/**
 * @brief MainWindow::OnNewImage - Open a NewCanvasDialogue prompting user to
 *                                 enterthe dimensions for a new image.
 */
void MainWindow::OnNewImage()
{
    canvas->createNewImage();
}

/**
 * @brief MainWindow::OnLoadImage - Open a QFileDialogue prompting user to
 *                                  browse for a file to open.
 */
void MainWindow::OnLoadImage()
{
    QString s = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    ".",
                                                    tr("BMP image (*.bmp)"));
    if (! s.isNull())
    {
        canvas->loadImage(s);
    }
}

/**
 * @brief MainWindow::OnSaveImage - Open a QFileDialogue prompting user to
 *                                  enter a filename and save location.
 */
void MainWindow::OnSaveImage()
{
    if(canvas->getImage()->isNull())
        return;

    // use custom dialog settings for appending suffixes
    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setAcceptMode(QFileDialog::AcceptSave);
    fileDialog->setDirectory(".");
    fileDialog->setNameFilter("BMP image (*.bmp)");
    fileDialog->setDefaultSuffix("bmp");
    fileDialog->exec();

    // if user hit 'OK' button, save file
    if (fileDialog->result())
    {
        QString s = fileDialog->selectedFiles().first();

        if (! s.isNull())
        {
            canvas->saveImage(s);
        }
    }
    // done with the dialog, free it
    delete fileDialog;
}

/**
 * @brief MainWindow::OnResizeImage - Change the dimensions of the image.
 *
 */
void MainWindow::OnResizeImage()
{
    QPixmap *image = canvas->getImage();
    if(image->isNull())
        return;
    canvas->resizeImage();
    /*
    CanvasSizeDialog* newCanvas = new CanvasSizeDialog(this, "Resize Image",
                                                       image->width(),
                                                       image->height());
    newCanvas->exec();
    // if user hit 'OK' button, create new image
    if (newCanvas->result())
    {
         canvas->resizeImage(QSize(newCanvas->getWidthValue(),
                                     newCanvas->getHeightValue()));
    }
    // done with the dialog, free it
    delete newCanvas;
    */
}

/**
 * @brief MainWindow::OnPickColor - Open a QColorDialog prompting the user to
 *                                  select a color.
 *
 */
void MainWindow::OnPickColor(int which)
{
    QColorDialog* colorDialog = new QColorDialog(this);
    QColor foregroundColor = canvas->getForegroundColor();
    QColor backgroundColor = canvas->getBackgroundColor();
    QColor color
        = colorDialog->getColor(which == foreground ? foregroundColor
                                : backgroundColor,
                                this,
                                which == foreground ? "Foreground Color"
                                : "Background Color",
                                QColorDialog::DontUseNativeDialog);
    if (color.isValid())
        canvas->updateColorConfig(color, which);

    delete colorDialog;
}

/**
 * @brief MainWindow::OnChangeTool - Sets the current tool based on argument.
 *
 */
void MainWindow::OnChangeTool(int newTool)
{
    currentTool = canvas->setCurrentTool(newTool); // notify observer
}

/**
 * @brief MainWindow::OnPenDialog - Open a PenDialog prompting the user to
 *                                  change pen settings.
 *
 */
void MainWindow::OnPenDialog()
{
    if(!penDialog)
        penDialog = new PenDialog(this, canvas);

    if(penDialog && penDialog->isVisible())
        return;

    penDialog->show();
}

/**
 * @brief MainWindow::OnLineDialog - Open a LineDialog prompting the user to
 *                                   change line tool settings.
 *
 */
void MainWindow::OnLineDialog()
{
    if(!lineDialog)
        lineDialog = new LineDialog(this, canvas);

    if(lineDialog && lineDialog->isVisible())
        return;

    lineDialog->show();
}

/**
 * @brief MainWindow::OnEraserDialog - Open a EraserDialog prompting the user
 *                                     to change eraser settings.
 *
 */
void MainWindow::OnEraserDialog()
{
    if (!eraserDialog)
        eraserDialog = new EraserDialog(this, canvas);

    if(eraserDialog->isVisible())
        return;

    eraserDialog->show();
}

/**
 * @brief MainWindow::OnRectangleDialog - Open a RectDialog prompting the user
 *                                        to change rect tool settings.
 *
 */
void MainWindow::OnRectangleDialog()
{
    if (!rectDialog)
        rectDialog = new RectDialog(this, canvas);

    if(rectDialog->isVisible())
        return;

    rectDialog->show();
}

/**
 * @brief MainWindow::openToolDialog - call the appropriate dialog function
 *                                     based on the current tool.
 *
 */
void MainWindow::openToolDialog()
{
    switch(currentTool->getType())
    {
    case pen: OnPenDialog();             break;
    case line: OnLineDialog();           break;
    case eraser: OnEraserDialog();       break;
    case rect_tool: OnRectangleDialog(); break;
    case render3d: break;
    }
}

void MainWindow::OnPenSize(int s) {
    canvas->get_pen()->setWidth(s);
    canvas->get_eraser()->setWidth(s);
    canvas->get_line()->setWidth(s);
    canvas->get_rect()->setWidth(s);
}

/**
 * @brief ToolBar::createMenuAndToolBar() - ensure that everything gets
 *                                          created in the correct order
 *
 */
void MainWindow::createMenuAndToolBar()
{
    // create actions and add them to the menu
    createMenuActions();

    // create the toolbar
    toolbar = new ToolBar(this, imageActions, toolActions);
    addToolBar(toolbar);

}

/**
 * @brief MainWindow::createMenu - create the actions that appear in the menu
 *
 */
void MainWindow::createMenuActions()
{
    ///////////////
    // resources //
    //////////////
    QIcon new_icon(":/resources/new");
    QIcon open_icon(":/resources/open");
    QIcon save_icon(":/resources/save");

    QIcon resize_icon(":/resources/resize");
    QIcon clear_icon(":/resources/clearAll");
    QIcon fColor_icon(":/resources/fColor");
    QIcon bColor_icon(":/resources/bColor");

    QIcon pen_icon(":/resources/pen");
    QIcon line_icon(":/resources/line");
    QIcon eraser_icon(":/resources/eraser");
    QIcon rect_icon(":/resources/rect");
    QIcon cube_icon(":/resources/cube");
    QIcon sphere_icon(":/resources/sphere");
    QIcon cylinder_icon(":/resources/cylinder");
    QIcon render_icon(":/resources/render");
    //////////
    // File //
    //////////
    QMenu* file = new QMenu(tr("File"), this);

    QAction *new_action = new QAction();
    new_action->setIcon(new_icon);
    new_action->setText(QString("new"));
    new_action->setShortcut(QKeySequence("Ctrl+N"));
    connect(new_action, SIGNAL(triggered()), this, SLOT(OnNewImage()));
    file->addAction(new_action);
    imageActions.append(new_action);

    QAction *load_action = new QAction();
    load_action->setIcon(open_icon);
    load_action->setText(QString("load"));
    load_action->setShortcut(QKeySequence("Ctrl+L"));
    connect(load_action, SIGNAL(triggered()), this, SLOT(OnLoadImage()));
    file->addAction(load_action);
    imageActions.append(load_action);

    QAction *save_action = new QAction();
    save_action->setIcon(save_icon);
    save_action->setText(QString("save"));
    save_action->setShortcut(QKeySequence("Ctrl+S"));
    connect(save_action, SIGNAL(triggered()), this, SLOT(OnSaveImage()));
    file->addAction(save_action);
    imageActions.append(save_action);

    QAction *quit_action = new QAction();
    quit_action->setText("quit");
    quit_action->setShortcut(QKeySequence("Ctrl+Q"));
    connect(quit_action, SIGNAL(triggered()), this, SLOT(close()));
    file->addAction(quit_action);


    //////////
    // Edit //
    //////////
    QMenu* edit = new QMenu(QString("Edit"), this);

    QAction *resize_action = new QAction();
    resize_action->setIcon(resize_icon);
    resize_action->setText(QString("resize"));
    resize_action->setShortcut(QKeySequence("Ctrl+R"));
    connect(resize_action, SIGNAL(triggered()), this, SLOT(OnResizeImage()));
    edit->addAction(resize_action);
    imageActions.append(resize_action);

    QAction *undo_action  = new QAction();
    undo_action->setText(QString("undo"));
    undo_action->setShortcut(QKeySequence("Ctrl+Z"));
    connect(undo_action, SIGNAL(triggered()), canvas, SLOT(OnUndo()));
    edit->addAction(undo_action);
    //imageActions.append(undo_action);

    QAction *redo_action  = new QAction();
    redo_action->setText(QString("redo"));
    redo_action->setShortcut(QKeySequence("Ctrl+Shift+Z"));
    connect(redo_action, SIGNAL(triggered()), canvas, SLOT(OnRedo()));
    edit->addAction(redo_action);
    //imageActions.append(redo_action);

    QAction *clear_action  = new QAction();
    clear_action->setIcon(clear_icon);
    clear_action->setText(QString("clear"));
    clear_action->setShortcut(QKeySequence("Ctrl+C"));
    connect(clear_action, SIGNAL(triggered()), canvas, SLOT(OnClearAll()));
    edit->addAction(clear_action);
    imageActions.append(clear_action);

    QSignalMapper *color_qsm = new QSignalMapper(this);

    QAction* fcolor_action = new QAction(fColor_icon, QString("palette"), this);
    color_qsm->setMapping(fcolor_action, foreground);
    connect(fcolor_action, SIGNAL(triggered()), color_qsm, SLOT(map()));
    edit->addAction(fcolor_action);
    imageActions.append(fcolor_action);

    QAction* bcolor_action = new QAction(bColor_icon, QString("Background Color"), this);
    connect(bcolor_action, SIGNAL(triggered()), color_qsm, SLOT(map()));
    color_qsm->setMapping(bcolor_action, background);
    connect(color_qsm, SIGNAL(mapped(int)), this, SLOT(OnPickColor(int)));
    edit->addAction(bcolor_action);
    imageActions.append(bcolor_action);

    //////////
    // Tool //
    //////////
    QMenu* tools = new QMenu(tr("Tools"), this);

    QSignalMapper *tool_qsm = new QSignalMapper(this);

    QAction* pen_action = new QAction(pen_icon, QString("Pen"), this);
    connect(pen_action, SIGNAL(triggered()), tool_qsm, SLOT(map()));
    tools->addAction(pen_action);
    toolActions.append(pen_action);

    QAction* eraser_action = new QAction(eraser_icon, QString("Eraser"), this);
    connect(eraser_action, SIGNAL(triggered()), tool_qsm, SLOT(map()));
    tools->addAction(eraser_action);
    toolActions.append(eraser_action);

    QAction* line_action = new QAction(line_icon, QString("Line"), this);
    connect(line_action, SIGNAL(triggered()), tool_qsm, SLOT(map()));
    tools->addAction(line_action);
    toolActions.append(line_action);

    QAction* rect_action = new QAction(rect_icon, QString("Rectangle"), this);
    connect(rect_action, SIGNAL(triggered()), tool_qsm, SLOT(map()));
    tools->addAction(rect_action);

    QAction *cube_action = new QAction(cube_icon, QString("Cube"), this);
    connect(cube_action, SIGNAL(triggered()), canvas, SLOT(add_cube()));
    tools->addAction(cube_action);
    toolActions.append(cube_action);

    QAction *sphere_action = new QAction(sphere_icon, QString("sphere"), this);
    connect(sphere_action, SIGNAL(triggered()), canvas, SLOT(add_sphere()));
    tools->addAction(sphere_action);
    toolActions.append(sphere_action);

    QAction *cylinder_action = new QAction(cylinder_icon, QString("cylinder"), this);
    connect(cylinder_action, SIGNAL(triggered()), canvas, SLOT(add_cylinder()));
    tools->addAction(cylinder_action);
    toolActions.append(cylinder_action);

    QAction *render_action = new QAction();
    render_action->setIcon(render_icon);
    render_action->setText(QString("render 3d"));
    connect(render_action, SIGNAL(triggered()), tool_qsm, SLOT(map()));
    tools->addAction(render_action);
    toolActions.append(render_action);
    //


    tool_qsm->setMapping(pen_action, pen);
    tool_qsm->setMapping(line_action, line);
    tool_qsm->setMapping(eraser_action, eraser);
    tool_qsm->setMapping(rect_action, rect_tool);
    tool_qsm->setMapping(render_action, render3d);
    connect(tool_qsm, SIGNAL(mapped(int)), this, SLOT(OnChangeTool(int)));
    toolActions.append(rect_action);

    tools->addAction(QString("Pen Properties"), this, SLOT(OnPenDialog()));
    tools->addAction(QString("Eraser Properties"), this, SLOT(OnEraserDialog()));
    tools->addAction(QString("Line Properties"), this, SLOT(OnLineDialog()));
    tools->addAction(QString("Rectangle Properties"), this, SLOT(OnRectangleDialog()));
    ///////////////////////
    // populate menu-bar //
    ///////////////////////
    menuBar()->addMenu(file);
    menuBar()->addMenu(edit);
    menuBar()->addMenu(tools);
}
