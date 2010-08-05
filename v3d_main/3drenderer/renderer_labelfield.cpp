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




/*
 * renderer_labelfield.cpp
 *
 *  Created on: Sep 4, 2008
 *      Author: ruanzongcai
 * Last update: by Hanchuan Peng, 090219/20. enable the save and load surface object name in v3ds format
 * last update: by Hanchuan Peng, 090222. now always sort the craeted label surface using the label; also allow using a VANO annotation (APO) file to import the surface name
 * last update: by Hanchuan Peng, 090223. change the format of the v3ds format and now supporting save/load the display states of individual objects
 * last update: 090406 by Hanchuan Peng. fix a bug not to set the default s.on states when create the label field surface
 */

#include "renderer_tex2.h"

#include "marchingcubes.cpp"


////////////////////////////////////////////////////
//namespace LabelField {

bool lf_busy = false; // 090504 RZC : to prevent draw listLabelSurf when creating/loading it
QString lf_data_title = "";
My4DImage* lf_image4d = 0;
bool lf_fromfile = false;
unsigned int lf_mask = 0;
unsigned char* lf_data1d = 0;
unsigned char* lf_data_ch = 0;
int lf_sz0 = 0;
int lf_sz1 = 0;
int lf_sz2 = 0;
int lf_sz3 = 0;

static
void lf_setDataCh(int ch)
{
	CLAMP(0, lf_sz3-1, ch);
	if (lf_mask==0xff)		lf_data_ch = (unsigned char*)lf_data1d + (lf_sz2*lf_sz1*lf_sz0)*ch;      //*1
	if (lf_mask==0xffff)	lf_data_ch = (unsigned char*)lf_data1d + (lf_sz2*lf_sz1*lf_sz0)*ch*2;    //*2
	if (lf_mask==0xffffffff)	lf_data_ch = (unsigned char*)lf_data1d + (lf_sz2*lf_sz1*lf_sz0)*ch*4;    //*4
}
static inline
float lf_4dimageSample(int x, int y, int z)
{
	float f =0;
	if (lf_mask==0xff)		f = *(unsigned  char*)&(lf_data_ch[ (z*lf_sz1*lf_sz0 + y*lf_sz0 + x) ]);   //*1
	if (lf_mask==0xffff)	f = *(unsigned short*)&(lf_data_ch[ (z*lf_sz1*lf_sz0 + y*lf_sz0 + x)*2 ]); //*2
	if (lf_mask==0xffffffff)	f = *(float*)&(lf_data_ch[ (z*lf_sz1*lf_sz0 + y*lf_sz0 + x)*4 ]); //*4
	return  (f);
}
static inline
int bit_mask(unsigned int lf_mask)
{
	int f = 0;
	if (lf_mask==0xff)		f = 8;
	if (lf_mask==0xffff)	f = 16;
	if (lf_mask==0xffffffff)	f = 32;
	return f;
}

static int mesh_type = 0;		//0/1 --- label field/ range surface
static int mesh_iso0 = 0;		// range min
static int mesh_iso1 = 0;		// range max
static int mesh_method = 0;
static int mesh_density = 0;

//}
/////////////////////////////////////////////////////
const bool compiledLabelSurf = true; // display list or instantly drawing, seems display list is better, 081210


#define WARNING_MessageBox(title, type, what) { \
	QMessageBox::warning( 0, title, QObject::tr("%1: DATA INVAILD.\n---%2 exception: %3")\
			.arg(title).arg(type).arg(what) + "\n\n" + \
		QObject::tr("3D View: The associated image had been closed, or there is not associated image channel.\n\n") ); \
}
static char* i2strRGB[] = //{"R", "G", "B"};
					{"1", "2", "3"}; //081230

