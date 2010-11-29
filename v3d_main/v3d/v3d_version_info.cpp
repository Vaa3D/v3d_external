/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it.

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




//by Hanchuan Peng
//081115: separated from the v3d_core.cpp
//last update: 090801 by PHC

#include "v3d_version_info.h"
#include "v3d_compile_constraints.h"
#include "../basic_c_fun/v3d_message.h"

#include <QtGui>
#include <QNetworkReply>
// #include <QXmlSchema> // Qt 4.6 or later only
#include <QtXml>
#include <QSettings>
#include <QDebug>
#include <QMessageBox>

// For testing:
QString v3dVersionUrlBase("http://brunsc-wm1.janelia.priv/~brunsc/v3d_version/");

// For production:
// QString v3dVersionUrlBase("http://penglab.janelia.org/proj/v3d/V3D/");

namespace v3d {
    // Set current version here.
    VersionInfo thisVersionOfV3D(2.543);
}
QString versionnumber = "V3D (" +
        v3d::thisVersionOfV3D.toQString() +
        "), V3D-Neuron (2.0), V3D Plugin Interface (2.1) ";

void v3d_aboutinfo()
{
	QString helptext =
		"<H3>V3D: A Swiss army knife for 3D/4D/5D volume image and surface visualization and processing, developed by Hanchuan Peng, Zongcai Ruan, Fuhui Long, et al. (JFRC, HHMI). All rights reserved.</H3> "
//		"<H3><span style=\"color:#FF0000\">If you are seeing this red color information, you are using an alpha-testing version of V3D. If you experience any problem, please contact Hanchuan Peng. </span></H3> "
		"If you have used V3D in your research and would like to cite it, please cite as the following: <br><br>"
		"<span style=\"color:#0000FF\">Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) \"V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,\" Nature Biotechnology, Vol. 28, No. 4, pp.348-353, DOI: 10.1038/nbt.1612. (http://penglab.janelia.org/proj/v3d) </span><br> "
		"<br>"
		"For the latest information/help and further documentations, visit the V3D website <a href=\"http://penglab.janelia.org/proj/v3d\">http://penglab.janelia.org/proj/v3d</a><br>"
		"<br>============================================="
		"<H1>Help Information</H1>"
		"=============================================<br>"
		"Most functions of this software should be intuitive and easy to use. A few short-cuts are listed below.<br>"
		"<H3>Supported file types</H3>"
		"You can also download a Matlab toolbox to read and write most of the basic data formats at the V3D website <a href=\"http://penglab.janelia.org/proj/v3d\">http://penglab.janelia.org/proj/v3d</a>."
		"<br><table border=\"1\">"
		"<tr><td>.TIF, .TIFF, .LSM (Tiff and LSM stacks) </td><td>3D image stacks. </td></tr>"
		"<tr><td>.RAW (V3D's RAW) </td><td>a simple raw file which supports 8-bit unsigned char, 16-bit unsigned short, 32-bit single-precision float 4D image stacks. </td></tr>"
		"<tr><td>.TIF (Series 2D tiff section) </td><td>the Leica scope produced series tiff files can be imported. </td></tr>"
		"<tr><td>.SWC </td><td>reconstructed neurons or other relational data which can be described as a graph. </td></tr>"
		"<tr><td>.APO </td><td>point cloud file used in WANO to describe the cells and any other image objects. </td></tr>"
		"<tr><td>.V3DS, .OBJ </td><td>surface object files. OBJ file is ASCII description of surface objects, which will be slow in opening but can be read by other software; V3DS is binary representation of surface and much faster to load. </td></tr>"
		"<tr><td>.ANO </td><td>the linker file which contains file names of other heterogeneous files. </td></tr>"
		"<tr><td>.MARKER </td><td>the landmark recording file (which is just a plain csv file with a specified format). </td></tr>"
		"<tr><td>.ATLAS </td><td>the linker file which contains a list of image-stack file names that can be opened using the atlas manager. </td></tr>"
		"<tr><td>.PC_ATLAS </td><td>the linker file which contains a list of point cloud files names for the point cloud atlas. </td></tr>"
									"<tr><td>.ZIP </td><td>on Mac and Linux the .zip files of the above V3D file formats can be opened automatically. </td></tr>"
		"</table><br>"
		"<H3>Keyboard operations</H3>"
		"When any view of images is active, the following short-cut keys can be used. </td></tr>"
		"<br><table border=\"1\">"
		//"<tr><td>N/. </td><td>Move to the next focal plane.</td></tr>"
		//"<tr><td>B/, </td><td>Move to the previous focal plane.</td></tr>"
		"<tr><td>M </td><td>Add a new landmark.</td></tr>"
		"<tr><td>I </td><td>Zoom-in by a factor 2.</td></tr>"
		"<tr><td>O </td><td>Zoom-out by a a factor 2 (will not be effective if any displayed dimension of an image is <=1 pixel.</td></tr>"
		"<tr><td>1/2/3/4 </td><td>set the tri-view zoom to x1 (no zoom-in or out) or x2, x3, x4 zoom. The windows may be zipped together but can be expaned by either slightly resizing the window boundary or click the cascade menu</td></tr>"
		"<tr><td>Ctrl-O (Windows, Linux) or Cmd-O (Mac) </td><td>Open an image stack. Note that you can also directly drag and drop an image file (Tiff stack or LSM or Hanchuan Peng's RAW files) into V3D to open it. </td></tr>"
		"<tr><td>Ctrl-I (Windows, Linux) or Cmd-I (Mac) </td><td>Import Leica tiff series as an image stack. </td></tr>"
		"<tr><td>Ctrl-S (Windows, Linux) or Cmd-S (Mac) </td><td>Save image file. </td></tr>"
		"<tr><td>Ctrl-C (Windows, Linux) or Cmd-C (Mac) </td><td>Crop image based on the Region Of Interest (ROI) defined (see the Ctrl-click mouse operation to edit the ROI). </td></tr>"
		"<tr><td>Shift-C </td><td>Switch among the three preset colormaps for an indexed/mask image (unsigned short int, 16bit). </td></tr>"
		//"<tr><td>Ctrl-D (Windows, Linux) or Cmd-D (Mac) </td><td>Delete the last added corner point of the ROI. </td></tr>"
		"<tr><td>Ctrl-R (Windows, Linux) or Cmd-R (Mac) </td><td>Rotate image. </td></tr>"
		"<tr><td>Ctrl-P (Windows, Linux) or Cmd-P (Mac) </td><td>Pop up the image processing dialog box. </td></tr>"
		"<tr><td>Ctrl-V (Windows, Linux) or Cmd-V (Mac) </td><td>3D view of an image stack (will downsize to 512x512x256 if bigger than that size). </td></tr>"
		"<tr><td>Shift-V </td><td>3D view of the region of interest (roi) defined for an image stack (if no roi defined, the  use entire image. auto downsize to 512x512x256 if bigger than that size). </td></tr>"
		"<tr><td>Ctrl-Shift-V (Windows, Linux) or Cmd-Shift-V (Mac) </td><td>3D view of an entire image stack (no image downsizing, may cause a video-memory crash if there is hardware limitation). </td></tr>"
		"<tr><td>Ctrl-A (Windows, Linux) or Cmd-A (Mac) </td><td>Launch atlas-viewer/image blender (together with the landmark manager). </td></tr>"
		"<tr><td>Ctrl-F (Windows, Linux) or Cmd-F (Mac) </td><td>Launch the landmark finder (together with atlas-viewer/image blender). </td></tr>"
		"<tr><td>Ctrl-Z (Windows, Linux) or Cmd-Z (Mac) </td><td>Undo last tracing operation.</td></tr>"
		"</table><br>"
		"<H3>Mouse operations</H3>"
		"<br><table border=\"1\">"
		"<tr><td>Double-click anywhere close to any existing landmark </td><td>choose to move or delete a landmark.</td></tr>"
		"<tr><td>Shift-click anywhere after selecting to move a landmark </td><td>move the specified landmark to a new location.</td></tr>"
		"<tr><td>Ctrl-click  (Windows, Linux) or Cmd-click (Mac) anywhere </td><td>add a new corner point to an ROI.</td></tr>"
	    "<tr><td>Alt-click anywhere </td><td>remove the last added corner point in an ROI.</td></tr>"
		"</table><br>"
		"<H3>Wheel operations</H3>"
		"<br><table border=\"1\">"
		"<tr><td>When click-in any of the XY, YZ, or XZ view of an image stack, you can scroll through different depths.</td></tr>"
		"</table><br>"
		"<H3>Special debugging keys</H3>"
		"<br><table border=\"1\">"
		//"<tr><td>Ctrl-E (Windows, Linux) or Cmd-E (Mac) </td><td>Change the landmark match method (useful for image registration). </td></tr>"
		//"<tr><td>Shift-E </td><td>Change the Displacement Field computing method. </td></tr>"
		//"<tr><td>W </td><td>short-cut to do find the best matching point for 1 single lamdmark. </td></tr>"
		"<tr><td>T </td><td>short-cut to trace a path between two landmarks (if landmarks defined). </td></tr>"
		"</table><br>"

		"<H3>For 3D view window short keys</H3>"
		"<br><table border=\"1\">"
		"<tr><td>Ctrl-W (Windows, Linux) or Cmd-W (Mac) </td><td>close current 3D viewer Window.</td></tr>"
		"<tr><td>Ctrl-A (Windows, Linux) or Cmd-A (Mac) </td><td>toggle Animating the rotation of objects.</td></tr>"
		"<tr><td>Ctrl-M (Windows, Linux) or Cmd-M (Mac) </td><td>generate Movie.</td></tr>"
		"<tr><td>Ctrl-B (Windows, Linux) or Cmd-B (Mac) </td><td>change Brightness of whole view.</td></tr>"
		"<tr><td>Ctrl-R (Windows, Linux) or Cmd-R (Mac) </td><td>Reload data set (return to the initial data set, but user-controlled parameters isn't changed).</td></tr>"
		"<tr><td>Ctrl-U (Windows, Linux) or Cmd-U (Mac) </td><td>Update landmarks and traced curves from associated tri-view.</td></tr>"
		"<tr><td>Ctrl-P (Windows, Linux) or Cmd-P (Mac) </td><td>toggle the Polygon-fill/line/point/transparent display mode of surface object.</td></tr>"
		"<tr><td>Alt-P									</td><td>change Polygon display options of surface object.</td></tr>"
		"<tr><td>Ctrl-L (Windows, Linux) or Cmd-L (Mac) </td><td>toggle skeLeton-view and tube-view of SWC objects (e.g. a neuron).</td></tr>"
		"<tr><td>Alt-L									</td><td>change skeLeton-view options of SWC objects (e.g. a neuron).</td></tr>"
		"<tr><td>Ctrl-Z (Windows, Linux) or Cmd-Z (Mac) </td><td>Undo last tracing/editing operation of SWC objects (e.g. a neuron).</td></tr>"
		"<tr><td>Ctrl-X (Windows, Linux) or Cmd-X (Mac) </td><td>Redo last tracing/editing operation of SWC objects (e.g. a neuron).</td></tr>"
		"<tr><td>Ctrl-N (Windows, Linux) or Cmd-N (Mac) </td><td>toggle Name of APO object (e.g. point cloud).</td></tr>"
		"<tr><td>Ctrl-V (Windows, Linux) or Cmd-V (Mac)	</td><td>update Volume image from tri-view.</td></tr>"
		"<tr><td>Alt-V									</td><td>change Volume rendering advance options.</td></tr>"
		"<tr><td>Ctrl-F (Windows, Linux) or Cmd-F (Mac) </td><td>toggle volume texture Filter.</td></tr>"
		"<tr><td>Mouse-drag </td>                         <td>Rotation around x/y-axis of view space, like track ball.</td></tr>"
		"<tr><td>Shift-Mouse-wheel, [ and ] </td>         <td>Rotation around z-axis of view space.</td></tr>"
		"<tr><td>Alt-Mouse, Alt-arrows </td>              <td>Rotation around axes of objects itself.</td></tr>"
		"<tr><td>\\ </td>                                 <td>Reset rotation.</td></tr>"
		"<tr><td>Ctrl-\\ (Windows, Linux) or Cmd-\\ (Mac) </td><td>toggle Parallel projection (Scale bar).</td></tr>"
		"<tr><td>Shift-Mouse, Shift-arrows </td>     <td>Shift the objects around.</td></tr>"
		"<tr><td>Left, Right, Up, Down arrows </td>       <td>Shift the view window around.</td></tr>"
		"<tr><td>Mouse-wheel, - and = </td>               <td>Zoom-out and zoom-in.</td></tr>"
		"<tr><td>Backspace </td>                          <td>Reset zoom and shift.</td></tr>"
		"<tr><td>, and . </td>                            <td>Move the volume front/f-slice plane.</td></tr>"
		"<tr><td>/ </td>                                  <td>Reset volume front plane and cross-section planes.</td></tr>"
		"<tr><td>Mouse-right-button </td>                 <td>Select object and active popup menu of the selected object.</td></tr>"
		"<tr><td>[n]-holding-Mouse-right-button </td>     <td>Marker on [n]th channel when define marker or curve.</td></tr>"
		"<tr><td>Shift - and Shift = </td>                <td>Decrease and increase marker size.</td></tr>"
		"<tr><td>Escape </td>							  <td>Exit from status of defining markers or curves.</td></tr>"
		"<tr><td>Shift , and Shift . </td>                <td>Backward and forward volume time point.</td></tr>"
		"<tr><td>Shift / </td>		    				  <td>Reset volume time point.</td></tr>"
		"</table><br>"
		"<H3>Volume colormap control</H3>"
		"Rectangle point is start or end of colormap curve, and its horizontal coordinate is locked at left or right side. Circle point can be moved freely.<br>"
		"<br><table border=\"1\">"
		"<tr><td>Mouse-left-click </td>     <td>Insert a control point to colormap curve.</td></tr>"
		"<tr><td>Mouse-right-click </td>    <td>Delete specified control point of colormap curve.</td></tr>"
		"<tr><td>Mouse-drag </td>    		<td>Move specified control point of colormap curve.</td></tr>"
		"</table><br>"

		"<H3>Some known problems</H3>"
		"So far V3D has been tested on hundreds of machines with different software/hardware environments; in most cases (Mac, Linux, Windows) the software has a nice performance. Yet the following are some known problems.<br>"
		"<br><table border=\"1\">"
		"<tr><td>Mac machines with Tiger (Mac OS X 10.4) and defective NVIDIA GeForce 7300GT video cards</td>     <td>GeForce 7300GT does not support multisampling while it claims to be. This incompatibility makes 3D viewer run very slow. A special V3D version disables the multipling support on this video card solves the problem.</td></tr>"
		"<tr><td>MacBook Pro with Tiger (Mac OS X 10.4) and defective NVIDIA GeForce 8600MGT video card driver</td>     <td>Sometimes the labels of markers may be hidden in the 3D volume image rendering. Upgrage to Leopard OS (and thus also upgrade Mac OpenGL driver) solves the problem.</td></tr>"
		"<tr><td>MacBook Pro with Tiger (Mac OS X 10.4) and defective NVIDIA GeForce 8600MGT video card driver</td>     <td>The 5D rendering displays abnormal patterns due to a problem in 3D texture of the video card driver. </td></tr>"
		"<tr><td>Some Windows machines cannot display the 3D viewer</td>     <td>Some video card drivers report that it supports OpenGL 2.0, but actually it cannot support. Try to press Ctrl-Shift-G to disable OpenGL 2.0 functions used.  </td></tr>"
		"<tr><td>Neuron editing: the 'merge to nearby segments' function may produce extra root-nodes</td>     <td>The neuron editing function can become quite complicated as we'd like to support any graph. if you see unwanted roots, you may choose 'reverse' nearby segments to correct; but this may make other segments' directions change.  Suggest use 'break' combined by 'tracing' instead of direct 'merge'.</td></tr>"
		"</table><br>"
		"<H2>Key releases:</H2><br> Feb 2006, Aug 16 (v1.1), Sept 24 (v1.2), Oct 11 (v1.3), 2006. July 16 (v1.4), Aug 6 (v1.5) 2007. "
		"Jan 7 (v1.6), April 16 (v1.70), June 13 (v1.81), Aug, 20 (v1.90), Aug 22 (v1.91), Aug 29 (v1.924), Sept 4 (v1.93), Sept 10 (v1.94), Oct 8 (v1.95), "
		"Oct 27 (v1.960), Nov 14 (v1.970), Dec 11 (v1.980), Dec 31 (v1.990), 2008. "
		"Feb 20 (v2.000), May 4 (v2.032), May 16 (v2.100), May 20 (v2.110), June 10 (v2.121), July 5 (v2.140), July 13 (v2.200), Aug 1 (v2.300), Aug 14 (v2.310), Aug 20 (v2.320), Sept (v2.400), 2009. "
		"Jan 31 (v2.449), Feb-Aug (v2.4-v2.5, a number of important updates), 2010. "
		"All rights reserved.<br><br> "
		"Version Note: if you see a version number followed by a letter (e.g. 1.80a), this is a customized build for a particular research lab. "
		"V3D also have additional image analysis modules, for further information, contact Hanchuan Peng.<br><br>";

	//QMessageBox::information(0, versionnumber, versionnumber + "<br>" + helptext);

	QString v3d_compile_info = "";
	if (COMPILE_TARGET_LEVEL==2)
		v3d_compile_info = "Advantage (alpha)"; //full version with all toolboxes
	else if (COMPILE_TARGET_LEVEL==1)
		v3d_compile_info = "Pro";
	else //COMPILE_TARGET_LEVEL==0
		v3d_compile_info = "Lite";

	QString build_info = QString("build time: ")+BUILD_TIME+", "+BUILD_OS_INFO+", "+BUILD_QT_INFO+" "+BUILD_BITS;


	QTextEdit *p_mytext = new QTextEdit(versionnumber + v3d_compile_info +
							"<br>" + build_info +
							"<br><br>" + helptext);
	p_mytext->setDocumentTitle(versionnumber);
	p_mytext->resize(700, 700); //use the QWidget function
	p_mytext->setReadOnly(true);
	p_mytext->setFontPointSize(12);
	p_mytext->show();

}


