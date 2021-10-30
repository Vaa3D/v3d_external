#include "FragmentGalleryWidget.h"
#include "../data_model/NeuronFragmentData.h"
#include "../DataFlowModel.h"
#include <cassert>
#include <cmath> /* for std::abs(double) */
#include <algorithm> /* for sort() */

FragmentGalleryWidget::FragmentGalleryWidget(QWidget *pparent)
    : QAbstractScrollArea(pparent)
    , leftPixel(0)
    , order(&indexOrder)
    , sortOrder(SORT_BY_INDEX)
    , neuronFragmentData(NULL)
    , buttonWidth(0)
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
        buttonHeight = button->layout()->sizeHint().height();
    }
    button->setGeometry(0, 0, buttonWidth, buttonHeight);
    button->setParent(viewport());
    contents.append(button);
    updateThumbnailPositions();
    updateScrollBar();
}

void FragmentGalleryWidget::updateButtonsGeometry()
{
    if (contents.isEmpty()) return;
    GalleryButton* firstButton = contents[0];
    buttonWidth = firstButton->layout()->sizeHint().width();
    buttonHeight = firstButton->layout()->sizeHint().height();
    for (int i = 0; i < contents.size(); ++i)
        contents[i]->resize(buttonWidth, buttonHeight);
    updateThumbnailPositions();
    updateScrollBar();
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

void FragmentGalleryWidget::scrollToFragment(NeuronIndex)
{
    //assert(false);
}

void FragmentGalleryWidget::updateThumbnailPositions()
{
    if (contents.size() < 1) return;

    for (int b = 0; b < contents.size(); ++b)
    {
        // default to unsorted button order
        GalleryButton * button = contents[b];
        // Apply sort order, if available
        if (order && order->size()) {
            // qDebug() << order->size() << contents.size() << __FILE__ << __LINE__;
            if (order->size() == contents.size()) {
                button = contents[(*order)[b]];
            }
        }
        int px = b * (buttonWidth) - leftPixel;
        if (px < -buttonWidth) {
            button->hide();
            continue; // off screen to left
        }
        if (px > viewport()->size().width()) {
            button->hide();
            continue; // off screen to right
        }
        // Center thumbnail vertically
        int py = (viewport()->size().height() - buttonHeight) / 2;
        button->move(px, py);
        button->show();
    }
}

/* slot */
void FragmentGalleryWidget::sortByIndex()
{
    if (sortOrder == SORT_BY_INDEX)
        return;
    sortOrder = SORT_BY_INDEX;
    order = &indexOrder;
    updateThumbnailPositions();
    // TODO
}

/* slot */
void FragmentGalleryWidget::sortByColor()
{
    if (sortOrder == SORT_BY_COLOR)
        return;
    sortOrder = SORT_BY_COLOR;
    order = &colorOrder;
    updateThumbnailPositions();
}

/* slot */
void FragmentGalleryWidget::sortBySize() {
    if (sortOrder == SORT_BY_SIZE)
        return;
    sortOrder = SORT_BY_SIZE;
    order = &sizeOrder;
    updateThumbnailPositions();
}

/* slot */
void FragmentGalleryWidget::sortByName() {
    if (sortOrder == SORT_BY_NAME)
        return;
    sortOrder = SORT_BY_NAME;
    order = &nameOrder;
    updateThumbnailPositions();
}

// Index is a class to help compute sort orders
template<class T>
struct IndexSorter
{
    IndexSorter(const std::vector<T>& valuesParam, bool bAscendingParam = false)
        : values(valuesParam)
        , bAscending(bAscendingParam)
    {}
    // comparison function sorts indices based on values in values table
    bool operator() (int i, int j) const
    {
        // qDebug() << i << j << values[i] << values[j] << (values[i] < values[j]);
        if (bAscending)
            return (values[i] < values[j]);
        // Cannot simply negate bAscending result, because >= is not a strict weak ordering
        else
            return (values[i] > values[j]);
    }
    // return ordered list of array indices
    std::vector<int> computeIndexOrder()
    {
        // allocate array
        std::vector<int> result(values.size(), 0);
        // initialize to ascending integer index order
        for (int i = 0; i < values.size(); ++i)
            result[i] = i;
        // sort based on values table
        std::sort(result.begin(), result.end(), *this);
        // for (int i = 0; i < values.size(); ++i)
        //     qDebug() << i << result[i] << __FILE__ << __LINE__;
        return result;
    }

    bool bAscending;
    const std::vector<T>& values;
};


std::vector<QString> FragmentGalleryWidget::getButtonNames() {
    std::vector<QString> result;
    for (int i = 0; i < contents.size(); ++i) {
        result.push_back(contents[i]->getLabelText());
    }
    return result;
}

void FragmentGalleryWidget::updateNameSortTable() {
    // NAMES - from button labels
    std::vector<QString> names = getButtonNames();
    IndexSorter<QString> nameSorter(names, true);
    nameOrder = nameSorter.computeIndexOrder();
    repaint();
}

void FragmentGalleryWidget::updateSortTables()
{
    // qDebug() << "FragmentGalleryWidget::updateSortTables()";
    // NAMES - from button labels
    updateNameSortTable();
    if (!neuronFragmentData) return;
    {
        NeuronFragmentData::Reader fragmentReader(*neuronFragmentData);
        if (neuronFragmentData->readerIsStale(fragmentReader)) return;
        int sf = fragmentReader.getNumberOfFragments();
        // INDEX
        // Sort by index is easy.  We don't need to look anything up.
        if (indexOrder.size() != sf) indexOrder.assign(sf, 0);
        for (int f = 0; f < sf; ++f)
            indexOrder[f] = f;
        // SIZE (number of voxels)
        IndexSorter<int> sizeSorter(fragmentReader.getFragmentSizes(), false);
        sizeOrder = sizeSorter.computeIndexOrder();
        // COLOR (hue)
        IndexSorter<float> hueSorter(fragmentReader.getFragmentHues());
        colorOrder = hueSorter.computeIndexOrder();
    }
    repaint();
}

void FragmentGalleryWidget::setDataFlowModel(const DataFlowModel* dataFlowModelParam)
{
    if (NULL == dataFlowModelParam)
    {
        neuronFragmentData = NULL;
        return;
    }
    neuronFragmentData = &dataFlowModelParam->getNeuronFragmentData();
    connect(neuronFragmentData, SIGNAL(dataChanged()),
            this, SLOT(updateSortTables()));
}


