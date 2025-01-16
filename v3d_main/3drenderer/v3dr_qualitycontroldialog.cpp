#include "v3dr_qualitycontroldialog.h"
#include <map>

const vector<QString> V3dr_qualitycontrolDialog::quality_control_types = {
    "Multifurcation",
    "Approaching bifurcation",
    "Loop",
    "Missing",
    "Crossing error",
    "Color mutation",
    "Isolated branch",
    "Angle error"
};

V3dr_qualitycontrolDialog::V3dr_qualitycontrolDialog(V3dR_GLWidget* widget, QWidget *parent)
    : QWidget{parent}
{
    glwidget = widget;
    init();
    createLayout();

    this->setWindowFlags(this->windowFlags() | Qt::WindowMinimizeButtonHint | Qt::WindowStaysOnTopHint);
    // 获取主屏幕的大小
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();
    this->resize(0.60 * screenHeight + 130, 700);
}

void V3dr_qualitycontrolDialog::init(){
    int maxresindex = terafly::CImport::instance()->getResolutions()-1;
    IconImageManager::VirtualVolume* vol = terafly::CImport::instance()->getVolume(maxresindex);
    ImageMaxRes = XYZ(vol->getDIM_H(),vol->getDIM_V(),vol->getDIM_D());
    title = "Quality Control Manager";
    if(glwidget){
        renderer = (Renderer_gl1*)glwidget->getRenderer();
    }
}

void V3dr_qualitycontrolDialog::createLayout(){
    table = createTableMarker();
    if(table)
    {
        connect(table, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(doubleClickHandler(int,int)));
        for(int row=0; row<table->rowCount(); row++){
            for(int col=0; col<table->columnCount(); col++){
                QTableWidgetItem* item = table->item(row, col);
                if(item){
                    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                }
            }
        }
    }
    createSatistians();
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    QGroupBox* leftGroup = new QGroupBox();
    leftGroup->setTitle("QC Marker Infos");
    leftLayout = new QVBoxLayout(leftGroup);
    QGroupBox* rightGroup = new QGroupBox();
    rightGroup->setTitle("QC Statisticians");
    QVBoxLayout* rightLayout = new QVBoxLayout(rightGroup);

    QFont font("Microsoft YaHei");
    //old:11 new:28
    font.setPointSize(11);
    leftGroup->setFont(font);
    rightGroup->setFont(font);

    if(table){
        for(int row = 0; row<table->rowCount(); row++){
            table->item(row, 1)->setTextAlignment(Qt::AlignCenter);
        }
    }

    if(table)
        leftLayout->addWidget(table);
    rightLayout->addWidget(sumCountLabel);
    rightLayout->addWidget(multifurCountLabel);
    //    rightLayout->addWidget(approchingBifurCountLabel);
    rightLayout->addWidget(loopCountLabel);
    rightLayout->addWidget(missingCountLabel);
    rightLayout->addWidget(crossingCountLabel);
    //    rightLayout->addWidget(colorMutationCountLabel);
    rightLayout->addWidget(dissocaitiveSegCountLabel);
    rightLayout->addWidget(angleCountLabel);
    rightLayout->addWidget(overlapSegsCountLabel);
    rightLayout->addWidget(errorSegsCountLabel);

    mainLayout->addWidget(leftGroup);
    mainLayout->addWidget(rightGroup);
    setLayout(mainLayout);

    setWindowTitle(title);
}

