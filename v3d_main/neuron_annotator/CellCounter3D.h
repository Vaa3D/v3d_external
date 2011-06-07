#ifndef CELLCOUNTER3D_H
#define CELLCOUNTER3D_H

#include <vector>
#include "../v3d/v3d_core.h"

#if defined (_MSC_VER)
#include "../basic_c_fun/vcdiff.h"
#else
#endif

class CellCounter3D
{
public:
    CellCounter3D();
    ~CellCounter3D();
    void loadMy4DImage(My4DImage* image);
    void loadInputFile();
    bool findCells();
    void markImage();
    void setInputFilePath(QString filePath) { this->inputFilePath=filePath; }
    void writeOutputImageFile();
    void writeOutputReportFile();
    int processArgs(vector<char*> *argList);

    static string getCommandLineDescription() {
        return "cell-counter";
    }

    static int ARG_STATUS_USAGE;
    static int ARG_STATUS_OK;
    static int ARG_STATUS_HELP;

    static string getHelp() {
        string help;
        help.append("\n");
        help.append("CellCounter3D by Sean Murphy for HHMI JFRC\n");
        help.append("\n");
        help.append("Help Page\n");
        help.append("\n");
        help.append("This tool takes as input an .lsm file (a specialized 3D .tif file) containing an image\n");
        help.append("stack with 2 channels of data. The first channel (0) is expected to be a reference channel\n");
        help.append("(background). The second channel (1) is expected to be cell marker signal\n");
        help.append("\n");
        help.append("The tool will attempt to locate the position of cells in the 3D volume. If successful, two\n");
        help.append("output files will be generated in the same location as the input file:\n");
        help.append("   * An RGB .tif file containing the 3D data which has been marked with identified cells\n");
        help.append("   * A report file containing a tab-separated list of each found cell with its 3D coordinates\n");
        help.append("\n");
        help.append("For the tool to be useful, its parameters must be manually tuned for each experiemntal protocol.\n");
        help.append("Once successful parameters are found, these can be re-used for all similar experimental data.\n");
        help.append("The main reason for generating the .tif output is to aid with this tuning process, in addition to\n");
        help.append("providing a means for human visual validation of results.\n");
        help.append("\n");
        help.append("In a high-throughput production environment, the primary useful output will be the report file,\n");
        help.append("which in addition to counting the cells provides coordinates which further downstream image processing\n");
        help.append("tools can use to analyze the 3D cell data.\n");
        help.append("\n");
        help.append("What follows is a summary of how the tool works and how the various parameters are used.\n");
        help.append("\n");
        help.append("(1) Both the reference channel \"background\" and the cell channel \"signal\" are thresholded. Voxels\n");
        help.append("    with intensity levels below threshold are set to zero. The thresholds are controlled by:\n");
        help.append("          -ist <initial signal threshold for cells>\n");
        help.append("          -ibt <initial background threshold for reference>\n");
        help.append("\n");
        help.append("(2) Both input channels are consolidated into a single working channel, by merging the channels such that\n");
        help.append("    voxels in which the signal is greater than background are set to signal value, whereas in the opposite\n");
        help.append("    case voxels are set to zero intensity\n");
        help.append("\n");
        help.append("(3) Signal is normalized in the working channel as follows. The standard deviation of the non-zero values\n");
        help.append("    is calculated. This parameter:\n");
        help.append("           -sn <sigma normalization>\n");
        help.append("    is used to create a range about the average non-zero intensity, such that values within the range are\n");
        help.append("    normalized from 0-255 and values above or below the range are set to 0 or 255, respectively. The following\n");
        help.append("    parameter is then used as a threshold, below which the voxels are set to zero:\n");
        help.append("           -nt <normalization threshold>\n");
        help.append("\n");
        help.append("(4) In this step, a series of erosion cycles are performed, such that voxels not having enough non-zero neighbors\n");
        help.append("    are themselves set to zero. In this step and the following dialation step, the working channel is converted to\n");
        help.append("    binary (either non-zero or zero). These three parameters control the erosion process:\n");
        help.append("           -ec <number of erosion cycles>\n");
        help.append("           -ees <the size of the cube surrounding the voxel within which neighbors are counted, 2 = 4x4 = 16 voxels>\n");
        help.append("           -et <number of neighbors required to be non-zero within cube to survive the erosion cycle>\n");
        help.append("\n");
        help.append("(5) The next step is similar to the last, except dialtion is performed rather than erosion. Voxels which are zero\n");
        help.append("    but which have above a certain number of non-zero neighbors are themselves converted to non-zero. These three\n");
        help.append("    parameters control the dialation process:\n");
        help.append("           -dc <number of dialation cycles>\n");
        help.append("           -des <the size of the cube within which to count neighbors, 2 = 4x4 = 16 voxels, for example>\n");
        help.append("           -dt <number of non-zero ne ighbors required to be converted from zero to non-zero>\n");
        help.append("\n");
        help.append("(6) After erosion and dialation, a center-surround filter is applied to more crisply define spherical objects of\n");
        help.append("    a size implied by parameters, which are:\n");
        help.append("           -cr <the radius, in voxels, of the \'center\' component of the filter>\n");
        help.append("           -cv <the value for each position in the center filter component>\n");
        help.append("           -sr <the radius of the \'surround\' component of the filter, such that total filter radius=center+surround radii>\n");
        help.append("           -sv <the value for each position of the surround filter component>\n");
        help.append("\n");
        help.append("(7) After center-surround filter is applied, the initial output is auto-scaled with min=0 and max=255. The next step is\n");
        help.append("    to threshold this scaled output. The following parameter provides the threshold - NOTE this parameter is critical in\n");
        help.append("    the fine-tuning required to capture all cells. The center-surround filter is implemented as a search beginning at a\n");
        help.append("    threshold which may fail. An increment is provided, such that the search progresses until it either finds a value which\n");
        help.append("    succeeds, or it finally produces an error - implying the threshold search should be increased in value range\n");
        help.append("           -cst <center-surround threshold search start>\n");
        help.append("           -csi <center-surround search increment>\n");
        help.append("           -csm <center-surround max value before error>\n");
        help.append("\n");
        help.append("(8) We now expect that each cell is represented by an isolated compact region, enabling a simple connected-neighbors\n");
        help.append("    approach for identifying the individual cells. We conduct a connected-neighbors search governed by these parameters:\n");
        help.append("           -mnr <minimum number of voxels in region required for consideration as cell>\n");
        help.append("           -mxr <maximum number of voxels permitted in region before we declare an error - implies connected regions>\n");
        help.append("\n");
        help.append("    NOTE: it is typical to encounter the too-many-voxels error when tuning parameters - the \'-cst\' parameter can help\n");
        help.append("    this problem. By increasing the -cst threshold, the regions should shrink.\n");
        help.append("\n");
        help.append("(9) Cells are marked using a marker with an inner and outer component. The inner component is a small cube, and the outer\n");
        help.append("    component is a larger surrounding sphere. The user can customize the presentation of the graphical results with these\n");
        help.append("    parameters. The voxels not within the marking spheres but with non-zero cell signal are also colorizable.\n");
        help.append("           -ms <size of the inner cube designating the x,y,z coordinate of the cell - always white>\n");
        help.append("           -mc <color of the surrounding sphere - in terms of a product with the original intensity magnitude>\n");
        help.append("           -sc <color of the extra-marker cell signal volume - in terms of a product with the original intensity magnitude>\n");
        return help;
    }

