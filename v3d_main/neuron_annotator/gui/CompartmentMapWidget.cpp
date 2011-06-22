#include "CompartmentMapWidget.h"
#include "v3d_core.h"
#include "../3drenderer/Renderer_gl2.h"
#include <iostream>
#include <cmath>
#include <cassert>

using namespace std;

CompartmentMapWidget::CompartmentMapWidget(QWidget* parent): V3dR_GLWidget(NULL, parent, "Title")
{
    _idep = new iDrawExternalParameter();
    _idep->image4d = NULL;
    _volCompress = false;

    // This method for eliminating tearing artifacts works but is supposedly obsolete;
    // http://stackoverflow.com/questions/5174428/how-to-change-qglformat-for-an-existing-qglwidget-at-runtime
    QGLFormat glFormat(context()->format());
    glFormat.setDoubleBuffer(true); // attempt to reduce tearing on Mac
    setFormat(glFormat);
    
    // cleanLabelfieldSurf
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
    
    //
    update = 0;

}

CompartmentMapWidget::~CompartmentMapWidget()
{
    delete _idep; _idep = NULL;
}

void CompartmentMapWidget::loadV3DSFile(const QString& filename)
{

	QFile qf(filename);
	if (! qf.open(QIODevice::ReadOnly) )
	{
		throw  (const char*)(QObject::tr("open file [%1] failed!").arg(filename).toStdString().c_str());
		return;
	}
    
	QDataStream qds(&qf);
#define QF_READ( x ) {if (qds.readRawData( (char*)&(x), sizeof(x)) != sizeof(x)) throw "QF_READ";}
    
    int f_num = 0;
    int v_num = 0;
    
	int i,j;
    
    int mesh_type;
	int g_num;	//group/label number
	QF_READ( g_num );
	if (g_num==1)
		mesh_type = 1; // range surface
	else
		mesh_type = 0; // label field
    
	try
	{        
		for (i=0; i<g_num; i++)		// each group/label
		{            
    		LabelSurf S;
            
    	    int g_label0;
			int g_label1;
			QF_READ( g_label0 );
			QF_READ( g_label1 );
            
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
			QF_READ( t_num );
            
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
					pT->vertex[iConner][2] = v.z;
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
            
			MESSAGE_ASSERT(t_num==numTriangles(pT0));
			list_listTriangle.append(pT0);				// one group triangles
		}// each group
        
	}
	catch(...)
	{
		QMessageBox::critical(0, "loadV3DSFile", QObject::tr("loadV3DSFile: OUT OF MEMEORY."));
	}
    
	// create compiled list -- compileLabelfieldSurf
    makeCurrent(); //ensure right context when multiple views animation or mouse drop, 081118
    
	int num_surf = list_listTriangle.size();
	if (update==0) // initial color & on
	{
        
		for (int i=0; i<listLabelSurf.size(); i++) // Label Surface: set random color
		{
			LabelSurf& S = listLabelSurf[i];
			S.color = random_rgba8(255);
			S.on = true; //commented on 090223 by PHC. to enable hiding some surface obj by default
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
        
        update++;
	}
    
	//
	for (int i=0; i<list_glistLabel.size(); i++)
		glDeleteLists(list_glistLabel[i], 1);
	list_glistLabel.clear();
    
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