QTableWidget* V3dr_qualitycontrolDialog::createTableMarker(){
    Renderer_gl1* r = renderer;
    if (! r)  return 0;

    QStringList qsl;
    qsl << "color" << "type" << "state";
    auto markers=terafly::PluginInterface::getLandmarkDirectly();
    type2IndexsMap.clear();
    row2IndexInUnCheckedMarkers.clear();
    for(auto it = markers.begin(); it != markers.end(); it++){
        if(std::find(quality_control_types.begin(), quality_control_types.end(), QString::fromStdString(it->comments)) != quality_control_types.end()){
            type2IndexsMap[it->comments].push_back(it-markers.begin());
        }
    }

    int row1Count = 0;
    int row2Count = glwidget->TeraflyCommunicator->checkedQcMarkers.size();
    for(auto it = markers.begin(); it != markers.end(); it++){
        if(std::find(quality_control_types.begin(), quality_control_types.end(), QString::fromStdString(it->comments)) != quality_control_types.end()){
            row1Count++;
        }
    }

    if(glwidget){
        vector<XYZ> results = glwidget->getImageCurResAndStartPoint();
        if(results.size()==2){
            ImageCurRes = results[0];
            ImageStartPoint = results[1];
            int row = row1Count + row2Count;
            int col = qsl.size();

            QTableWidget* t = new QTableWidget(row, col, this);
            QFont font("Microsoft YaHei");
            //old:10 new:26
            font.setPointSize(10);
            t->setHorizontalHeaderLabels(qsl);
            t->horizontalHeader()->setFont(font);

            int r=0;
            for (auto it=type2IndexsMap.begin(); it!=type2IndexsMap.end(); it++)
            {
                for(auto it_vec=it->second.begin(); it_vec!=it->second.end(); it_vec++){
                    int j=0;
                    QTableWidgetItem *curItem;
                    curItem = new QTableWidgetItem(QVariant::Color);
                    t->setItem(r, j++, curItem);
                    curItem->setData(0, VCOLOR(markers[*it_vec].color));
                    curItem->setData(Qt::DecorationRole, VCOLOR(markers[*it_vec].color));

                    curItem = new QTableWidgetItem(QString::fromStdString(markers[*it_vec].comments));
                    t->setItem(r, j++, curItem);

                    curItem = new QTableWidgetItem("unchecked");
                    t->setItem(r, j++, curItem);
                    MESSAGE_ASSERT(j==col);
                    row2IndexInUnCheckedMarkers[r] = *it_vec;
                    r++;
                }
            }

            for(int i=glwidget->TeraflyCommunicator->checkedQcMarkers.size()-1; i>=0; i--){
                int j=0;
                QTableWidgetItem *curItem;

                curItem = new QTableWidgetItem(QVariant::Color);
                t->setItem(r, j++, curItem);
                curItem->setData(0, VCOLOR(glwidget->TeraflyCommunicator->checkedQcMarkers[i].color));
                curItem->setData(Qt::DecorationRole, VCOLOR(glwidget->TeraflyCommunicator->checkedQcMarkers[i].color));

                curItem = new QTableWidgetItem(glwidget->TeraflyCommunicator->checkedQcMarkers[i].comment);
                t->setItem(r, j++, curItem);

                curItem = new QTableWidgetItem("checked");
                t->setItem(r, j++, curItem);

                MESSAGE_ASSERT(j==col);

                for(int p=0; p<col; p++){
                    QTableWidgetItem* item = t->item(r, p);
                    if(item)
                        item->setBackground(Qt::gray);
                }
                r++;
            }
            t->setFont(font);
            t->resizeColumnsToContents();
            t->resizeRowsToContents();
            if(row >= 1){
                QTableWidgetItem *curItem = t->item(0, 1);
                QString preVal = curItem->text();
                curItem->setText("Approaching bifurcation");
                t->resizeColumnToContents(1);
                curItem->setText(preVal);
            }
            return t;
        }
    }
    return 0;
}