void Renderer_tex2::loadLabelfieldSurf(const QString& filename, int ch)
{
	qDebug("    Renderer_tex2::loadLabelfieldSurf(ch = %d)", ch);

    lf_data_title = filename; // 081004

#ifndef test_main_cpp
	if (lf_image4d && lf_fromfile) delete lf_image4d;	lf_image4d = 0;
	lf_fromfile = false;

	if (filename.isEmpty())  // using current image data
	{
		if (_idep==0) return;
		iDrawExternalParameter* ep = (iDrawExternalParameter*) _idep;
		lf_image4d = (My4DImage*) ep->image4d;
		if (! lf_image4d || ! (lf_image4d->valid()))
		{
			WARNING_MessageBox("loadLabelfieldSurf", "", "image invalid");
			return;
		}
		if (ch >= lf_image4d->getCDim())
		{
			WARNING_MessageBox("loadLabelfieldSurf", "", "no specified image channel");
			return;
		}
	    lf_data_title = ep->xwidget->windowTitle();
	}
#endif  // otherwise load image data from file after parameters were selected

    QString qtitle = QObject::tr("Creating Surface");
    QString qchannel = QObject::tr("From channel %1\n of %2\n\n\n").arg(i2strRGB[ch]).arg(lf_data_title);
	bool ok;

	// mesh type
	{
		QStringList items;
		items << "Label field surface" << "Range surface";
		QString item = QInputDialog::getItem(0, qtitle,
										qchannel+ QObject::tr("Creating Mesh type:"), items, 0, false, &ok);
		if (! ok) return;
		mesh_type = items.indexOf(item);
	}

	    qtitle = QObject::tr("Creating Label Field Surface");

	if (mesh_type==1) // range surface
    {
    	qtitle = QObject::tr("Creating Range Surface");
    	mesh_iso0 = QInputDialog::getInteger(0, qtitle,
										qchannel+ QObject::tr("Range from: "),
    									0, 0, 0xffff, 1, &ok);					// 0--2^16
    	if (! ok) return;
    	mesh_iso1 = QInputDialog::getInteger(0, qtitle,
										qchannel+  QObject::tr("Range from %1 to: ").arg(mesh_iso0),
    									mesh_iso0, mesh_iso0, 0xffff, 1, &ok);		// 0--2^16
    	if (! ok) return;
    }

    // common selection
	{
		QStringList items;
		items << "Marching Cubes" << "Marching Tetrahedrons";
		QString item = QInputDialog::getItem(0, qtitle,
										qchannel+ QObject::tr("Creating Mesh method:"), items, 0, false, &ok);
		if (! ok) return;
		mesh_method = items.indexOf(item);
		mesh_density = QInputDialog::getInteger(0, qtitle,
										qchannel+ QObject::tr("Creating Mesh density:"), 100, 0, 1000, 1, &ok);
		if (! ok) return;
	}

	//////////////////////////////////////////////////////////////////////////////
    PROGRESS_DIALOG( ((mesh_type==0)? "Loading label field" : "Loading range data"), widget);
	PROGRESS_PERCENT(20);

#ifndef test_main_cpp
	if (! filename.isEmpty()) // load image data from file
	{
		lf_image4d = new My4DImage;
		if (lf_image4d==0) return;

		lf_image4d->loadImage((char*)(filename.toStdString().c_str()));
		if (! (lf_image4d->valid()))
		{
			throw (const char*)"loadLabelfieldSurf -- loadImage failed";
			return;
		}
		lf_fromfile = true;
	    lf_data_title = filename;
	}
	int dataBytes = lf_image4d->getUnitBytes();

	lf_mask = 0;
	if (dataBytes==1)	lf_mask = 0xff;
	if (dataBytes==2)	lf_mask = 0xffff;
	if (dataBytes==4)	lf_mask = 0xffffffff;

	lf_data1d = lf_image4d->getRawData();
	lf_sz0 = lf_image4d->getXDim();
	lf_sz1 = lf_image4d->getYDim();
	lf_sz2 = lf_image4d->getZDim();
	lf_sz3 = lf_image4d->getCDim();

	lf_setDataCh(ch); ////////////////////////////////////////

#endif

    PROGRESS_PERCENT(100); //auto close when reach 100%


    ///////////////////////////////////////////////////////////////
    // marching cubes
    constructLabelfieldSurf(mesh_method, mesh_density);
}


static inline
float _labelSampleFunc(float fX, float fY, float fZ)
{
	float x, y, z;
	x = CLAMP01(fX)*(lf_sz0-1);
	y = CLAMP01(fY)*(lf_sz1-1);
	z = CLAMP01(fZ)*(lf_sz2-1);

	#define SAMPLE(ix,iy,iz)	lf_4dimageSample(ix,iy,iz)

	int x0,x1, y0,y1, z0,z1;
	x0 = floor(x); 		x1 = ceil(x);
	y0 = floor(y); 		y1 = ceil(y);
	z0 = floor(z); 		z1 = ceil(z);
	float xf, yf, zf;
	xf = x-x0;
	yf = y-y0;
	zf = z-z0;
	float is[2][2][2];
	is[0][0][0] = SAMPLE(x0, y0, z0);
	is[0][0][1] = SAMPLE(x0, y0, z1);
	is[0][1][0] = SAMPLE(x0, y1, z0);
	is[0][1][1] = SAMPLE(x0, y1, z1);
	is[1][0][0] = SAMPLE(x1, y0, z0);
	is[1][0][1] = SAMPLE(x1, y0, z1);
	is[1][1][0] = SAMPLE(x1, y1, z0);
	is[1][1][1] = SAMPLE(x1, y1, z1);
	float sf[2][2][2];
	sf[0][0][0] = (1-xf)*(1-yf)*(1-zf);
	sf[0][0][1] = (1-xf)*(1-yf)*(  zf);
	sf[0][1][0] = (1-xf)*(  yf)*(1-zf);
	sf[0][1][1] = (1-xf)*(  yf)*(  zf);
	sf[1][0][0] = (  xf)*(1-yf)*(1-zf);
	sf[1][0][1] = (  xf)*(1-yf)*(  zf);
	sf[1][1][0] = (  xf)*(  yf)*(1-zf);
	sf[1][1][1] = (  xf)*(  yf)*(  zf);

	float* ip = &is[0][0][0];
	float* sp = &sf[0][0][0];
	float count = 0;
	for (int i=0; i<8; i++) // percent filter
	{
		count += (mesh_iso0<=ip[i] && ip[i]<=mesh_iso1) * sp[i];
	}
	return (count*255);
}

