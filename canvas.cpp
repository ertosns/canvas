#include <QPainter>
#include <QPaintEvent>

//
#include <vtkActor.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>
#include <vtkCubeSource.h>
#include <vtkCylinderSource.h>
#include <vtkVersion.h>


#if VTK_VERSION_NUMBER >= 89000000000ULL
#define VTK890 1
#endif

#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>
#include <QDebug>
#include <vtkWindowToImageFilter.h>
#include <QRgba64>
#include <QException>
#include <exception>


//
#include "commands.h"
#include "canvas.h"
#include "main_window.h"


/**
 * @brief Canvas::Canvas - constructor for our Draw Area.
 *                             Pointers to the MainWindow's
 *                             tools and image are mandatory
 *
 */
Canvas::Canvas(QWidget *parent)
    : QWidget(parent)
{
    // initialize the undo stack
    undoStack = new QUndoStack(this);
    undoStack->setUndoLimit(UNDO_LIMIT);

    // initialize image
    image = new QPixmap();

    //create the pen, line, eraser, & rect tools
    createTools();

    // initialize colors
    foregroundColor = Qt::red;
    backgroundColor = Qt::white;

    // initialize state variables
    drawing = false;
    drawingPoly = false;
    drawing3d = false;
    currentLineMode = single;

    // small optimizations
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_StaticContents);

    widget = new QVTKOpenGLNativeWidget;
}

Canvas::~Canvas()
{
    delete image;
    delete penTool;
    delete lineTool;
    delete eraserTool;
    delete rectTool;
}


/**
 * @brief Canvas::paintEvent - redraw part of the image based
 *                               on what was modified
 *
 */
void Canvas::paintEvent(QPaintEvent *e)

{
    QPainter painter(this);
    QRect modifiedArea = e->rect(); // only need to redraw a small area
    painter.drawPixmap(modifiedArea, *image, modifiedArea);
}

/**
 * @brief Canvas::mousePressEvent - left-click initiates a draw
 *
 *                                  - right-click opens a dialog
 *                                    menu for the current tool
 *
 */
void Canvas::mousePressEvent(QMouseEvent *e)
{

    if(e->button() == Qt::RightButton)
    {
        // open the dialog menu
        static_cast<MainWindow*>(parent())->mousePressEvent(e);
    }
    else if (e->button() == Qt::LeftButton)
    {
        if(image->isNull())
            return;

        drawing = true;

        if(!drawingPoly) {
            currentTool->setStartPoint(e->pos());
        }

        // save a copy of the old image
        oldImage = image->copy(QRect());
    }
}

/**
 * @brief Canvas::mouseMoveEvent - draw
 *
 */
void Canvas::mouseMoveEvent(QMouseEvent *e)
{
    if (e->buttons() & Qt::LeftButton && drawing)
    {
        if(image->isNull())
            return;

        ToolType type = currentTool->getType();
        if(type == line || type == rect_tool)
        {
            *image = oldImage;
            if(type == line && currentLineMode == poly)
            {
                drawingPoly = true;
            }
        }
        if (type != render3d) {
            currentTool->drawTo(e->pos(), this, image);
        }
    }
}

/**
 * @brief Canvas::mouseReleaseEvent - finish drawing
 *
 */
void Canvas::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && drawing)
    {
        drawing = false;

        if(image->isNull())
            return;

        if (drawing3d) {
            oldImage=image->copy(QRect());
            currentTool->drawTo(e->pos(), this, image);
        }

        if(drawingPoly)
        {
            currentTool->setStartPoint(e->pos());
            //return;
        }
        if(currentTool->getType() == pen)
            currentTool->drawTo(e->pos(), this, image);

        // for undo/redo - make sure there was a change
        // (in case drawing began off-image)
        if(oldImage.toImage() != image->toImage())
            saveDrawCommand(oldImage);
    }
}

/**
 * @brief Canvas::mouseDoubleClickEvent - cancel poly mode
 *
 */
void Canvas::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        if(drawingPoly)
            drawingPoly = false;
    }
}