void V3dr_qualitycontrolDialog::createSatistians(){
    auto uncheckedMarkers = terafly::PluginInterface::getLandmarkDirectly();
    int allCount = 0;
    for(auto it = type2IndexsMap.begin(); it!=type2IndexsMap.end(); it++){
        allCount += it->second.size();
    }

    sumCountLabel = new QLabel(this);
    multifurCountLabel = new QLabel(this);
    approchingBifurCountLabel = new QLabel(this);
    loopCountLabel = new QLabel(this);
    missingCountLabel = new QLabel(this);
    crossingCountLabel = new QLabel(this);
    colorMutationCountLabel = new QLabel(this);
    dissocaitiveSegCountLabel = new QLabel(this);
    angleCountLabel = new QLabel(this);
    overlapSegsCountLabel = new QLabel(this);
    errorSegsCountLabel = new QLabel(this);

    sumCountLabel->setText(QString("Total number of markers:\t\t  %1 ").arg(allCount));
    multifurCountLabel->setText(QString("Multifurcation:\t\t\t  %1 ").arg(type2IndexsMap["Multifurcation"].size()));
    //    approchingBifurCountLabel->setText(QString("Approaching bifurcation:  %1 ").arg(type2IndexsMap["Approaching bifurcation"].size()));
    loopCountLabel->setText(QString("Loop:\t\t\t\t  %1 ").arg(type2IndexsMap["Loop"].size()));
    missingCountLabel->setText(QString("Missing:\t\t\t\t  %1 ").arg(type2IndexsMap["Missing"].size()));
    crossingCountLabel->setText(QString("Crossing direction error:\t\t  %1 ").arg(type2IndexsMap["Crossing error"].size()));
    //    colorMutationCountLabel->setText(QString("Color mutation:\t\t  %1 ").arg(type2IndexsMap["Color mutation"].size()));
    dissocaitiveSegCountLabel->setText(QString("Isolated branch:\t\t\t  %1 ").arg(type2IndexsMap["Dissociative seg"].size()));
    angleCountLabel->setText(QString("Angle error:\t\t\t  %1 ").arg(type2IndexsMap["Angle error"].size()));
    overlapSegsCountLabel->setText(QString("Overlapping branches(removed):\t  %1 ").arg(glwidget->TeraflyCommunicator->removedOverlapSegNum));
    errorSegsCountLabel->setText(QString("Error seg(tuned):\t\t  %1 ").arg(glwidget->TeraflyCommunicator->removedErrSegNum));

    QFont font("Microsoft YaHei");
    font.setPointSize(11);
    sumCountLabel->setFont(font);
    multifurCountLabel->setFont(font);
    approchingBifurCountLabel->setFont(font);
    loopCountLabel->setFont(font);
    missingCountLabel->setFont(font);
    crossingCountLabel->setFont(font);
    colorMutationCountLabel->setFont(font);
    dissocaitiveSegCountLabel->setFont(font);
    angleCountLabel->setFont(font);
    overlapSegsCountLabel->setFont(font);
    errorSegsCountLabel->setFont(font);
}

XYZ V3dr_qualitycontrolDialog::ConvertGlobaltoLocalBlockCroods(double x,double y,double z)
{
    auto node=ConvertMaxRes2CurrResCoords(x,y,z);
    node.x-=(ImageStartPoint.x-1);
    node.y-=(ImageStartPoint.y-1);
    node.z-=(ImageStartPoint.z-1);
    return node;
}

XYZ V3dr_qualitycontrolDialog::ConvertLocalBlocktoGlobalCroods(double x,double y,double z)
{
    x+=(ImageStartPoint.x-1);
    y+=(ImageStartPoint.y-1);
    z+=(ImageStartPoint.z-1);

    XYZ node=ConvertCurrRes2MaxResCoords(x,y,z);
    return node;
}

XYZ V3dr_qualitycontrolDialog::ConvertMaxRes2CurrResCoords(double x,double y,double z)
{
    x/=(ImageMaxRes.x/ImageCurRes.x);
    y/=(ImageMaxRes.y/ImageCurRes.y);
    z/=(ImageMaxRes.z/ImageCurRes.z);
    return XYZ(x,y,z);
}

XYZ V3dr_qualitycontrolDialog::ConvertCurrRes2MaxResCoords(double x,double y,double z)
{
    //    qDebug()<<ImageMaxRes.x/ImageCurRes.x;
    x*=(ImageMaxRes.x/ImageCurRes.x);
    y*=(ImageMaxRes.y/ImageCurRes.y);
    z*=(ImageMaxRes.z/ImageCurRes.z);
    return XYZ(x,y,z);
}