void Renderer_tex2::constructLabelfieldSurf(int mesh_method, int mesh_density)
{
	qDebug("    Renderer_tex2::constructLabelfieldSurf");

	//==========================================
	//090222: try to import surface obj name if possible
	QList <CellAPO> mycelllist;
	if (QMessageBox::question (0, "", "Do you want to import label field name from a VANO APO annotation file?", QMessageBox::Yes, QMessageBox::No)
		== QMessageBox::Yes)
	{
		QString curFile = QFileDialog::getOpenFileName(0,
													   QObject::tr("Select a VANO APO annotation file... "),
													   QObject::tr(""),
													   QObject::tr("annotation (*.apo);;All Files (*)"));
		if (!curFile.isEmpty())
			mycelllist = listFromAPO_file(curFile);
	}

	//original code

	lf_busy = true;
	PROGRESS_DIALOG("Creating geometric data", widget);
	PROGRESS_PERCENT(1); // 0 or 100 not be displayed. 081102

	QTime qtime;
	qtime.start();

	////////////////////////////////////////////
	cleanLabelfieldSurf();
	////////////////////////////////////////////

	////////////////////////////////////////////////
	//count label list
	listLabelSurf.clear();
    if (mesh_type==0) // 090423 RZC: for Label Surface
    {
		qDebug("-------------------------------------------------------");
		unsigned int count = 0;
		{
			for (int z=0; z<lf_sz2; z++)
			{
				PROGRESS_TEXT( QObject::tr("Counting label: %1 labels").arg(count));
				PROGRESS_PERCENT(z*99/lf_sz2);

				for(int y=0; y<lf_sz1; y++)
				for(int x=0; x<lf_sz0; x++)
				{
					if (count==lf_mask) break;

					int label = lf_4dimageSample(x,y,z);
					LabelSurf S;
					S.n = count+1;
					S.label = label;
					S.label2 = label;
					S.name = ""; //strcpy(S.name, "");
					S.on = true; //090406
					//S.color;

					if (label>0  &&  ! listLabelSurf.contains(S))
					{
//						//always generate a sorted list. by PHC, 090222  // 090427 RZC: replaced by qSort
//						bool b_insert=false;
//						for (int tmpi=0;tmpi<listLabelSurf.size();tmpi++)
//						{
//							if (listLabelSurf[tmpi].label>label)
//							{
//								b_insert=true; listLabelSurf.insert(tmpi, S); break;
//							}
//						}
//						if (b_insert==false)
							listLabelSurf.append(S);
						count++;
					}
				}
			}
			//090427 RZC: Sorting after counting do not make it fast. Because most of time costed are in walking through the volume
			qSort(listLabelSurf);

			qDebug("----%dx%dx%d %d-channel %d-bit-------------read %d labels",
					lf_sz0,lf_sz1,lf_sz2,lf_sz3, bit_mask(lf_mask), listLabelSurf.size());
		}
    }
    else // 090504 RZC: for Range Surface, just 1 label group
    {
		LabelSurf S;
		S.n = 1;
		S.label = mesh_iso0;
		S.label2 = mesh_iso1;
		S.name = ""; //strcpy(S.name, "");
		S.on = true; //090406
		//S.color;

		listLabelSurf.append(S);
    }

	//==========================================
	//090222: try to import surface obj name if possible

	for (int i=0;i<mycelllist.size() && i<listLabelSurf.size();i++)
	{
		listLabelSurf[i].name = mycelllist[i].name;
		listLabelSurf[i].comment = mycelllist[i].comment;
	}

	//=========================================
	//MESSAGE_ASSERT(0);

	int f_num = 0;
	int num_surf = listLabelSurf.size();
	for (int i=0; i<num_surf; i++)
	{
		int t_num = 0;
		PROGRESS_TEXT( QObject::tr("Creating geometric group/label %1 of %2").arg(i+1).arg(num_surf) );
		PROGRESS_PERCENT((i+1)*90/num_surf);
		{

			MESSAGE_ASSERT(i>=0 && i<listLabelSurf.size());
			// _labelSampleFunc parameters
			mesh_iso0 = listLabelSurf.at(i).label;
			mesh_iso1 = listLabelSurf.at(i).label2;

			Triangle* pT = MarchingCubes(mesh_density, 255/2.f, _labelSampleFunc, mesh_method); //method 0 is better for label data

			// restore scale
			for (Triangle* p = pT; p!=NULL; p = p->next)
				for (int iCorner = 0; iCorner < 3; iCorner++)
				{
					p->vertex[iCorner][0] *= lf_sz0;
					p->vertex[iCorner][1] *= lf_sz1;
					p->vertex[iCorner][2] *= lf_sz2;
				}

			t_num = numTriangles(pT);
			qDebug("		#%d label(%d-%d) triangle num = %d", i, mesh_iso0, mesh_iso1, t_num);

			list_listTriangle.append(pT);
		}
		f_num += t_num;
	}
	qDebug( "num_surf=%d, list_pTriangle.size()=%d", num_surf, list_listTriangle.size());
	MESSAGE_ASSERT(num_surf==list_listTriangle.size());

	PROGRESS_TEXT("Compiling geometric data");
	PROGRESS_PERCENT(95);


	// clear image after create compiled list
	// reserve triangle list for save to file
#ifndef test_main_cpp
	if (lf_image4d && lf_fromfile) delete lf_image4d;
#endif
	lf_image4d = 0;
	lf_fromfile = false;
	lf_mask = 0;
	lf_data1d = 0;

	// create compiled list
		compileLabelfieldSurf();

	qDebug("    Renderer_tex2::createLabelfieldSurf %d faces, cost time = %g sec", f_num, qtime.elapsed()*0.001);
	lf_busy =false;

}