    static string getUsage() {
        string usage;
        usage.append("  CellCounter3D by Sean Murphy for HHMI JFRC                              \n");
        usage.append(" [ -h   This will print the help page for the cell counter               ]\n");
        usage.append("   -i   <input lsm filepath>\n");
        usage.append(" [ -ist <initial signal threshold,       int     0-255  default=10>      ]\n");
        usage.append(" [ -ibt <initial background threshold,   int     0-255  default=10>      ]\n");
        usage.append(" [ -sn  <sigma normalization,            double  >0.0   default=2.0>     ]\n");
        usage.append(" [ -nt  <normalization threshold,        int     0-255  default=20>      ]\n");
        usage.append(" [ -ec  <erosion cycles,                 int     >0     default=3>       ]\n");
        usage.append(" [ -ees <erosion element size,           int     >1     default=2>       ]\n");
        usage.append(" [ -et  <erosion threshold,              int     0-255  default=44>      ]\n");
        usage.append(" [ -dc  <dialation cycles,               int     >0     default=3>       ]\n");
        usage.append(" [ -des <dialation element size,         int     >1     default=2>       ]\n");
        usage.append(" [ -dt  <dialation threshold,            int     0-255  default=44>      ]\n");
        usage.append(" [ -cr  <center-surround center radius   double  >0.0   default=11.0>    ]\n");
        usage.append(" [ -cv  <center-surround center value    double  any    default=1.0>     ]\n");
        usage.append(" [ -sr  <center-surround surround radius double  >0.0   default=2.0>     ]\n");
        usage.append(" [ -sv  <center-surround surround value  double  any    default=-2.0>    ]\n");
        usage.append(" [ -cst <center-surround threshold start int     0-255  default=70>      ]\n");
        usage.append(" [ -csi <center-surround increment       int     >0     default=5>       ]\n");
        usage.append(" [ -csm <center surround threshold max   int     >0     default=180>     ]\n");
        usage.append(" [ -ms  <mark-size                       int     >0     default=2>       ]\n");
        usage.append(" [ -mr  <mark-radius                     int     >0     default=10>      ]\n");
        usage.append(" [ -mc  <mark color                  int int int >0     default= 1000 0 1000> ]\n");
        usage.append(" [ -sc  <signal color                int int int >0     default= 255 255 255> ]\n");
        usage.append(" [ -mnr <minimum region voxels           int     >=0    default=100>     ]\n");
        usage.append(" [ -mxr <maximum region voxels           int     >=0    default=40000>   ]\n");
        return usage;
    }