void v3d_Lite_info()
{
	QString helptext =      "<H2><span style=\"color:#0000FF\">You are using the </span><span style=\"color:#FF0000\">V3D Lite</span>.<span style=\"color:#0000FF\"> "
							"Most 3D visualization modules have been enabled. However the 3D data/image processing modules have been disabled. "
							"To enable these tools (and disable this message), you can <span style=\"color:#FF0000\">register as a V3D Pro user for free</span> at http://penglab.janelia.org/proj/v3d </span></H2>";

	//QMessageBox::information(0, versionnumber, versionnumber + "<br>" + helptext);

	QString v3d_compile_info = "";
	if (COMPILE_TARGET_LEVEL==2)
		v3d_compile_info = "Advantage (alpha)"; //full version with all toolboxes
	else if (COMPILE_TARGET_LEVEL==1)
		v3d_compile_info = "Pro";
	else //COMPILE_TARGET_LEVEL==0
		v3d_compile_info = "Lite";

	QString build_info = QString("build time: ")+BUILD_TIME+", "+BUILD_OS_INFO+", "+BUILD_QT_INFO+" "+BUILD_BITS;


	v3d_msg(versionnumber + v3d_compile_info +
			"<br>" + build_info +
			"<br><br>" + helptext);
//	p_mytext->setDocumentTitle(versionnumber);
//	p_mytext->resize(300, 300); //use the QWidget function
//	p_mytext->setReadOnly(true);
//	p_mytext->setFontPointSize(12);
//	p_mytext->show();
//
}

