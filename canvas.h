#ifndef CANVAS_H
#define CANVAS_H

#include <QUndoStack>
#include <QSlider>
#include <QGridLayout>

#include "constants.h"
#include "tool.h"

#include <vtkActor.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>
#include <vtkVersion.h>
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>
#include <QDebug>
#include <vtkWindowToImageFilter.h>

class Canvas : public QWidget
{
    Q_OBJECT

public:
    QGridLayout *grid;
    QWidget *vtkwidget;
    QVTKOpenGLNativeWidget *widget;
    vtkNew<vtkNamedColors> colors;

    Canvas(QWidget *parent);
    ~Canvas();

    PenTool* get_pen() { return penTool; }
    LineTool* get_line() { return lineTool; }
    EraserTool* get_eraser() { return eraserTool; }
    RectTool* get_rect() { return rectTool; }
    QPixmap* getImage() { return image; }
    Tool* getCurrentTool() const { return currentTool; }
    QColor getForegroundColor() { return foregroundColor; }
    QColor getBackgroundColor() { return backgroundColor; }

    Tool* setCurrentTool(int);
    void setLineMode(const DrawType mode);

    void createNewImage();
    void loadImage(const QString&);
    void saveImage(const QString&);
    void resizeImage();
    void clearImage();
    void updateColorConfig(const QColor&, int);

    void saveDrawCommand(const QPixmap&);

public slots:

    void OnUndo();
    void OnRedo();
    void OnClearAll();
    void OnPenCapConfig(int);
    void OnPenSizeConfig(int);
    void OnEraserConfig(int);
    void OnLineStyleConfig(int);
    void OnLineCapConfig(int);
    void OnDrawTypeConfig(int);
    void OnLineThicknessConfig(int);
    void OnRectBStyleConfig(int);
    void OnRectShapeTypeConfig(int);
    void OnRectFillConfig(int);
    void OnRectBTypeConfig(int);
    void OnRectLineConfig(int);
    void OnRectCurveConfig(int);
    void add_cube();
    void add_sphere();
    void add_cylinder();

protected:
    void virtual mousePressEvent(QMouseEvent *event) override;
    void virtual mouseMoveEvent(QMouseEvent *event) override;
    void virtual mouseReleaseEvent(QMouseEvent *event) override;
    void virtual mouseDoubleClickEvent(QMouseEvent *event) override;

    void virtual paintEvent(QPaintEvent *event) override;

private:
    void createTools();
    QUndoStack* undoStack;

    Tool* currentTool;
    DrawType currentLineMode;

    QPixmap* image;
    QPixmap oldImage;

    QColor foregroundColor;
    QColor backgroundColor;

    PenTool* penTool;
    LineTool* lineTool;
    EraserTool* eraserTool;
    RectTool* rectTool;
    RenderTool* renderTool;

    bool drawing;
    bool drawingPoly;
    bool drawing3d;

    Canvas(const Canvas&);
    Canvas& operator=(const Canvas&);
};

extern bool imagesEqual(const QPixmap& image1, const QPixmap& image2);

#endif // CANVAS_H
