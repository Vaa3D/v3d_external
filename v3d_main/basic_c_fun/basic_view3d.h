/*
 * basic_view3d.h
 *
 *  Created on: Aug 11, 2010
 *      Author: ruanz
 */

#ifndef BASIC_VIEW3D_H_
#define BASIC_VIEW3D_H_

class View3DInterface
{
public:
	virtual int renderMode() const =0;
	virtual int dataDim1() const =0;
	virtual int dataDim2() const =0;
	virtual int dataDim3() const =0;
	virtual int dataDim4() const =0;
	virtual int dataDim5() const =0;

	virtual int xRot() const =0;
	virtual int yRot() const =0;
	virtual int zRot() const =0;
	virtual int zoom() const =0;
	virtual int xShift() const =0;
	virtual int yShift() const =0;
	virtual int zShift() const =0;
	virtual bool isAbsoluteRot() const =0;

	virtual bool isVolCompress() const =0;
	virtual bool isShowBoundingBox() const =0;
	virtual bool isShowAxes() const =0;

	virtual void hideTool() =0;
	virtual void showTool() =0;
	virtual void updateTool() =0;
	virtual void updateControl() =0;

	virtual void stillPaint() =0;

	virtual int setVolumeTimePoint(int t) =0;
	virtual void incVolumeTimePoint(float step) =0;

	virtual void setRenderMode_Mip(bool b) =0;
	virtual void setRenderMode_Alpha(bool b) =0;
	virtual void setRenderMode_Cs3d(bool b) =0;

	virtual void setCSTransparent(int) =0;
	virtual void setThickness(double) =0;
	virtual void setCurChannel(int) =0;

	virtual void setChannelR(bool b) =0;
	virtual void setChannelG(bool b) =0;
	virtual void setChannelB(bool b) =0;
	virtual void setVolCompress(bool b) =0;

	virtual void volumeColormapDialog() =0;
	virtual void surfaceSelectDialog(int curTab=0) =0;
	virtual void surfaceSelectTab(int curTab=0) =0;
	virtual void surfaceDialogHide() =0;
	virtual void annotationDialog(int dataClass, int surfaceType, int index) =0;

	virtual void setXRotation(int angle) =0;
	virtual void setYRotation(int angle) =0;
	virtual void setZRotation(int angle) =0;
	virtual void resetRotation(bool b_emit=true) =0;
	virtual void modelRotation(int xRotStep, int yRotStep, int zRotStep) =0;
	virtual void viewRotation(int xRotStep, int yRotStep, int zRotStep) =0;
	virtual void absoluteRotPose() =0;
	virtual void doAbsoluteRot() =0;

	virtual void setZoom(int r) =0;
	virtual void setXShift(int s) =0;
	virtual void setYShift(int s) =0;
	virtual void setZShift(int s) =0;
	virtual void resetZoomShift() =0;

	virtual void enableFrontSlice(bool) =0;
	virtual void enableXSlice(bool) =0;
	virtual void enableYSlice(bool) =0;
	virtual void enableZSlice(bool) =0;

	virtual void setFrontCut(int s) =0;
	virtual void setXCut0(int s) =0;
	virtual void setYCut0(int s) =0;
	virtual void setZCut0(int s) =0;
	virtual void setXCut1(int s) =0;
	virtual void setYCut1(int s) =0;
	virtual void setZCut1(int s) =0;
	virtual void setXCS(int s) =0;
	virtual void setYCS(int s) =0;
	virtual void setZCS(int s) =0;
	virtual void setXClip0(int s) =0;
	virtual void setYClip0(int s) =0;
	virtual void setZClip0(int s) =0;
	virtual void setXClip1(int s) =0;
	virtual void setYClip1(int s) =0;
	virtual void setZClip1(int s) =0;

	virtual void enableShowAxes(bool b) =0;
	virtual void enableShowBoundingBox(bool b) =0;
	virtual void enableOrthoView(bool b) =0;
	virtual void setBackgroundColor() =0;
	virtual void setBright() =0;

	virtual void setShowMarkers(int s) =0;
	virtual void setShowSurfObjects(int s) =0;
	virtual void enableMarkerLabel(bool) =0;
	virtual void setMarkerSize(int s) =0;
	virtual void enableSurfStretch(bool) =0;
	virtual void toggleCellName() =0;

	virtual void createSurfCurrentR() =0;
	virtual void createSurfCurrentG() =0;
	virtual void createSurfCurrentB() =0;
	virtual void loadObjectsFromFile() =0;
	virtual void loadObjectsFromFile(QString url) =0;
	virtual void loadObjectsListFromFile() =0;
	virtual void saveSurfFile() =0;

	virtual void togglePolygonMode() =0;
	virtual void toggleLineType() =0;
	virtual void toggleObjShader() =0;

	virtual void changeLineOption() =0;
	virtual void changeVolShadingOption() =0;
	virtual void changeObjShadingOption() =0;

	virtual void toggleTexFilter() =0;
	virtual void toggleTex2D3D() =0;
	virtual void toggleTexCompression() =0;
	virtual void toggleTexStream() =0;
	virtual void toggleShader() =0;
	virtual void showGLinfo() =0;

	virtual void updateWithTriView() =0;
	virtual void updateImageData() =0;
	virtual void reloadData() =0;
	virtual void cancelSelect() =0;

};

#endif /* BASIC_VIEW3D_H_ */