void Canvas::add_cube() {
    #if VTK890
    widget->setRenderWindow(renderTool->renderWindow);
#else
    widget->SetRenderWindow(renderTool->renderWindow);
#endif

    widget->resize(100, 100);
    vtkNew<vtkCubeSource> source;

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(source->GetOutputPort());

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    //actor->GetProperty()->SetColor(colors->GetColor4d("Tomato").GetData());
    actor->GetProperty()->SetColor(foregroundColor.red()/255.0,
                                   foregroundColor.green()/255.0,
                                   foregroundColor.blue()/255.0);

    vtkNew<vtkRenderer> renderer;
    renderer->AddActor(actor);
    renderer->SetBackground(backgroundColor.red()/255.0,
                            backgroundColor.green()/255.0,
                            backgroundColor.blue()/255.0);
#if VTK890
    widget->renderWindow()->AddRenderer(renderer);
    widget->renderWindow()->SetWindowName("RenderWindowNoUIFile");
#else
    widget->GetRenderWindow()->AddRenderer(renderer);
    widget->GetRenderWindow()->SetWindowName("RenderWindowNoUIFile");

#endif
    //
    grid->setVerticalSpacing(50);
    grid->setHorizontalSpacing(50);

    grid->addWidget(widget,0,2);
    update();
}

void Canvas::add_sphere(){
#if VTK890
    widget->setRenderWindow(renderTool->renderWindow);
#else
    widget->SetRenderWindow(renderTool->renderWindow);
#endif

    widget->resize(100, 100);
    vtkNew<vtkSphereSource> sphereSource;

    vtkNew<vtkPolyDataMapper> sphereMapper;
    sphereMapper->SetInputConnection(sphereSource->GetOutputPort());

    vtkNew<vtkActor> actor;
    actor->SetMapper(sphereMapper);
    actor->GetProperty()->SetColor(foregroundColor.red()/255.0,
                                   foregroundColor.green()/255.0,
                                   foregroundColor.blue()/255.0);

    vtkNew<vtkRenderer> renderer;
    renderer->AddActor(actor);
    renderer->SetBackground(backgroundColor.red()/255.0,
                            backgroundColor.green()/255.0,
                            backgroundColor.blue()/255.0);
#if VTK890
    widget->renderWindow()->AddRenderer(renderer);
    widget->renderWindow()->SetWindowName("RenderWindowNoUIFile");
#else
    widget->GetRenderWindow()->AddRenderer(renderer);
    widget->GetRenderWindow()->SetWindowName("RenderWindowNoUIFile");
#endif
    //
    grid->setVerticalSpacing(50);
    grid->setHorizontalSpacing(50);

    grid->addWidget(widget,0,2);
    update();
}

void Canvas::add_cylinder(){
#if VTK890
    widget->setRenderWindow(renderTool->renderWindow);
#else
    widget->SetRenderWindow(renderTool->renderWindow);
#endif

    widget->resize(100, 100);
    vtkNew<vtkCylinderSource> source;
    source->SetResolution(8);

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(source->GetOutputPort());

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(foregroundColor.red()/255.0,
                                   foregroundColor.green()/255.0,
                                   foregroundColor.blue()/255.0);
    vtkNew<vtkRenderer> renderer;
    renderer->AddActor(actor);
    renderer->SetBackground(backgroundColor.red()/255.0,
                            backgroundColor.green()/255.0,
                            backgroundColor.blue()/255.0);
#if VTK890
    widget->renderWindow()->AddRenderer(renderer);
    widget->renderWindow()->SetWindowName("RenderWindowNoUIFile");
#else
    widget->GetRenderWindow()->AddRenderer(renderer);
    widget->GetRenderWindow()->SetWindowName("RenderWindowNoUIFile");
#endif
    //
    grid->setVerticalSpacing(50);
    grid->setHorizontalSpacing(50);

    grid->addWidget(widget,0,2);
    update();
}

/**
 * @brief Canvas::OnSaveImage - Undo a previous action
 *
 */
void Canvas::OnUndo()
{
    if(!undoStack->canUndo())
        return;

    undoStack->undo();
    update();
}

/**
 * @brief Canvas::OnRedo - Redo a previously undone action
 *
 */
void Canvas::OnRedo()
{
    if(!undoStack->canRedo())
        return;

    undoStack->redo();
    update();
}

/**
 * @brief Canvas::OnClearAll - Clear the image
 *
 */
void Canvas::OnClearAll()
{
    if(image->isNull())
        return;

    clearImage();
}

/**
 * @brief Canvas::OnPenCapConfig - Update cap style for pen tool
 *
 */
void Canvas::OnPenCapConfig(int capStyle)
{
    switch (capStyle)
    {
        case flat: penTool->setCapStyle(Qt::FlatCap);       break;
        case square: penTool->setCapStyle(Qt::SquareCap);   break;
        case round_cap: penTool->setCapStyle(Qt::RoundCap); break;
        default:                                            break;
    }
}

/**
 * @brief Canvas::OnPenSizeConfig - Update pen size
 *
 */
void Canvas::OnPenSizeConfig(int value)
{
    penTool->setWidth(value);
}