void V3dr_qualitycontrolDialog::doubleClickHandler(int row, int col){
    QTableWidgetItem* item = table->item(row, col);
    if(item && item->background() != Qt::gray){
        if (! renderer)  return;

        vector<XYZ> results = glwidget->getImageCurResAndStartPoint();
        if(results.size()==2){
            ImageCurRes = results[0];
            ImageStartPoint = results[1];
        }
        else
            return;

        LandmarkList markers=terafly::PluginInterface::getLandmarkDirectly();
        int tmpind = row2IndexInUnCheckedMarkers[row];
        if (tmpind>=0)
        {
            LocationSimple mk = markers.at(tmpind); //get the specified landmark

            v3d_msg("Invoke terafly local-zoomin based on an existing marker.", 0);
            vector <XYZ> loc_vec;
            XYZ loc = ConvertGlobaltoLocalBlockCroods(mk.x, mk.y, mk.z);
            //            loc.x = mk.x; loc.y = mk.y; loc.z = mk.z;
            loc_vec.push_back(loc);

            renderer->b_grabhighrez = true;
            renderer->produceZoomViewOf3DRoi(loc_vec,
                                             0  //one means from non-wheel event
                                             );
            //            terafly::CViewer *cur_win = terafly::CViewer::getCurrent();
            //            glwidget = cur_win->getGLWidget();
            //            renderer = (Renderer_gl1*)glwidget->getRenderer();
        }
    }
}

void V3dr_qualitycontrolDialog::updateInfo(){
    if(table){
        leftLayout->removeWidget(table);
        table->deleteLater();
        table = 0;
    }
    table = createTableMarker();
    if(table){
        connect(table, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(doubleClickHandler(int,int)));
        for(int row=0; row<table->rowCount(); row++){
            for(int col=0; col<table->columnCount(); col++){
                QTableWidgetItem* item = table->item(row, col);
                if(item){
                    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                }
            }
        }
        leftLayout->addWidget(table);
    }
}

void V3dr_qualitycontrolDialog::updateMarkersCounts(){
    auto uncheckedMarkers = terafly::PluginInterface::getLandmarkDirectly();
    int allCount = 0;
    for(auto it = type2IndexsMap.begin(); it!=type2IndexsMap.end(); it++){
        allCount += it->second.size();
    }

    sumCountLabel->setText(QString("Total number of markers:\t\t  %1 ").arg(allCount));
    multifurCountLabel->setText(QString("Multifurcation:\t\t\t  %1 ").arg(type2IndexsMap["Multifurcation"].size()));
    //    approchingBifurCountLabel->setText(QString("Approaching bifurcation:  %1 ").arg(type2IndexsMap["Approaching bifurcation"].size()));
    loopCountLabel->setText(QString("Loop:\t\t\t\t  %1 ").arg(type2IndexsMap["Loop"].size()));
    missingCountLabel->setText(QString("Missing:\t\t\t\t  %1 ").arg(type2IndexsMap["Missing"].size()));
    crossingCountLabel->setText(QString("Crossing direction error:\t\t  %1 ").arg(type2IndexsMap["Crossing error"].size()));
    //    colorMutationCountLabel->setText(QString("Color mutation:\t\t  %1 ").arg(type2IndexsMap["Color mutation"].size()));
    dissocaitiveSegCountLabel->setText(QString("Isolated branch:\t\t\t  %1 ").arg(type2IndexsMap["Dissociative seg"].size()));
    angleCountLabel->setText(QString("Angle error:\t\t\t  %1 ").arg(type2IndexsMap["Angle error"].size()));
}

void V3dr_qualitycontrolDialog::updateSegsCounts(){
    overlapSegsCountLabel->setText(QString("Overlapping branches(removed):\t  %1 ").arg(glwidget->TeraflyCommunicator->removedOverlapSegNum));
    errorSegsCountLabel->setText(QString("Error seg(tuned):\t\t  %1 ").arg(glwidget->TeraflyCommunicator->removedErrSegNum));
}
