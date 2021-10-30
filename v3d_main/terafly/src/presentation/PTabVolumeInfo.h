#ifndef P_TAB_VOLUME_INFO_H
#define P_TAB_VOLUME_INFO_H

#include "theader.h"

#include "../control/CPlugin.h"
#include "QGradientBar.h"

namespace terafly
{
    class PTabVolumeInfo;
    class CUserInactivityFilter;

    quint64 dir_size(const QString & str);
}

class terafly::CUserInactivityFilter : public QObject
{
    Q_OBJECT

    public:

        QElapsedTimer timer;

        CUserInactivityFilter(QObject* parent=0) : QObject(parent){timer.start();}

    protected:

        bool eventFilter(QObject *obj, QEvent *ev)
        {
            if(ev->type() == 6 || ev->type() == 5)
            {
                //printf("elapsed = %d\n", timer.elapsed());
                timer.restart();
            }

            return QObject::eventFilter(obj, ev);
        }
};

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
        QLineEdit* vol_path;

        // virtual pyramid info panel
        QGroupBox* vp_panel;
        QLineEdit* vp_path;
        QPushButton* vp_open;
        QLineEdit* vp_size;
        QPushButton* vp_recheck;
        QLineEdit* vp_subsampling;
        QLineEdit* vp_tiledims;
        QLineEdit* vp_tileformat;

        // virtual pyramid exploration panel
        QGroupBox* vp_exploration_panel;
        QCheckBox* vp_refill_auto_checkbox;
        QGradientBar* vp_exploration_bar_local;
        QGradientBar* vp_exploration_bar_global;
        QComboBox* vp_empty_viz_method_combobox;
        QSpinBox*   vp_empty_viz_intensity;
        QPushButton* vp_refill_button;
        QComboBox* vp_refill_strategy_combobox;
        QComboBox* vp_refill_stop_combobox;
        QSpinBox* vp_refill_times_spinbox;
        QSpinBox* vp_refill_coverage_spinbox;
        QSpinBox* vp_block_dimX;
        QSpinBox* vp_block_dimY;
        QSpinBox* vp_block_dimZ;
        QLineEdit* vp_refill_time_spent;
        QCheckBox* vp_highest_res_cache;
        QCheckBox* vp_highest_res_freeze;

        // virtual pyramid RAM panel
        QGroupBox* vp_ram_panel;
        QDoubleSpinBox* vp_max_ram_spinbox;
        //QPushButton* vp_ram_show_res_buttons;
        std::vector <QLabel*> vp_ram_labels;
        std::vector <QGradientBar*> vp_ram_bars;
        std::vector <QPushButton*> vp_ram_clear_buttons;
        static const size_t vp_ram_max_size = 8;

        QTimer updateTimer;
        QMutex refill_mutex;
        CUserInactivityFilter inactivityDetector;
        float refill_time_total;
        int refill_blocks_total;


    public:

        PTabVolumeInfo(QWidget *parent);

        void showVirtualPyramidTab(){tabs->setCurrentIndex(0);}

    signals:

    public slots:

        void reset();
        void init();
        void update();

        void open_button_clicked();
        void recheck_button_clicked();
        void clear_button_clicked();
        void ram_limit_changed(double v);
        void empty_combobox_index_changed(int v);
        void empty_intensity_value_changed(int v);
        void vp_refill_button_clicked(bool in_background = false);
        void vp_refill_strategy_combobox_changed(int v);
        void vp_refill_times_spinbox_changed(int v);
        void vp_refill_auto_checkbox_changed(bool v);
        void vp_refill_stop_combobox_changed(int v);
        void vp_refill_coverage_spinbox_changed(int v);
        void vp_highest_res_cache_checkbox_changed(bool v);
        void vp_highest_res_freeze_checkbox_changed(bool v);

};

#endif // PABOUT_H
