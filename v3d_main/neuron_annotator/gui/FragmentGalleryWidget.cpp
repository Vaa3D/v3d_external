#include "FragmentGalleryWidget.h"
#include <cassert>
#include <cmath> /* for std::abs(double) */

FragmentGalleryWidget::FragmentGalleryWidget(QWidget *pparent)
    : QAbstractScrollArea(pparent)
    , leftPixel(0)
{
    assert(viewport());
    assert(horizontalScrollBar());
    // Update view when user moves scroll bar
    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)),
            this, SLOT(setScrollPixel(int)));
    connect(this, SIGNAL(scrollPixelChanged(int)),
            horizontalScrollBar(), SLOT(setValue(int)));
}

/* virtual */
FragmentGalleryWidget::~FragmentGalleryWidget()
{
    clear();
}

/* virtual */
void FragmentGalleryWidget::resizeEvent(QResizeEvent * e) {
    updateThumbnailPositions();
    updateScrollBar();
}

// Scroll wheel to advance neurons, one half thumnail per tick
/* virtual */
void FragmentGalleryWidget::wheelEvent(QWheelEvent * e)
{
    double numDegrees = e->delta()/8.0;
    double numTicks = numDegrees/15.0;
    if (numTicks == 0) return;
    double numPixels = numTicks * buttonWidth * 0.5;
    // move at least one pixel
    if (std::abs(numPixels) < 1.0)
        numPixels = numPixels < 0 ? -1.0 : 1.0;
    setScrollPixel(leftPixel - numPixels);
}

// Pixel value at left edge of viewport
void FragmentGalleryWidget::setScrollPixel(int pixel)
{
    // check range of value
    int maxPixel = contents.size() * buttonWidth - viewport()->width();
    if (pixel > maxPixel)
        pixel = maxPixel;
    if (pixel < 0)
        pixel = 0;
    // check whether value has changed
    if (leftPixel == pixel)
        return;
    leftPixel = pixel;
    updateThumbnailPositions();
    emit scrollPixelChanged(pixel);
}

void FragmentGalleryWidget::clear() // delete all fragment widgets
{
    while ( ! contents.isEmpty() ) {
        GalleryButton * w = contents.takeLast();
        delete w;
    }
    updateScrollBar();
}

void FragmentGalleryWidget::appendFragment(GalleryButton * button)
{
    // Assume all buttons are the same size
    if (contents.isEmpty()) {
        buttonWidth = button->layout()->sizeHint().width();
        buttonHeight = button->layout()->sizeHint().width();
    }
    button->setGeometry(0, 0, buttonWidth, buttonHeight);
    button->setParent(viewport());
    contents.append(button);
    updateThumbnailPositions();
    updateScrollBar();
    // When a gallery button changes (e.g. gamma), repaint Gallery
    // connect(button, SIGNAL(widgetChanged(FragmentIndex)), this, SLOT(onThumbnailChanged(FragmentIndex)));
}

void FragmentGalleryWidget::updateScrollBar()
{
    int viewWidth = viewport()->width();
    horizontalScrollBar()->setPageStep(buttonWidth * 0.5); // scroll wheel ticks this much
    horizontalScrollBar()->setSingleStep(buttonWidth * 0.5);
    int barMax = contents.size() * buttonWidth - viewWidth;
    if (barMax < 0) barMax = 0;
    horizontalScrollBar()->setRange(0, barMax);
}

void FragmentGalleryWidget::scrollToFragment(FragmentIndex)
{
    //assert(false);
}

void FragmentGalleryWidget::updateThumbnailPositions()
{
    if (contents.size() < 1) return;

    for (int b = 0; b < contents.size(); ++b)
    {
        GalleryButton * button = contents[b];
        int px = b * (buttonWidth) - leftPixel;
        if (px < -buttonWidth) {
            button->hide();
            continue; // off screen to left
        }
        if (px > viewport()->size().width()) {
            button->hide();
            continue; // off screen to right
        }
        int py = 0;
        button->move(px, py);
        button->show();
    }
}

