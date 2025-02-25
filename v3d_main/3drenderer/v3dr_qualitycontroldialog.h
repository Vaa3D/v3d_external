
#ifndef V3DR_QUALITYCONTROLDIALOG_H
#define V3DR_QUALITYCONTROLDIALOG_H

#include "v3dr_common.h"
#include "renderer_gl1.h"
#include "v3dr_glwidget.h"
#include "../terafly/src/control/CImport.h"
#include "../terafly/src/control/CViewer.h"
#include <QWidget>


class V3dr_qualitycontrolDialog : public QWidget
{
    Q_OBJECT
public:
    const static vector<QString> quality_control_types;

    explicit V3dr_qualitycontrolDialog(V3dR_GLWidget* widget, QWidget *parent = nullptr);
    XYZ ImageMaxRes;//
    XYZ ImageCurRes;
    XYZ ImageStartPoint;
    map<string, vector<int>> type2IndexsMap;
    map<int, int> row2IndexInUnCheckedMarkers;

    V3dR_GLWidget* glwidget;
    Renderer_gl1* renderer;

    //Coordinate transform
    XYZ ConvertGlobaltoLocalBlockCroods(double x,double y,double z);
    XYZ ConvertLocalBlocktoGlobalCroods(double x,double y,double z);
    XYZ ConvertMaxRes2CurrResCoords(double x,double y,double z);
    XYZ ConvertCurrRes2MaxResCoords(double x,double y,double z);

private:
    QLabel* sumCountLabel;
    QLabel* multifurCountLabel;
    QLabel* approchingBifurCountLabel;
    QLabel* loopCountLabel;
    QLabel* missingCountLabel;
    QLabel* crossingCountLabel;
    QLabel* colorMutationCountLabel;
    QLabel* dissocaitiveSegCountLabel;
    QLabel* angleCountLabel;
    QLabel* overlapSegsCountLabel;
    QLabel* errorSegsCountLabel;
    QTableWidget* table;
    QVBoxLayout* leftLayout;
    QString title;

    void init();
    void createLayout();
    QTableWidget* createTableMarker();
    void createSatistians();

signals:

public slots:
    void doubleClickHandler(int row, int col);
    void updateMarkersCounts();
    void updateSegsCounts();
    void updateInfo();

};

#endif // V3DR_QUALITYCONTROLDIALOG_H