namespace v3d {

V3DVersionChecker::V3DVersionChecker(QWidget *guiParent_param)
    : guiParent(guiParent_param)
{}

void V3DVersionChecker::checkForLatestVersion(bool b_verbose)
{
    this->b_showAllMessages = b_verbose;
    QUrl versionUrl(v3dVersionUrlBase + "v3d_version.xml");
    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    nam->get(QNetworkRequest(versionUrl));
    connect(nam, SIGNAL(finished(QNetworkReply*)),
        this, SLOT(gotVersion(QNetworkReply*)));
}

// Examines last time version updater was queried to decide whether it might
// be time to check again now.
bool V3DVersionChecker::shouldCheckNow() {
    bool bCheckNow = true; // default is to check for updates
    // Load relevant persistent variables from Qt cache
	QSettings settings("HHMI", "V3D");
    QVariant latestCheckVariant = settings.value("timeOfLatestUpdateCheck");
    QVariant checkIntervalVariant = settings.value("updateCheckInterval");
    // Use default behavior unless persistent variables are set
    if ( latestCheckVariant.isValid() && checkIntervalVariant.isValid())
    {
        // we know how long it has been and how long it should be
        int checkInterval = checkIntervalVariant.toInt();
        if (checkInterval == 0) // zero means "always"
            bCheckNow = true; // redundant, as true is the default
        else if (checkInterval < 0) // negative means "never"
            bCheckNow = false;
        else {
            // finite interval needs comparison
            QDateTime latestCheck = latestCheckVariant.toDateTime();
            int elapsedInterval = latestCheck.secsTo(QDateTime::currentDateTime());
            if (elapsedInterval < checkInterval)
                bCheckNow = false; // user does not wish to be bothered now
            else
                bCheckNow = true; // redundant, true is default
        }
    }
    return bCheckNow;
}

void V3DVersionChecker::gotVersion(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug("Problem downloading latest version information");
        if (b_showAllMessages) {
            QMessageBox::information(guiParent,
                    "Unable to connect to V3D server",
                    "Could not get latest version information.\n"
                    "Please try again later.");
        }
        return; // error occurred, so don't bother
    }

