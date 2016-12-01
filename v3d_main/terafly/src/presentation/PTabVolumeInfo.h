#ifndef P_TAB_VOLUME_INFO_H
#define P_TAB_VOLUME_INFO_H

#include "theader.h"

#include "../control/CPlugin.h"
#include "QGradientBar.h"

namespace terafly
{
    class PTabVolumeInfo;

    quint64 dir_size(const QString & str);
}

class terafly::PTabVolumeInfo : public QWidget
{
        Q_OBJECT

    private:


        QTabWidget *tabs;
        QWidget *info_tab;
        QWidget *pyramid_tab;

        // info panel
        QWidget* info_panel;
        QLineEdit* vol_format_field;
        QLineEdit* vol_size_field;
        QLineEdit* vol_dims_mm_field;
        QLineEdit* vol_dims_vxl_field;
        QLabel* voxel_dims_label;
        QLineEdit* vxl_field;
        QLineEdit* org_field;

        // virtual pyramid info panel
        QGroupBox* vp_panel;
        QLineEdit* vp_path;
        QPushButton* vp_open;
        QLineEdit* vp_size;
        QPushButton* vp_recheck;
        QLineEdit* vp_subsampling;
        QLineEdit* vp_tiledims;

        // virtual pyramid exploration panel
        QGroupBox* vp_exploration_panel;
        QLineEdit* vp_coverage_line;
        QPushButton* vp_prefetch_button;
        QSpinBox* vp_prefetch_blocks_spinbox;
        QSpinBox* vp_prefetch_blocks_dims;

        // virtual pyramid RAM panel
        QGroupBox* vp_ram_panel;
        QDoubleSpinBox* vp_max_ram_spinbox;
        //QPushButton* vp_ram_show_res_buttons;
        std::vector <QLabel*> vp_ram_labels;
        std::vector <QLabel*> vp_ram_used_labels;
        std::vector <QGradientBar*> vp_ram_bars;
        std::vector <QPushButton*> vp_ram_clear_buttons;
        static const size_t vp_ram_max_size = 10;

        QTimer updateTimer;


    public:

        PTabVolumeInfo(QWidget *parent);

    signals:

    public slots:

        void reset();
        void init();
        void update();

        void open_button_clicked();
        void recheck_button_clicked();
        void clear_button_clicked();
        void ram_limit_changed(double v);
        void fetch_button_clicked();
        //void show_ram_layers_toggled(bool checked);

};

#endif // PABOUT_H
