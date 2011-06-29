#include "FragmentSelectionModel.h"

/* explicit */
FragmentSelectionModel::FragmentSelectionModel(QObject *parent /* = 0 */ )
        : QObject(parent)
        , currentHighlightedFragment(InvalidFragmentIndex)
    {}

void FragmentSelectionModel::setHighlightedFragment(FragmentIndex fi)
{
    if (currentHighlightedFragment == fi) return;
    currentHighlightedFragment = fi;
    emit fragmentHighlightChanged(fi);
}

void FragmentSelectionModel::setSelectedFragments(FragmentSet fs)
{
    if (currentSelectedFragments == fs) return;
    currentSelectedFragments = fs;
    emit fragmentSelectionChanged(fs);
}

void FragmentSelectionModel::appendSelectedFragments(FragmentSet fs)
{
    FragmentSet newSet = fs + currentSelectedFragments;
    if (newSet == currentSelectedFragments) return;
    currentSelectedFragments = newSet;
    emit fragmentSelectionChanged(newSet);
}

void FragmentSelectionModel::setVisibleFragments(FragmentSet fs)
{
    if (fs == currentVisibleFragments) return;
    currentVisibleFragments = fs;
    emit fragmentVisibilityChanged(fs);
}

void FragmentSelectionModel::appendVisibleFragment(FragmentSet fs)
{
    FragmentSet newSet = fs + currentVisibleFragments;
    if (newSet == currentVisibleFragments) return;
    currentVisibleFragments = newSet;
    emit fragmentVisibilityChanged(fs);
}