void Renderer_tex2::compileLabelfieldSurf(int update)
{
	makeCurrent(); //ensure right context when multiple views animation or mouse drop, 081118

	int num_surf = list_listTriangle.size();
	if (update==0) // initial color & on
	{

		for (int i=0; i<listLabelSurf.size(); i++) // Label Surface: set random color
		{
			LabelSurf& S = listLabelSurf[i];
			S.color = random_rgba8(255);
			//S.on = true; //commented on 090223 by PHC. to enable hiding some surface obj by default
		}

		// bounding box
		labelBB = NULL_BoundingBox;
		for (int i=0; i<num_surf; i++)
		{
			struct Triangle* p;
			struct Triangle* pnext;
			for (p = list_listTriangle[i]; p != NULL; p = pnext)
			{
				pnext = p->next;
				for (int j=0; j<3; j++)
				{
					labelBB.expand(XYZ(p->vertex[j][0],p->vertex[j][1],p->vertex[j][2]));
				}
			}
		}
	}

	////////////////////////////////////////////////////
	for (int i=0; i<list_glistLabel.size(); i++)
		glDeleteLists(list_glistLabel[i], 1);
	list_glistLabel.clear();

	if (compiledLabelSurf)
		for (int i=0; i<listLabelSurf.size(); i++)
	{
		GLuint g = glGenLists(1);
		list_glistLabel.append(g);

		glNewList(g, GL_COMPILE);
		{
			// set color move to drawing, 081114

			RENDER_TRIANGLES(list_listTriangle[i]);
		}
		glEndList();
	}

}

void Renderer_tex2::cleanLabelfieldSurf()
{
	for (int i=0; i<list_listTriangle.size(); i++)
	{
		delTriangles(list_listTriangle[i]);
	}
	list_listTriangle.clear();

	for (int i=0; i<list_glistLabel.size(); i++)
	{
		glDeleteLists(list_glistLabel[i], 1);
	}
	list_glistLabel.clear();

	listLabelSurf.clear();
}

void Renderer_tex2::drawLabelfieldSurf()
{
	if (sShowSurfObjects==0 || lf_busy) return;
	//qDebug("    Renderer_tex2::drawLabelfieldSurf");

	//glPushMatrix();

	for (int pass=0; pass<numPassFloatDraw(sShowSurfObjects); pass++)
	{
		setFloatDrawOp(pass, sShowSurfObjects);

		for (int i=0; i<listLabelSurf.size(); i++)
		{
			const LabelSurf& S = listLabelSurf[i];
			if (! S.on) continue;
			if (S.selected)  HIGHLIGHT_ON();

			glColor4ubv(S.color.c);

			glPushName(1+i);
			if (compiledLabelSurf)
				glCallList(list_glistLabel[i]);
			else
				RENDER_TRIANGLES(list_listTriangle[i]);
			glPopName();

			if (S.selected) HIGHLIGHT_OFF();
		}
	}
	setFloatDrawOp(-1, sShowSurfObjects);

	//glPopMatrix();
}


void Renderer_tex2::createSurfCurrent(int ch)
{
	try {

		loadLabelfieldSurf("", ch);

	} catch(const char* s) {
		WARNING_MessageBox("createSurfCurrent", "", s);
	}

    updateBoundingBox(); ///// 081121, all of created bounding-box are updated here
}


///////////////////////////////////////////////////////////////////////////////////////////