    static void convertMy4DImage2channelToRGB(My4DImage & sourceImage, My4DImage & targetImage) {
        int xDim=sourceImage.getXDim();
        int yDim=sourceImage.getYDim();
        int zDim=sourceImage.getZDim();
        targetImage.loadImage(xDim, yDim, zDim, 3 /* 3-channels of data */, 1 /* 8-bits per channel */ );
        unsigned char**** sourceData = (unsigned char****)sourceImage.getData();
        unsigned char**** targetData = (unsigned char****)targetImage.getData();
        for (int z=0;z<zDim;z++) {
            for (int y=0;y<yDim;y++) {
                for (int x=0;x<xDim;x++) {
                    targetData[0][z][y][x]=sourceData[1][z][y][x]; // red
                    targetData[1][z][y][x]=sourceData[0][z][y][x]; // green
                    targetData[2][z][y][x]=0; // blue
                }
            }
        }
    }


protected:

    void dialateOrErode(int type, unsigned char*** s, unsigned char*** t, int elementSize, int neighborsForThreshold);
    void dialateOrErodeZslice(int type, int z, int elementSize, int neighborsForThreshold);
    void copyToImage(int sourceChannel, unsigned char**** d, unsigned char**** data, bool nonZeroOnly, bool markAllChannels);
    void addCrossMark(unsigned char**** d, int z, int y, int x, int size);

    long countNonZero(unsigned char*** d);
    void clearChannel(int channel, unsigned char**** d);

    void gammaFilter(unsigned char*** d, double gamma);
    void normalizeNonZero(unsigned char*** d, double sigma);

    void centerSurroundFilter(unsigned char*** source, unsigned char*** output, double centerSize, double centerValue, double surroundSize, double surroundValue);
    void centerSurroundFilterZSlice(int z);
    void applyBinaryThreshold(unsigned char*** source, unsigned char*** target, unsigned char threshold);
    bool findConnectedRegions(unsigned char*** d);
    void findNeighbors(int x, int y, int z, unsigned char*** d, unsigned char*** mask, QList<int> & neighborList);


    My4DImage* image;
    unsigned char***** workingData;
    QString inputFilePath;

    // Command-Line parameters /////////////////////////
    int INITIAL_SIGNAL_THRESHOLD;
    int INITIAL_BACKGROUND_THRESHOLD;
    double SIGMA_NORMALIZATION;
    int NORMALIZATION_THRESHOLD;
    int EROSION_CYCLES;
    int EROSION_ELEMENT_SIZE;
    int EROSION_THRESHOLD;
    int DIALATION_CYCLES;
    int DIALATION_ELEMENT_SIZE;
    int DIALATION_THRESHOLD;
    double CS_CENTER_RADIUS;
    double CS_CENTER_VALUE;
    double CS_SURROUND_RADIUS;
    double CS_SURROUND_VALUE;
    int CS_THRESHOLD_START;
    int CS_THRESHOLD_INCREMENT;
    int CS_THRESHOLD_MAX;
    int MARK_SIZE;
    int MARK_RADIUS;
    int MARK_COLOR[3];
    int SIGNAL_COLOR[3];
    int MIN_REGION_VOXELS;
    int MAX_REGION_VOXELS;
    ////////////////////////////////////////////////////

    int xDim;
    int yDim;
    int zDim;

    // Shared for dialateOrErode threads
    unsigned char*** currentSource;
    unsigned char*** currentTarget;
    int lastTargetIndex;

    // Shared for center-surround threads
    unsigned char*** csSource;
    unsigned char*** csOutput;
    double*** csFilterMatrix;
    double*** csTarget;
    double csCenterSize;
    double csCenterValue;
    double csSurroundSize;
    double csSurroundValue;
    double csFilterMin;
    double csFilterMax;

    QMutex csLock;
    QList<int> regionCoordinates;

    int errorStatus;
    bool loadedFromInputFile;


};

#endif // CELLCOUNTER3D_H
