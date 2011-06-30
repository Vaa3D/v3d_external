#ifndef FRAGMENTSELECTIONMODEL_H
#define FRAGMENTSELECTIONMODEL_H

#include <QObject>
#include <QSet>

// FragmentSelectionModel manages information about whether neuron fragments are
// 1) highlighted or 2) selected or 3) visible.  These three concepts are, in
// principle, independent.
class FragmentSelectionModel : public QObject
{
    Q_OBJECT
public:
    typedef int FragmentIndex;
    typedef QSet<FragmentIndex> FragmentSet;
    static const FragmentIndex InvalidFragmentIndex = -1;

    explicit FragmentSelectionModel(QObject *parentObj = 0);

signals:
    // At most one fragment is highlighted at a time; usually when the users mouse hovers over a fragment
    void fragmentHighlightChanged(FragmentIndex);
    // Any number of fragments can be included in the current selection
    void fragmentSelectionChanged(const FragmentSet&);
    // Whether fragments should be visible is yet a third category
    void fragmentVisibilityChanged(const FragmentSet&);

public slots:
    void setHighlightedFragment(FragmentIndex);
    void setSelectedFragments(FragmentSet);
    void appendSelectedFragments(FragmentSet);
    void setVisibleFragments(FragmentSet);
    void appendVisibleFragment(FragmentSet);

protected:
    FragmentIndex currentHighlightedFragment;
    FragmentSet currentSelectedFragments;
    FragmentSet currentVisibleFragments;
};

#endif // FRAGMENTSELECTIONMODEL_H