/**
 * @brief Canvas::OnEraserConfig - Update eraser thickness
 *
 */
void Canvas::OnEraserConfig(int value)
{
    eraserTool->setWidth(value);
}

/**
 * @brief Canvas::OnLineStyleConfig - Update line style for line tool
 *
 */
void Canvas::OnLineStyleConfig(int lineStyle)
{
    switch (lineStyle)
    {
        case solid: lineTool->setStyle(Qt::SolidLine);                break;
        case dashed: lineTool->setStyle(Qt::DashLine);                break;
        case dotted: lineTool->setStyle(Qt::DotLine);                 break;
        case dash_dotted: lineTool->setStyle(Qt::DashDotLine);        break;
        case dash_dot_dotted: lineTool->setStyle(Qt::DashDotDotLine); break;
        default:                                                      break;
    }
}

/**
 * @brief Canvas::OnLineCapConfig - Update cap style for line tool
 *
 */
void Canvas::OnLineCapConfig(int capStyle)
{
    switch (capStyle)
    {
        case flat: lineTool->setCapStyle(Qt::FlatCap);       break;
        case square: lineTool->setCapStyle(Qt::SquareCap);   break;
        case round_cap: lineTool->setCapStyle(Qt::RoundCap); break;
        default:                                             break;
    }
}

/**
 * @brief Canvas::OnDrawTypeConfig - Update draw type for line tool
 *
 */
void Canvas::OnDrawTypeConfig(int drawType)
{
    switch (drawType)
    {
        case single: setLineMode(single); break;
        case poly:   setLineMode(poly);   break;
        default:     break;
    }
}

/**
 * @brief Canvas::OnLineThicknessConfig - Update line thickness for line tool
 *
 */
void Canvas::OnLineThicknessConfig(int value)
{
    lineTool->setWidth(value);
}

/**
 * @brief Canvas::OnRectBStyleConfig - Update rectangle boundary line style
 *
 */
void Canvas::OnRectBStyleConfig(int boundaryStyle)
{
    switch (boundaryStyle)
    {
        case solid: rectTool->setStyle(Qt::SolidLine);                break;
        case dashed: rectTool->setStyle(Qt::DashLine);                break;
        case dotted: rectTool->setStyle(Qt::DotLine);                 break;
        case dash_dotted: rectTool->setStyle(Qt::DashDotLine);        break;
        case dash_dot_dotted: rectTool->setStyle(Qt::DashDotDotLine); break;
        default:                                                      break;
    }
}

/**
 * @brief Canvas::OnRectShapeTypeConfig - Update rectangle shape setting
 *
 */
void Canvas::OnRectShapeTypeConfig(int shape)
{
    switch (shape)
    {
        case rectangle: rectTool->setShapeType(rectangle);                 break;
        case rounded_rectangle: rectTool->setShapeType(rounded_rectangle); break;
        case ellipse: rectTool->setShapeType(ellipse);                     break;
        default:                                                           break;
    }
}

/**
 * @brief Canvas::OnRectFillConfig - Update rectangle fill setting
 *
 */
void Canvas::OnRectFillConfig(int fillType)
{
    switch (fillType)
    {
        case foreground: rectTool->setFillMode(foreground);
                         rectTool->setFillColor(foregroundColor);      break;
        case background: rectTool->setFillMode(background);
                         rectTool->setFillColor(backgroundColor);      break;
        case no_fill: rectTool->setFillMode(no_fill);
                      rectTool->setFillColor(QColor(Qt::transparent)); break;
        default:                                                       break;
    }
}

/**
 * @brief Canvas::OnRectBTypeConfig - Update rectangle join style
 *
 */
void Canvas::OnRectBTypeConfig(int boundaryType)
{
    switch (boundaryType)
    {
        case miter_join: rectTool->setJoinStyle(Qt::MiterJoin);  break;
        case bevel_join: rectTool->setJoinStyle(Qt::BevelJoin);  break;
        case round_join: rectTool->setJoinStyle(Qt::RoundJoin);  break;
        default:                                                 break;
    }
}

/**
 * @brief Canvas::OnRectLineConfig - Update rectangle line width
 *
 */
void Canvas::OnRectLineConfig(int value)
{
    rectTool->setWidth(value);
}

/**
 * @brief Canvas::OnRectCurveConfig - Update rounded rectangle curve
 *
 */
void Canvas::OnRectCurveConfig(int value)
{
    rectTool->setCurve(value);
}

/**
 * @brief Canvas::createNewImage - creates a new image of
 *                                   user-specified dimensions
 *
 */