    // CMB Oct-22-2010
    // If we get this far, consider the update "checked" for the purpose
    // of update check timeout interval.  In particular, even if the update
    // file is unparsable, don't necessarily check again immediately.
	QSettings settings("HHMI", "V3D");
    settings.setValue("timeOfLatestUpdateCheck", QDateTime::currentDateTime());

    // CMB Oct-22-2010
    // You might wonder why I would use an xml file for such a simple version file.
    // The reason is that we might in the future want to have different versions
    // for different
    // platforms or for different versions of Qt.  This way we can maintain
    // backwards compatibility.  We can add new elements and new attributes
    // to the xml for this purpose, and older versions of v3d will just ignore
    // them, looking only at the major and minor versions.

    // Qt 4.6 or later is required for validation with xml schema
    QDomDocument versionDoc("v3d_version");
    versionDoc.setContent(reply->readAll());
    processVersionXmlFile(versionDoc);
}

void V3DVersionChecker::processVersionXmlFile(const QDomDocument& versionDoc)
{
    QDomElement root = versionDoc.documentElement();
    if (root.tagName() != "v3d_version") {
        qDebug() << "Unrecognized root element " << root.tagName();
        if (b_showAllMessages) {
            QMessageBox::information(guiParent,
                    "Unable to parse version information",
                    "Could not parse latest version information.\n"
                    "Please try again later.");
        }
        return; // silently continue on version finding error
    }
    QDomNode node = root.firstChild();
    QString latest_major_version = "0";
    QString latest_minor_version = "000";
    while (!node.isNull())
    {
        QDomElement elem = node.toElement();
        if (!elem.isNull()) {
            if (elem.tagName() == "latest_stable_version") {
                latest_major_version = elem.attribute("major", "0");
                latest_minor_version = elem.attribute("minor", "0");
                break;
            }
        }
    }

    QString latestVersionString(latest_major_version + "." + latest_minor_version);
    qDebug() << "latest version = " << latestVersionString;
    v3d::VersionInfo latestVersion(latestVersionString);
    if (latestVersion > v3d::thisVersionOfV3D) {
        qDebug("There is a newer V3D version");
        V3dUpdateDialog v3dUpdateDialog(guiParent);
        int returnValue = v3dUpdateDialog.exec();
        if (returnValue == QDialog::Accepted) { // clicked "Open"
            bool b_openurl_worked;
            // Open user's browser to the V3D download page
            b_openurl_worked = QDesktopServices::openUrl(
                QUrl("http://penglab.janelia.org/proj/v3d/V3D/Download.html"));
            if (! b_openurl_worked)
                QMessageBox::warning(guiParent,
                        "Error opening V3D download page", // title
                        "Oops. V3D could not open your browser.\n"
                        "Please browse to\n"
                        "http://penglab.janelia.org/proj/v3d/V3D/Download.html\n"
                        "to get the latest version");
        }
    }
    else if (latestVersion == v3d::thisVersionOfV3D) {
        if (b_showAllMessages) {
            QMessageBox::information(guiParent,
                    "V3D is up to date",
                    "You are using the latest version of V3D");
        }
        qDebug("V3D version is up to date");
    }
    else {
        if (b_showAllMessages) {
            QMessageBox::information(guiParent,
                    "Special V3D version",
                    "You are using an unreleased development version of V3D");
        }
        qDebug("V3D version is more than up to date");
    }
}