void Renderer_tex2::saveWavefrontOBJ(const QString& filename)
{
	QFile qf(filename);
	if (! qf.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		//QMessageBox::critical(0, QObject::tr("open file"), QObject::tr("open file [%1] failed!").arg(filename));
		throw  (const char*)(QObject::tr("open file [%1] failed!").arg(filename).toStdString().c_str());
		return;
	}

    qDebug("-------------------------------------------------------");
    int f_num = 0;
    int v_num = 0;
    char buf[200];
	for (int i=0; i<list_listTriangle.size(); i++)
	{
		int g_label0 = (mesh_type==0)? listLabelSurf[i] : mesh_iso0;
		int g_label1 = (mesh_type==0)? listLabelSurf[i] : mesh_iso1;
		sprintf(buf, "g %d %d\n", g_label0, g_label1);	//qDebug("%s", buf);
		qf.write(buf, strlen(buf));

		Triangle* pT = list_listTriangle[i];
		for (Triangle* p = pT; p != NULL; p = p->next)
		{
			for (int iCorner = 0; iCorner < 3; iCorner++)
			{
				XYZ v, vn;
				v.x = p->vertex[iCorner][0];
				v.y = p->vertex[iCorner][1];
				v.z = p->vertex[iCorner][2];
				vn.x = p->normal[iCorner][0];
				vn.y = p->normal[iCorner][1];
				vn.z = p->normal[iCorner][2];

				sprintf(buf, "v %g %g %g\n", v.x, v.y, v.z);	//qDebug("%s", buf);
				qf.write(buf, strlen(buf));
				sprintf(buf, "vn %g %g %g\n", vn.x, vn.y, vn.z);	//qDebug("%s", buf);
				qf.write(buf, strlen(buf));
			}

			sprintf(buf, "f %d//%d %d//%d %d//%d\n", v_num+1,v_num+1, v_num+2,v_num+2, v_num+3,v_num+3);	//qDebug("%s", buf);
			qf.write(buf, strlen(buf));
			v_num += 3;
			f_num ++;
		}
		qDebug("		#%d label(%d-%d) triangle num = %d", i, g_label0, g_label1, numTriangles(pT));
	}
	qDebug("---------------------write %d objects, %d triangles, %d vertices", list_listTriangle.size(), f_num, v_num);
}


inline
static QStringList _readVTN( TriangleIndex& S, int k, const QString& qs)
{
	QStringList qsl = qs.split("/");
	for (int i=0; i<qsl.size(); i++)
	{
		if (i==0)  S.vi[k] = qsl[i].toInt();
		if (i==1)  S.ti[k] = qsl[i].toInt();
		if (i==2)  S.ni[k] = qsl[i].toInt();
	}
	return qsl;
}

static void _convertIndexedFace2Triangle(
	    QList <XYZ> & listVertex,
	    QList <XYZ> & listNormal,
	    QList <TriangleIndex> & listFaceIndex,
	    QList <QList <int> > & list2Group,  // group(faceIndex list)
		QList <Triangle*> & list_pTriangle) // output
{
	try
	{
		for (int i=0; i<list2Group.size(); i++)		// each group/label
		{
			QList <int> & list1Group = list2Group[i];
			Triangle *pT0, *pT1;
			pT0 = pT1 = 0;

			for (int iFace=0; iFace<list1Group.size(); iFace++)		// each face/triangle
			{
				int f_num = list1Group[iFace];
				MESSAGE_ASSERT(f_num>0);
				TriangleIndex & Ti = listFaceIndex[f_num-1];

				Triangle* pT = new Triangle;
				for (int iConner=0; iConner<3; iConner++)
				{
					int v_num = Ti.vi[iConner];
					int n_num = Ti.ni[iConner];
					//qDebug("[iFace iConner: fi vi ni] %d %d: %d %d %d", iFace,iConner, f_num, v_num, n_num);

					MESSAGE_ASSERT(v_num>0);
					XYZ vertex = listVertex[ v_num-1 ];
					XYZ normal = (n_num>0)? listNormal[ n_num-1 ] : XYZ(1);
					pT->vertex[iConner][0] = vertex.x;
					pT->vertex[iConner][1] = vertex.y;
					pT->vertex[iConner][2] = vertex.z;
					pT->normal[iConner][0] = normal.x;
					pT->normal[iConner][1] = normal.y;
					pT->normal[iConner][2] = normal.z;
					pT->next = NULL;
				}

				if (iFace == 0)
				{
					pT0 = pT1 = pT;
				}
				else
				{
					pT1->next = pT;
					pT1 = pT;
				}
			}// each face

			//qDebug("      group(%d)  %d faces converted %d triangles", i, list1Group.size(),numTriangles(pT0));
			MESSAGE_ASSERT(list1Group.size()==numTriangles(pT0));
			list_pTriangle.append(pT0);				// one group triangles
		}// each group
	}
	catch(...)
	{
		QMessageBox::critical(0, "convertIndexedFace2Triangle", QObject::tr("convertIndexedFace2Triangle: OUT OF MEMEORY."));
	}
}


