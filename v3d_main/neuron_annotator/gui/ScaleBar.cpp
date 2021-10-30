#include "ScaleBar.h"

#include <QString>
#include <QColor>
#include <QFont>
#include <QList>

#include <cmath>

void ScaleBar::paint(float xVoxelSizeInMicrons, float screenPixelsPerImageVoxel, int windowWidth, int windowHeight, QPainter& painter)
{
    // 1 - set scale bar parameters
    const QColor bgColor = QColor(0, 0, 0, 128); // transparent black background
    const QColor fgColor = QColor(180, 180, 180, 255); // solid white foreground
    int borderWidth = 3;
    int barHeight = 5;
    int widgetWidth = 150;
    int labelHeight = 15;
    int labelSep = 0;
    QFont font = QFont("Times");
    font.setPixelSize(labelHeight);
    QList<float> roundNumbers = QList<float>() <<
            1 << 1.5 << 2 << 3 << 4 << 5 << 7.5; // Max difference 1.5X

    // 2 - compute scale
    // How many pixels per ten micrometers?
    QString units = QString::fromUtf8(" μm");
    float micronsPerVoxel = xVoxelSizeInMicrons;
    // qDebug() << "sampleScaleX = " << micronsPerVoxel;
    if (xVoxelSizeInMicrons == 0.0) { // zero means "unknown"
        units = " voxels";
        micronsPerVoxel = 1.0;
    }
    // getZoomScale() is in units of pixels per voxel
    float micronsPerPixel = micronsPerVoxel / screenPixelsPerImageVoxel;
    float maxBarMicrons = widgetWidth * micronsPerPixel;
    float barMicrons = 0;
    Q_FOREACH(const float& rounded, roundNumbers) {
        int exponent = (int) std::floor(std::log10(maxBarMicrons/rounded));
        float testBarMicrons = rounded * std::pow(10, (double)exponent);
        if (testBarMicrons > barMicrons) {
            barMicrons = testBarMicrons;
        }
    }
    // barMicrons = 1.0;
    int barPixels = (int)(barMicrons / micronsPerPixel);

    QString value;
    value.sprintf("%g",
                  barMicrons
                  // (float) barPixels
                  );
    QString label = value + units;

    // 3 - draw the scale bar
    // paint semi-transparent background square for contrast
    int x = windowWidth - widgetWidth - 2*borderWidth - 20;
    int w = widgetWidth + 2*borderWidth;
    int h = 3*borderWidth + barHeight + labelHeight + labelSep;
    int y = windowHeight - h - 20;
    painter.fillRect(x, y, w, h, bgColor);
    // paint the scale bar itself
    x += borderWidth;
    y += borderWidth;
    w = barPixels;
    h = barHeight;
    painter.fillRect(x, y, w, h, fgColor);
    // paint the value and units label
    y += labelHeight + barHeight + labelSep;
    painter.setPen(fgColor);
    painter.setFont(font);
    painter.drawText(x, y, label);
}