V3dUpdateDialog::V3dUpdateDialog(QWidget* guiParent) : QDialog(guiParent)
{
    setupUi(this);
    // Sync with current update frequency
    QSettings settings("HHMI", "V3D");
    QVariant checkIntervalVariant = settings.value("updateCheckInterval");
    if ( checkIntervalVariant.isValid() )
    {
        int checkInterval = checkIntervalVariant.toInt();
        if (checkInterval < 0)
            comboBox->setCurrentIndex(comboBox->findText(tr("never")));
        else if (checkInterval == 0)
            comboBox->setCurrentIndex(comboBox->findText(tr("every time")));
        else if (checkInterval == 86400)
            comboBox->setCurrentIndex(comboBox->findText(tr("once a day")));
        else if (checkInterval == 604800)
            comboBox->setCurrentIndex(comboBox->findText(tr("once a week")));
        else if (checkInterval == 2592000)
            comboBox->setCurrentIndex(comboBox->findText(tr("once a month")));
        else
            qDebug() << "Error: unrecognized interval";
    }
}

void V3dUpdateDialog::on_comboBox_currentIndexChanged(const QString& updateFrequency)
{
    // Store update interval in the persistent cache
    qDebug() << "Changing update frequency to " << updateFrequency;
    QSettings settings("HHMI", "V3D");
    if (updateFrequency == tr("never"))
        settings.setValue("updateCheckInterval", -1);
    else if (updateFrequency == tr("every time"))
        settings.setValue("updateCheckInterval", 0);
    else if (updateFrequency == tr("once a day"))
        settings.setValue("updateCheckInterval", 86400);
    else if (updateFrequency == tr("once a week"))
        settings.setValue("updateCheckInterval", 604800);
    else if (updateFrequency == tr("once a month"))
        settings.setValue("updateCheckInterval", 2592000);
    else
        qDebug() << "Error: unrecognized interval";
}