void Renderer_tex2::loadWavefrontOBJ(const QString& filename)
{
	QFile qf(filename);
	if (! qf.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		//QMessageBox::critical(0, QObject::tr("open file"), QObject::tr("open file [%1] failed!").arg(filename));
		throw  (const char*)(QObject::tr("open file [%1] failed!").arg(filename).toStdString().c_str());
		return;
	}

	lf_busy = true;
	PROGRESS_DIALOG("Loading surface          ", widget);
	PROGRESS_PERCENT(1); // 0 or 100 not be displayed. 081102

	QTime qtime;
	qtime.start();

	////////////////////////////////////////////
	cleanLabelfieldSurf();
	////////////////////////////////////////////


	mesh_type = 0;

	int count = 0;
    int v_num = 0;
	int f_num = 0;
	int g_num = 0;
    QList <XYZ> listVertex, listNormal;
    QList <TriangleIndex> listFaceIndex;
    QList <QList <int> > list2Group;
    listLabelSurf.clear();
    qDebug("-------------------------------------------------------");

	try
	{
		PROGRESS_PERCENT(100); //trick

		while (! qf.atEnd())
		{
			char _buf[200], *buf;
			qf.readLine(_buf, sizeof(_buf));
			for (buf=_buf; (*buf && *buf==' '); buf++);
			if (buf[0]=='#' ||buf[0]=='\0')	continue;

			count++;
		    LabelSurf S;
	        //memset(&S, 0, sizeof(S));

			QStringList qsl = QString(buf).split(" ",QString::SkipEmptyParts);
			switch (buf[0])
			{

			///////////////////////////////////////////////////////////////////////
			case 'g':
			{
				g_num++;
				PROGRESS_TEXT( QObject::tr("Loading geometric group/label %1 ").arg(g_num) );
				PROGRESS_PERCENT((g_num) %90);

				S.n = g_num;
				for (int i=0; i<qsl.size(); i++)
				{
					qsl[i].truncate(99);
					if (i==1) S.label = qsl[i].toInt();
					if (i==2) S.label2 = qsl[i].toInt();
					if (i==3) S.name = qsl[i]; //strcpy(S.name, Q_CSTR(qsl[i]));
				}
				S.on = true; // 090504 RZC
				//qDebug("%s  ///  g(%d)  %d", buf,  g_num, S.label);

				listLabelSurf.append(S);

				QList <int> list1Group;
				list1Group.clear();
				list2Group.append(list1Group);
			} break;

			///////////////////////////////////////////////////////////////////////
			case 'v':
			{
				XYZ V;
				for (int i=0; i<qsl.size(); i++)
				{
					qsl[i].truncate(99);
					if (i==1)  V.x = qsl[i].toFloat();
					if (i==2)  V.y = qsl[i].toFloat();
					if (i==3)  V.z = qsl[i].toFloat();
				}
				if (buf[1]=='n')
				{
					//qDebug("%s  ///  vn  %g %g %g", buf,  V.x, V.y, V.z);
					listNormal.append(V);
				}
				else
				{
					//qDebug("%s  ///  v  %g %g %g", buf,  V.x, V.y, V.z);
					listVertex.append(V);
					v_num ++;
				}
			} break;

			///////////////////////////////////////////////////////////////////////
			case 'f':
			{
				if (g_num<=1 && f_num%10000==0) // only one group
				{
					PROGRESS_TEXT( QObject::tr("Loading geometric face %1 ").arg(f_num) );
					PROGRESS_PERCENT(f_num/10000 %90);
				}

				TriangleIndex T;
				memset(&T, 0, sizeof(T));
				for (int i=0; i<qsl.size(); i++)
				{
					qsl[i].truncate(99);
					if (i==1)  _readVTN(T, i-1, qsl[i]); // v_num is 1-based
					if (i==2)  _readVTN(T, i-1, qsl[i]);
					if (i==3)  _readVTN(T, i-1, qsl[i]);
				}
				//qDebug("%s  ///  f  %d/%d/%d  %d/%d/%d  %d/%d/%d", buf,  T.vi[0],T.ti[0],T.ni[0], T.vi[1],T.ti[1],T.ni[1], T.vi[2],T.ti[2],T.ni[2]);

				listFaceIndex.append(T);
				f_num ++;

				if (g_num>0)  list2Group[g_num-1].append(f_num); // f_num is 1-based
			} break;

			/////////////////////////////////////////////////////////////////////////
			default:
				break;
			}// switch
		}
		if (g_num==0) // only one default group
		{
			g_num++;
			int label = 0;
			qDebug("  /// only one default group %d",  label);

		    LabelSurf S;
	        //memset(&S, 0, sizeof(S));

	        S.n = g_num;
			S.label = label;
			S.label2 = label;
			S.name = ""; //strcpy(S.name, "");
			S.on = true; // 090504 RZC
			//S.color;

			listLabelSurf.append(S);

			QList <int> list1Group;
			list1Group.clear();
			for (int i=0; i<listFaceIndex.size(); i++)
			{
				list1Group.append(i+1); // f_num is 1-based
			}
			list2Group.append(list1Group);
		}
		qDebug("-----------read %d lines, %d vertices/normals, %d faces, %d groups/labels", count, v_num, f_num, g_num);
		MESSAGE_ASSERT(v_num==listVertex.size() && f_num==listFaceIndex.size() && g_num==list2Group.size());
		MESSAGE_ASSERT(g_num==listLabelSurf.size());


		PROGRESS_TEXT("Indexing geometric data");
		PROGRESS_PERCENT(90);

		// convert indexed face to group of list of Triangle
			_convertIndexedFace2Triangle(listVertex, listNormal, listFaceIndex, list2Group, list_listTriangle);

	}
	catch(...)
	{
		QMessageBox::critical(0, "loadWavefrontOBJ", QObject::tr("loadWavefrontOBJ: OUT OF MEMEORY."));
	}

	PROGRESS_TEXT("Compiling geometric data");
	PROGRESS_PERCENT(95);

	// create compiled list
		compileLabelfieldSurf();

	qDebug("    Renderer_tex2::loadWavefrontOBJ cost time = %g sec", qtime.elapsed()*0.001);
	lf_busy=false;
}



