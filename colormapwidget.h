#ifndef COLORMAPWIDGET_H
#define COLORMAPWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QVector3D>
#include <QLinearGradient>
#include <QColor>
#include <QString>
#include <QFontMetrics>

#include "vizdata.h"

class ColorMapWidget : public QWidget
{
    Q_OBJECT

public:
    ColorMapWidget(QWidget *parent = nullptr) : QWidget(parent) {
        vizData = new VizData();
        colorMapMinMax = QPair<double,double>(0, 1);
    }

    void setColorMapMinMax(QPair<double,double> minMax) {
        colorMapMinMax = minMax;
        update();
    }

private:
    VizData *vizData;
    QPair<double,double> colorMapMinMax;

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QPainter painter(this);

        // Create a horizontal linear gradient from left to right
        QLinearGradient gradient(0, 0, width(), 0);  // Gradient from x=0 (left) to x=width() (right)

        // Add color stops to the gradient (in original order)
        for (int i = 0; i < vizData->colorMap.size(); ++i) {
            QVector3D stop = vizData->colorMap[i];
            gradient.setColorAt(static_cast<double>(i) / (vizData->colorMap.size() - 1), QColor::fromRgbF(stop.x(), stop.y(), stop.z()));
        }

        // Draw the horizontal gradient on the widget
        int colorMapHeight = 30;  // Define the height of the colormap
        int colormapPaddingY = 20;  // Vertical padding between the colormap and the top of the widget
        painter.fillRect(rect().adjusted(20, colormapPaddingY, -20, -height() + colormapPaddingY + colorMapHeight), gradient);

        // Set up painter to draw text
        QPen pen;
        pen.setColor(Qt::GlobalColor::gray);
        pen.setWidth(4);
        painter.setPen(pen);
        QFont font = painter.font();
        font.setPointSize(14);
        painter.setFont(font);

        QFontMetrics metrics(painter.font());

        // Define values to display above the colormap
        QVector<QString> valueLabels = {
            QString::number(colorMapMinMax.first, 'f', 2),
            QString::number((colorMapMinMax.first + colorMapMinMax.second) / 4, 'f', 2),
            QString::number((colorMapMinMax.first + colorMapMinMax.second) / 2, 'f', 2),
            QString::number(((colorMapMinMax.first + colorMapMinMax.second) / 4)*3, 'f', 2),
            QString::number(colorMapMinMax.second, 'f', 2)
        };

        int textPaddingY = 0;  // Vertical padding between text and top of the widget
        int tickLength = 0;    // Length of the tick marks

        for (int i = 0; i < valueLabels.size(); ++i) {
            // Calculate the x-coordinate for each label
            double positionFactor = static_cast<double>(i) / (valueLabels.size() - 1);  // Value from 0 to 1
            int x = static_cast<int>(positionFactor * (width() - 40)) + 20;  // Adjust x based on colormap width and padding

            // Align text horizontally for first and last labels to avoid going outside
            int textX;
            if (i == 0) {
                textX = x;  // Align first label to the left of the colormap
            } else if (i == valueLabels.size() - 1) {
                textX = x - metrics.horizontalAdvance(valueLabels[i]);  // Align last label to the right of the colormap
            } else {
                textX = x - metrics.horizontalAdvance(valueLabels[i]) / 2;  // Center the middle label
            }

            int textY = textPaddingY + metrics.ascent();  // Position the text above the colormap

            // Draw the text above the colormap
            painter.drawText(textX, textY, valueLabels[i]);

            // Draw the tick mark below the text
            painter.drawLine(x, colormapPaddingY, x, colormapPaddingY - tickLength);
        }
    }
};

#endif // COLORMAPWIDGET_H
