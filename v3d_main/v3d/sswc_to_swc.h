#ifndef SSWC_TO_SWC_H
#define SSWC_TO_SWC_H

#include <fstream>
#include "../basic_c_fun/v3d_interface.h"

using namespace std;

class sswc_to_swc
{
public:
    sswc_to_swc(){}

public:
    static NeuronTree readSSWC_file(const QString& filename);


};

#endif // SSWC_TO_SWC_H
