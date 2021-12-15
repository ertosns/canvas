#include <QPainter>
#include <vtkBorderWidget.h>
#include <vtkCommand.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>
#include <vtkVersion.h>

#include "tool.h"
#include "canvas.h"

//

#if VTK_VERSION_NUMBER >= 89000000000ULL
#define VTK890 1
#endif


void RenderTool::drawTo(const QPoint &endPoint, Canvas *canvas, QPixmap *image) {
    vtkWindowToImageFilter *w2if = vtkWindowToImageFilter::New();
    w2if->ReadFrontBufferOff();
    w2if->SetInput(renderWindow);
    w2if->SetInputBufferTypeToRGBA();
    w2if->Update();
    //TODO (fix) extract foreground only
    vtkImageData *img = w2if->GetOutput();
    int *dims = img->GetDimensions();
    int width=dims[0];
    int height=dims[1];
    QImage qimage(width, height,  QImage::Format_ARGB32);
    //image is mirrored vertically
    qimage=qimage.mirrored(false, true);
    //rgb channels are swapped
    qimage=qimage.rgbSwapped();
    /*
    QImage qimage(width, height, QImage::Format_RGBA64);
    QRgba64* rgbaPtr =
        reinterpret_cast<QRgba64*>(qimage.bits()) + width * (height - 1);
    unsigned char* colorsPtr =
        reinterpret_cast<unsigned char*>(img->GetScalarPointer());

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            //TODO (fix) some pixels coordinates are out of range!
            // Swap the vtkImageData RGBA values with an equivalent QColor
            *(rgbaPtr++) = QColor(colorsPtr[0], colorsPtr[1], colorsPtr[2], 255).rgba64();
            colorsPtr += img->GetNumberOfScalarComponents();

            qimage.setPixel(row, col, *rgbaPtr);
        }
        rgbaPtr -= width * 2;
    }
    */

    //QPixmap *alpha = new QPixmap;
    //alpha->fill(Qt::transparent);
    //
    QPixmap *overlay = new QPixmap;
    //overlay->setAlphaChannel(*alpha);
    //overlay->fill(Qt::transparent);
    *overlay = QPixmap::fromImage(qimage);

    QPixmap result(image->width(), image->height());
    //result.fill(Qt::transparent); // force alpha channel
    {
        QPainter painter(&result);
        //painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        //painter.setOpacity(0.5);
        painter.drawPixmap(0, 0, *image);

        QPoint startp = getStartPoint();
        int x1 = startp.x();
        int y1 = startp.y();
        int x2 = endPoint.x();
        int y2 = endPoint.y();
        int x = std::min(x1, x2);
        int y = std::min(y1, y2);
        int h = std::max(y1, y2) - y;
        int w = std::max(x1, x2) - x;
        painter.drawPixmap(x, y, h, w, *overlay);
    }
    *image=result;
    canvas->update();
}
/**
 * @brief PenTool::drawTo - Draws line from startPoint to endPoint, where
 *                          startpoint is either:
 *
 *                          -where mouse was clicked (the initial left click)
 *                          -where mouse was moved FROM (last event's endPoint)
 *
 *                          -endPoint is where the mouse was moved TO on this event.
 *
 */
void PenTool::drawTo(const QPoint &endPoint, Canvas *canvas, QPixmap *image)
{
    QPainter painter(image);
    painter.setPen(static_cast<QPen>(*this));
    painter.drawLine(getStartPoint(), endPoint);

    // speed things up a bit by only updating the immediate
    // radius of the Canvas
    int rad = (this->width() / 2) + 2;
    canvas->update(QRect(getStartPoint(), endPoint)
                     .normalized()
                     .adjusted(-rad, -rad, +rad, +rad));
    setStartPoint(endPoint);
}

/**
 * @brief LineTool::drawTo - Draws line from startPoint to endPoint, where:
 *                           -startpoint is where mouse was clicked, and
 *                           -endPoint is where the mouse was released
 *
 */
void LineTool::drawTo(const QPoint &endPoint,
                      Canvas *canvas,
                      QPixmap *image)
{
    QPainter painter(image);
    painter.setPen(static_cast<QPen>(*this));
    painter.drawLine(getStartPoint(), endPoint);
    canvas->update();
}

/**
 * @brief RectTool::RectTool - Constructor for a rectangle tool.
 *
 */
RectTool::RectTool(const QBrush &brush, qreal width,
                   Qt::PenStyle s,
                   Qt::PenCapStyle c,
                   Qt::PenJoinStyle j,
                   QColor fill,
                   ShapeType shape,
                   FillColor mode,
                   int curve)
    : Tool(brush, width, s, c, j)
{
    fillColor = fill;
    fillMode = mode;
    shapeType = shape;
    roundedCurve = curve;
}

/**
 * @brief RectTool::drawTo - Draws shape from startPoint to endPoint, where:
 *                           -startpoint is where mouse was clicked, and
 *                           -endPoint is where the mouse was released
 *
 */
void RectTool::drawTo(const QPoint &endPoint,
                      Canvas *canvas,
                      QPixmap *image)
{
    QPainter painter(image);
    painter.setPen(static_cast<QPen>(*this));
    QRect rect = adjustPoints(endPoint);

    //draw a rectangle, square, or ellipse--fill or no fill--based on settings
    switch(shapeType)
    {
        case rectangle:
        {
            if(fillColor != no_fill)
                painter.fillRect(rect, fillColor);
            painter.drawRect(rect);
            break;
        }
        case rounded_rectangle:
        {
            if(fillMode != no_fill)
                painter.setBrush(QBrush(fillColor));
            painter.drawRoundedRect(rect, roundedCurve,
                                    roundedCurve,
                                    Qt::RelativeSize);
            break;
        }
        case ellipse:
        {
            if(fillMode != no_fill)
                painter.setBrush(QBrush(fillColor));
            painter.drawEllipse(rect);
            break;
        }
        default:
          break;
    }
    canvas->update();
}

/**
 * @brief RectTool::adjustPoints - adjusts the points when constructing
 *                                 a rectangle
 *
 */
QRect RectTool::adjustPoints(const QPoint &endPoint)
{
    // 'top left' and 'bottom right' are relative, so we may need to
    // switch the points
    QRect rect;
    if(endPoint.x() < getStartPoint().x())
        rect = QRect(endPoint, getStartPoint());
    else
        rect = QRect(getStartPoint(), endPoint);
    return rect;
}