char V3DS_LOGO[128] = "V3DS1\0";

void Renderer_tex2::saveV3DSurface(const QString& filename)
{
	QFile qf(filename);
	if (! qf.open(QIODevice::WriteOnly) )
	{
		//QMessageBox::critical(0, QObject::tr("open file"), QObject::tr("open file [%1] failed!").arg(filename));
		throw  (const char*)(QObject::tr("open file [%1] failed!").arg(filename).toStdString().c_str());
		return;
	}

	QDataStream qds(&qf);
//#define QF_WRITE( x ) {if (qf.write( (char*)&(x), sizeof(x)) != sizeof(x)) throw "QF_WRITE";}
#define QF_WRITE( x ) {if (qds.writeRawData( (char*)&(x), sizeof(x)) != sizeof(x)) throw "QF_WRITE";}
//#define QF_WRITE( x ) {qds << (x);}

	//////////////////////////////////////////////////////////////////
	// LOGO g_num g_label0 g_label1 t_num Triangle

    qDebug("-------------------------------------------------------");
    int f_num = 0;
    int v_num = 0;

	int i,j;
	for (i=0; i<sizeof(V3DS_LOGO); i++) QF_WRITE( V3DS_LOGO[i] );

	int g_num = list_listTriangle.size();	//group/label number
	QF_WRITE( g_num );

	for (i=0; i<g_num; i++)
	{
		int g_label0 = (mesh_type==0)? listLabelSurf[i] : mesh_iso0;
		int g_label1 = (mesh_type==0)? listLabelSurf[i] : mesh_iso1;
		QF_WRITE( g_label0 );
		QF_WRITE( g_label1 );

		//save display state on 090223
		char b_disp = (listLabelSurf[i].on) ? '1' : '0';
		//qDebug()<<"i="<<i<<"b_disp="<<b_disp;
		QF_WRITE( b_disp );
		//added name and comment on 090220
		char * p_tmpstr=0;
		int len_name = listLabelSurf[i].name.length();
		QF_WRITE( len_name );
		p_tmpstr = (char *)(qPrintable(listLabelSurf[i].name));
		for (j=0;j<len_name;j++) QF_WRITE( p_tmpstr[j] );
		int len_comment = listLabelSurf[i].comment.length();
		QF_WRITE( len_comment );
		p_tmpstr = (char *)(qPrintable(listLabelSurf[i].comment));
		for (j=0;j<len_comment;j++) QF_WRITE( p_tmpstr[j] );

		//now write the triangle data

		Triangle* pT = list_listTriangle[i];
		int t_num = numTriangles(pT);
		QF_WRITE( t_num );

		for (Triangle* p = pT; p != NULL; p = p->next)
		{
			for (int iCorner = 0; iCorner < 3; iCorner++)
			{
				XYZ v, vn;
				v.x = p->vertex[iCorner][0];
				v.y = p->vertex[iCorner][1];
				v.z = p->vertex[iCorner][2];
				vn.x = p->normal[iCorner][0];
				vn.y = p->normal[iCorner][1];
				vn.z = p->normal[iCorner][2];

				QF_WRITE(v.x); QF_WRITE(v.y); QF_WRITE(v.z);
				QF_WRITE(vn.x); QF_WRITE(vn.y); QF_WRITE(vn.z);
			}
			v_num += 3;
		}
		//qDebug("		#%d label(%d-%d) triangle num = %d", i, g_label0, g_label1, t_num);
		f_num += t_num;
	}
	qDebug("---------------------write %d objects, %d faces, %d vertices", list_listTriangle.size(), f_num, v_num);
}