UpdatesAvailableDialog::UpdatesAvailableDialog(QWidget *parent) : QMessageBox(parent)
{
    // Use V3D application icon
    QIcon appIcon(":/pic/v3dIcon128.png");
    QPixmap iconPixmap = appIcon.pixmap(75,75);
    setIconPixmap(iconPixmap);
    setText(tr("There are updates available."));
    setInformativeText("Do you want to start the V3D updater now?");
    setDetailedText("The V3D team periodically makes software improvements. "
            "Click 'Yes' to install recent improvements now.");
    setWindowTitle(tr("V3D software update check"));
    setSizeGripEnabled(false);

    QPushButton *noButton = addButton( tr("Remind me later"), QMessageBox::RejectRole);
    connect(noButton, SIGNAL(clicked()), this, SLOT(remind_me_later()));

    QPushButton *neverButton = addButton( tr("Never"), QMessageBox::DestructiveRole);
    connect(neverButton, SIGNAL(clicked()), this, SLOT(never_update()));

    QPushButton *yesButton = addButton( tr("Yes, update"), QMessageBox::AcceptRole);
    connect(yesButton, SIGNAL(clicked()), this, SLOT(yes_update()));
}

void UpdatesAvailableDialog::yes_update()
{
    emit update();
}

void UpdatesAvailableDialog::never_update()
{
    QSettings settings("HHMI", "V3D");
    settings.setValue("updateCheckInterval", -1); // never
}

void UpdatesAvailableDialog::remind_me_later()
{
    QSettings settings("HHMI", "V3D");
    int interval = settings.value("updateCheckInterval").toInt();
    if (interval < 0) // if set to never
        // run again when user starts V3D, but not for at least 5 minutes.
        settings.setValue("updateCheckInterval", 300);
}

} // namespace v3d