void Canvas::createNewImage()
{
    // save a copy of the old image
    oldImage = image->copy();
    *image = QPixmap(this->size());
    image->fill(backgroundColor);
    update();

    // for undo/redo
    if(!imagesEqual(oldImage, *image))
        saveDrawCommand(oldImage);
}

/**
 * @brief Canvas::loadImage - Load an image from a user-specified file
 *
 */
void Canvas::loadImage(const QString &fileName)
{
    // save a copy of the old image
    oldImage = image->copy();

    image->load(fileName);
    update();

    // for undo/redo
    if(!imagesEqual(oldImage, *image))
        saveDrawCommand(oldImage);
}

/**
 * @brief Canvas::saveImage - Save an image to user-specified file
 *
 */
void Canvas::saveImage(const QString &fileName)
{
    image->save(fileName, "BMP");
}

/**
 * @brief Canvas::resizeImage - Resize image to user-specified dimensions
 *
 */
void Canvas::resizeImage()
{
    // save a copy of the old image
    oldImage = image->copy();


    // else re-scale the image
    *image = image->scaled(this->size(), Qt::IgnoreAspectRatio);
    update();

    // for undo/redo
    saveDrawCommand(oldImage);
}

/**
 * @brief Canvas::clearImage - clears an image by filling it with
 *                               the background color
 *
 */
void Canvas::clearImage()
{
    // save a copy of the old image
    oldImage = image->copy();

    image->fill(backgroundColor);
    update(image->rect());

    // for undo/redo
    if(!imagesEqual(oldImage, *image))
        saveDrawCommand(oldImage);
}

/**
 * @brief Canvas::updateColorConfig - Updates the tools' colors
 *                                      as appropriate
 *
 */
void Canvas::updateColorConfig(const QColor &color, int which)
{
    if(which == foreground)
    {
         foregroundColor = color;
         penTool->setColor(foregroundColor);
         lineTool->setColor(foregroundColor);
         rectTool->setColor(foregroundColor);

         if(rectTool->getFillMode() == foreground)
             rectTool->setFillColor(foregroundColor);
    }
    else
    {
        backgroundColor = color;
        eraserTool->setColor(backgroundColor);

        if(rectTool->getFillMode() == background)
            rectTool->setFillColor(backgroundColor);
    }
}

/**
 * @brief Canvas::setCurrentTool - Sets the current tool, unsetting
 *                                   poly mode if necessary
 *
 */
Tool* Canvas::setCurrentTool(int newType)
{
    // get the current tool's type
    int currType = currentTool->getType();

    // if no change, return --else cancel poly mode & set tool
    if(newType == currType)
        return currentTool;

    if(currType == line)
        drawingPoly = false;

    switch(newType)
    {
    case pen: currentTool = penTool;        break;
    case line: currentTool = lineTool;      break;
    case eraser: currentTool = eraserTool;  break;
    case rect_tool: currentTool = rectTool; break;
    case render3d: {
        drawing3d=true;
        currentTool = renderTool; break;

    }
    default:                                break;
    }
    return currentTool;
}

/**
 * @brief Canvas::setLineMode - Sets the current line draw mode,
 *                                unsetting poly mode if necessary
 *
 */
void Canvas::setLineMode(const DrawType mode)
{
    if(mode == single)
        drawingPoly = false;

    currentLineMode = mode;
}

/**
 * @brief Canvas::SaveDrawCommand - Put together a DrawCommand
 *                                  and save it on the undo/redo stack.
 *
 */
void Canvas::saveDrawCommand(const QPixmap &old_image)
{
    // put the old and new image on the stack for undo/redo
    QUndoCommand *drawCommand = new DrawCommand(old_image, image);
    undoStack->push(drawCommand);
}

/**
 * @brief Canvas::createTools - takes care of creating the tools
 *
 */
void Canvas::createTools()
{
    // create the tools
    penTool = new PenTool(QBrush(Qt::black), DEFAULT_PEN_THICKNESS);
    lineTool = new LineTool(QBrush(Qt::black), DEFAULT_PEN_THICKNESS);
    eraserTool = new EraserTool(QBrush(Qt::white), DEFAULT_ERASER_THICKNESS);
    rectTool = new RectTool(QBrush(Qt::black), DEFAULT_PEN_THICKNESS);
    renderTool = new RenderTool();

    // set default tool
    currentTool = static_cast<Tool*>(penTool);
}

/**
 * @brief imagesEqual - returns true if the two images are the same
 *
 */
bool imagesEqual(const QPixmap &image1, const QPixmap &image2)
{
    return image1.toImage() == image2.toImage();
}