void Renderer_tex2::loadV3DSurface(const QString& filename)
{
//tmp use by Hanchuan for a movie
	int myoffset;
	bool ok1;
	myoffset = QInputDialog::getInteger((QWidget*)widget, "z offset", "z offset:", 0, -1000, 1000, 1, &ok1);
	if (!ok1)
	  myoffset=0;

	ACTIVATE(widget); //to set foucus

//

	QFile qf(filename);
	if (! qf.open(QIODevice::ReadOnly) )
	{
		//QMessageBox::critical(0, QObject::tr("open file"), QObject::tr("open file [%1] failed!").arg(filename));
		throw  (const char*)(QObject::tr("open file [%1] failed!").arg(filename).toStdString().c_str());
		return;
	}

	lf_busy = true;
	PROGRESS_DIALOG("Loading surface        ", widget);
	PROGRESS_PERCENT(1); // 0 or 100 not be displayed. 081102


	QDataStream qds(&qf);
//#define QF_READ( x ) {if (qf.read( (char*)&(x), sizeof(x)) != sizeof(x)) throw "QF_READ";}
#define QF_READ( x ) {if (qds.readRawData( (char*)&(x), sizeof(x)) != sizeof(x)) throw "QF_READ";}
//define QF_READ( x ) { qds >> (x); }

	//////////////////////////////////////////////////////////////////
	// LOGO g_num g_label0 g_label1 t_num Triangle

    qDebug("-------------------------------------------------------");
    int f_num = 0;
    int v_num = 0;

	int i,j;

    char logo[sizeof(V3DS_LOGO)];
	for (i=0; i<sizeof(V3DS_LOGO); i++) QF_READ( logo[i] );	 qDebug("	[%s]", logo);
	bool b_logo = true;
	for (i=0; i<strlen(V3DS_LOGO); i++) b_logo = (b_logo && V3DS_LOGO[i]==logo[i]); // end with \0
	if (! b_logo)
	{
		throw (const char*)"V3DS1: file format not match";
		return;
	}

	////////////////////////////////////////////
	cleanLabelfieldSurf();
	////////////////////////////////////////////


	int g_num;	//group/label number
	QF_READ( g_num );									//qDebug("		g_num= %d", g_num);
	if (g_num==1)
		mesh_type = 1; // range surface
	else
		mesh_type = 0; // label field

	try
	{
		PROGRESS_PERCENT(100); //trick

		for (i=0; i<g_num; i++)		// each group/label
		{

       		PROGRESS_TEXT( QObject::tr("Loading geometric group/label %1 of %2").arg(i+1).arg(g_num) );
    		PROGRESS_PERCENT(90*(i+1)/g_num);

    		LabelSurf S;
	        //memset(&S, 0, sizeof(S)); //cannot simply clear complex class

    	    int g_label0;
			int g_label1;
			QF_READ( g_label0 );					//qDebug("		g_label0= %d", g_label0);
			QF_READ( g_label1 );					//qDebug("		g_label1= %d", g_label1);

			S.n = i+1;
			S.label = g_label0;
			S.label2 = g_label1;

			//read the default display option on 090223
			char b_disp=0;
			QF_READ( b_disp );
			//qDebug()<<"i="<<i<<"b_disp="<<b_disp;
			S.on = (b_disp=='1')? true : false;

			//added name and comment on 090220
			char * p_tmpstr=0;
			int len_name;
			QF_READ( len_name );
			if (len_name<0 || len_name>5000) {throw (const char*)"V3DS1: the surface object name length is invalid (<0 or >5000 letters)";	return;}
			p_tmpstr = new char [len_name+1];
			for (j=0;j<len_name;j++) QF_READ( p_tmpstr[j] );
			p_tmpstr[len_name] = '\0';
			S.name = p_tmpstr;
			if (p_tmpstr) {delete []p_tmpstr; p_tmpstr=0;}

			int len_comment;
			QF_READ( len_comment );
			if (len_comment<0 || len_comment>10000) {throw (const char*)"V3DS1: the surface object comment length is invalid (<0 or >10000 letters)";	return;}
			p_tmpstr = new char [len_comment+1];
			for (j=0;j<len_comment;j++) QF_READ( p_tmpstr[j] );
			p_tmpstr[len_comment] = '\0';
			S.comment = p_tmpstr;
			if (p_tmpstr) {delete []p_tmpstr; p_tmpstr=0;}

			//S.color;

			listLabelSurf.append(S);

			//now read the triangles

			Triangle *pT0, *pT1;
			pT0 = pT1 = 0;
			int t_num;
			QF_READ( t_num );						//qDebug("		group(%d) t_num= %d", i, t_num);

			for (int iFace=0; iFace<t_num; iFace++)		// each face/triangle
			{
				Triangle* pT = new Triangle;
				for (int iConner=0; iConner<3; iConner++)
				{
					XYZ v, vn;
					QF_READ(v.x); QF_READ(v.y); QF_READ(v.z);
					QF_READ(vn.x); QF_READ(vn.y); QF_READ(vn.z);

					pT->vertex[iConner][0] = v.x;
					pT->vertex[iConner][1] = v.y;
					pT->vertex[iConner][2] = v.z+myoffset;
					pT->normal[iConner][0] = vn.x;
					pT->normal[iConner][1] = vn.y;
					pT->normal[iConner][2] = vn.z;
					pT->next = NULL;
				}
				v_num += 3;

				if (iFace == 0)
				{
					pT0 = pT1 = pT;
				}
				else
				{
					pT1->next = pT;
					pT1 = pT;
				}
			}// each face
			f_num += t_num;

			//qDebug("		#%d label(%d-%d) triangle num = %d", i, g_label0, g_label1, t_num);
			MESSAGE_ASSERT(t_num==numTriangles(pT0));
			list_listTriangle.append(pT0);				// one group triangles
		}// each group

		qDebug("---------------------read %d objects, %d faces, %d vertices", list_listTriangle.size(), f_num, v_num);
	}
	catch(...)
	{
		QMessageBox::critical(0, "loadV3DSurface", QObject::tr("loadV3DSurface: OUT OF MEMEORY."));
	}

	PROGRESS_TEXT("Compiling geometric data");
	PROGRESS_PERCENT(95);

	// create compiled list
		compileLabelfieldSurf();

	lf_busy=false;
}


