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




//separated from v3d_core.cpp by Hanchuan Peng, 090819

#include <stdio.h>
#include <QtGui>

#include "v3d_core.h"
#include "mainwindow.h"

#include "import_images_tool_dialog.h"

#include "../io/io_bioformats.h"

bool XFormWidget::importGeneralImageFile(QString filename)
{
	if (!filename.isEmpty())
	{
		openFileNameLabel = filename;

		TimePackType timepacktype;
		QStringList mylist = importSeriesFileList_addnumbersort(filename, timepacktype);

		return importGeneralImgSeries(mylist, timepacktype);
	}
	else return false;
}

bool XFormWidget::importGeneralImgSeries(const QStringList & mylist, TimePackType timepacktype)
{
	//foreach (QString qs, myList)  qDebug() << qs;
	V3DLONG ntime = mylist.size();
	if (ntime <1)
	{
		v3d_msg("The import list is empty. do nothing.\n");
		return false;
	}

	//if there are files to import, then clean data

	if (imgData)  cleanData();
  	imgData = new My4DImage;
	if (!imgData)  return false;

	//now we can simply read files one by one, and arrange them in term of (1) the color channel and (2) z-planes. At this stage we need to verify each plane has the same size, and NOT colored image!!

    V3DLONG nsz0=0, nsz1=0;
    ImagePixelType cur_datatype=V3D_UNKNOWN, ndatatype=V3D_UNKNOWN;
    V3DLONG ncolors=0, nthick=0;
    V3DLONG pack_color=0, pack_z=0;

    for (V3DLONG i = 0; i < ntime; ++i)
	{
        QString tmpfileInfo = mylist.at(i);
        printf("importing %i file: {%s}\n", i, qPrintable(tmpfileInfo));

		unsigned char * cur_data1d=0;
		V3DLONG * cur_sz=0;
		if (!readSingleImageFile(tmpfileInfo.toUtf8().data(), cur_data1d, cur_sz, cur_datatype))
		{
			v3d_msg("Error occurs in reading the file. Exit importing.\n");
			return false;
		}
		if (!cur_data1d || !cur_sz)
		{
			v3d_msg("Error occurs in reading the file. Exit importing.\n");
			return false;
		}

		//-----------------------------------------------------------------------
		// 090731 RZC: (3D time series --> 4D color image) packed time in Z dim.
		//-----------------------------------------------------------------------

		if (i==0) ncolors = cur_sz[3];
		if (cur_sz[3]<=0 || cur_sz[3]!=ncolors)
		{
			printf("The current file has invalid or different colors [=%ld] from first section [=%ld]. Exit importing.\n", cur_sz[3], ncolors);
			v3d_msg("The current file has invalid or different colors\n");
			if (cur_data1d) {delete []cur_data1d; cur_data1d=0;}
			if (cur_sz) {delete []cur_sz; cur_sz=0;}
			return false;
		}
		if (i==0) ndatatype = cur_datatype;
		if (cur_datatype != ndatatype)
		{
			printf("The current file has different data type [=%ld] from first section [=%ld]. Exit importing.\n", cur_datatype, ndatatype);
			v3d_msg("The current file has different data type\n");
			if (cur_data1d) {delete []cur_data1d; cur_data1d=0;}
			if (cur_sz) {delete []cur_sz; cur_sz=0;}
			return false;
		}


		if (i==0)
		{
			nsz0 = cur_sz[0]; nsz1 = cur_sz[1]; nthick = cur_sz[2];
			if (timepacktype==TIME_PACK_Z)
			{
				pack_z     = nthick*ntime;
				pack_color = ncolors;
			}
			else // TIME_PACK_C
			{
				pack_z     = nthick;
				pack_color = ncolors*ntime;
			}

			if (imgData->createImage(nsz0, nsz1, pack_z, pack_color, cur_datatype)==false)
			{
				v3d_msg("Fail to allocate memory for the image stack. Exit importing.\n");
				return false;
			}
			imgData->setTDim( ntime );
			imgData->setTimePackType( timepacktype );
		}
		else
		{
			if (cur_sz[0]!=nsz0 || cur_sz[1]!=nsz1 || cur_sz[2]!=nthick)
			{
				printf("The current image has a different [width, height, thick]=[%ld, %ld, %ld] from the first section [%ld, %ld, %ld]. Exit importing.\n",
					   cur_sz[0], cur_sz[1], cur_sz[2], nsz0, nsz1, nthick);
				v3d_msg("The current image has a different [width, height, thick]\n");
				if (cur_data1d) {delete []cur_data1d; cur_data1d=0;}
				if (cur_sz) {delete []cur_sz; cur_sz=0;}
				return false;
			}
		}

		//now copy data of different planes into the 5D stack
		V3DLONG element_bytes = imgData->getUnitBytes();
		V3DLONG cur_time = i;
		V3DLONG block_size = (nthick*nsz0*nsz1)*(element_bytes);
		for (V3DLONG cur_ch=0; cur_ch<ncolors; cur_ch++)
		{
			unsigned char * cur_data1d_block = cur_data1d + (cur_ch)*block_size;
			unsigned char * cur_target1d_block;
			if (timepacktype==TIME_PACK_Z)
			{
				cur_target1d_block = imgData->getRawData() + (cur_ch*ntime + cur_time)*block_size;
			}
			else
			{
				cur_target1d_block = imgData->getRawData() + (cur_ch + cur_time*ncolors)*block_size;
			}

			//for (V3DLONG j=0; j<(block_size); j++)   cur_target1d_block[j] = cur_data1d_block[j];
			memcpy(cur_target1d_block, cur_data1d_block, block_size);
		}

		//now delete the temporary image data
		if (cur_data1d) {delete []cur_data1d; cur_data1d=0;}
		if (cur_sz) {delete []cur_sz; cur_sz=0;}

    }
    printf("Finished importing data. Now img data size = [%ld, %ld, %ld, %ld]\n", imgData->getXDim(), imgData->getYDim(), imgData->getZDim(), imgData->getCDim());
	if (imgData->getTDim()>1 && imgData->getTimePackType()==TIME_PACK_Z)
	{
		printf("Packed time point in [z = %ld * %ld]\n", imgData->getTDim(), imgData->getZDim()/imgData->getTDim());
	}
	if (imgData->getTDim()>1 && imgData->getTimePackType()==TIME_PACK_C)
	{
		printf("Packed time point in [c = %ld * %ld]\n", imgData->getTDim(), imgData->getCDim()/imgData->getTDim());
	}

	//now create all the 4D pointers, etc
	imgData->setupData4D();

	openFileNameLabel = openFileNameLabel + "_import.tif";
	imgData->setFileName((char *)qPrintable(openFileNameLabel));

    if (imgData->getCDim()>=3)
	    Ctype = colorRGB;
    else if (imgData->getCDim()==2)
	    Ctype = colorRG;
	else //==1
	    Ctype = colorRed2Gray;

    imgData->setFlagLinkFocusViews(bLinkFocusViews);
    imgData->setFlagDisplayFocusCross(bDisplayFocusCross);

    //imgData->setFlagImgValScaleDisplay((imgValScaleDisplayCheckBox->checkState()==Qt::Checked) ? true : false); //disabled this 100814, PHC

	//now set the disp_zoom. 081114

	if (imgData->getXDim()>512 || imgData->getYDim()>512 || imgData->getZDim()>512)
	{
		disp_zoom= double(512) / qMax(imgData->getXDim(), qMax(imgData->getYDim(), imgData->getZDim()));
		b_use_dispzoom=true;
	}


	//now set the orientation
	if (getMainControlWindow()->global_setting.b_yaxis_up)
	{
		getImageData()->flip(axis_y);
	}


	// update the interface

    updateDataRelatedGUI();

	return true;
}


QStringList importSeriesFileList_addnumbersort(const QString & individualFileName, TimePackType & packtype)
{
	QStringList myList;
	myList.clear();

	//Get the image files namelist in the directory

	QFileInfo fileInfo(individualFileName);
	QString curFilePath = fileInfo.path();
        QString curSuffix = fileInfo.suffix();

	QDir dir(curFilePath);
	if (!dir.exists())
	{
		qWarning("Cannot find the directory");
		return myList;
	}

	QStringList imgfilters;
        imgfilters.append("*." + curSuffix);
	foreach (QString file, dir.entryList(imgfilters, QDir::Files, QDir::Name))
	{
        myList += QFileInfo(dir, file).absoluteFilePath();
	}

	//sort image sequence by numeric order instead of alphanumeric order
	//e.g. "a9.tiff" "a10.tiff" "b1.tiff"
	QStringList sortedList, tmpList;

	//-----------------------------------------------------------------------
	// 090731 RZC: fixed numerically sorting file names list, for XFormWidget::importGeneralImgSeries
	//-----------------------------------------------------------------------
	QString fileNameStr, fileNameDigits;	//absolute file name is separated to 2 parts: strings and digits
        QRegExp r("(\\d+)");		//find digits
	QMap<V3DLONG, QString> mapList;

	mapList.clear();
    for(V3DLONG i=0; i<myList.size(); ++i)
	{
        fileNameStr = myList.at(i);
        QFileInfo fileFullName(myList.at(i));
        QString fileFullNameStr = fileFullName.completeBaseName();


		//extract the fileNameDigits from fileNameStr
		//e.g. "a9_b2009051801.tif.raw" into "a9_b.tif.raw" & "2009051801"

		V3DLONG pos = 0;
		fileNameDigits = "";
        while ((pos = r.indexIn(fileFullNameStr, pos)) != -1)
		{
                    fileNameDigits = r.cap(1);
                    pos += r.matchedLength();
		}

		if (fileNameDigits.isEmpty()) continue;


        V3DLONG num = fileNameDigits.toULong();
		mapList.insert(num, fileNameStr);
	}
	// must be sorted by QMap
	myList = mapList.values();
	//foreach (QString qs, myList)  qDebug() << qs;

    //no need to pop-out a dialog if no meaningful file has been detected. 131017
    if (myList.isEmpty())
    {
        v3d_msg("It seems no file contains a digit-portion in the file name. Naming convention should be sth like xxx_000001.yyy, xxx_000002.yyy, .... Check your data before importing.");
        return myList;
    }

	//Get the tiff image sequence by usr interaction

	ImportImgPara p;
	p.countImg = myList.size();

	import_images_tool_Dialog d(curFilePath);

    //need to update the information based on the current myList info. 131017
    d.numimgBox->setMaximum(p.countImg);
    d.numimgBox->setValue(p.countImg);
    d.numimgBox->setMinimum(p.countImg);

    d.startimgBox->setMaximum(p.countImg);
    d.startimgBox->setValue(1);

    d.incBox->setMaximum(p.countImg);

    d.endimgBox->setMaximum(p.countImg);
    d.endimgBox->setValue(p.countImg);

	int res = d.exec();
	if (res==QDialog::Accepted)
	{
		d.fetchData(&p);

		//get the QStringList
		myList = myList.filter(p.filt);

		tmpList.clear();
        for (V3DLONG i = p.startImg-1; i<= p.endImg-1; i+=p.inc)
			tmpList << myList.at(i);//.toLocal8Bit().constData();

        myList = tmpList;
        packtype = (p.packType==0)? TIME_PACK_Z : TIME_PACK_C;
	}
	else
	{
		myList.clear();
	}

	return myList;
}

bool XFormWidget::importLeicaFile(QString filename)
{
	if (!filename.isEmpty())
	{
		openFileNameLabel = filename;
		return importLeicaData();
	}
	else
		return false;
}

bool XFormWidget::importLeicaData()
{
    if (imgData)
	{
		cleanData();
	}

  	imgData = new My4DImage;
	if (!imgData)
		return false;

	int i;

	//split to get the file name
	QFileInfo fileInfo(openFileNameLabel);
	QString curFilePath = fileInfo.path();
	QString curFileName = fileInfo.fileName();
	QString curFileBase = fileInfo.baseName();
	QString curFileSuffix = fileInfo.suffix();
	QDir curFileDir = fileInfo.dir();

	//further split
	QStringList mylist = curFileBase.split(QRegExp("_"));
	for (i = 0; i < mylist.size(); ++i)
		printf(" [ %s ]\n", mylist.at(i).toLocal8Bit().constData());
	printf("\n");

	//analyze the file name convention
	if (mylist.at(mylist.count()-2).startsWith("z", Qt::CaseInsensitive)==false ||
	    mylist.at(mylist.count()-1).startsWith("ch", Qt::CaseInsensitive)==false ||
		curFileSuffix.startsWith("tif", Qt::CaseInsensitive)==false)
	{
		v3d_msg("This file does not have a legal 'z'-'ch' labels or has no tif suffix. Thus does not like a Leica tif series. Cannot import.");
		return false;
	}

	//then list all files
	QString curSlice, curCh;

	curFileDir.setNameFilters(QStringList("*.tif*"));
	curFileDir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    curFileDir.setSorting(QDir::Name);

    QFileInfoList filist = curFileDir.entryInfoList();
    printf("     filenames\n");
    for (i = 0; i < filist.size(); ++i)
	{
        QFileInfo tmpfileInfo = filist.at(i);
        printf("%s\n", qPrintable(tmpfileInfo.fileName()));
    }

	//then produce file names:
	//first detect the lower and upper bounds of z planes and color channels
	int zplane0, zplane1, c0, c1; bool ok;
	mylist = filist.at(0).baseName().split(QRegExp("_"));
	if (mylist.at(mylist.count()-2).startsWith("z", Qt::CaseInsensitive)==false ||
	    mylist.at(mylist.count()-1).startsWith("ch", Qt::CaseInsensitive)==false)
	{
		v3d_msg(QString("This file [%1] does not have a legal 'z'-'ch' labels. Cannot import.").arg(filist.at(0).baseName()));
		return false;
	}
	else
	{
		QString tmpstr = mylist.at(mylist.count()-2);
		zplane0 = tmpstr.remove(0,1).toInt(&ok);
		if (!ok)
		{
			v3d_msg(QString("The file [%1] does not contain a valid number index [zplane0=%2]. Cannot import.").arg(filist.at(0).baseName()).arg(zplane0));
			return false;
		}

		tmpstr = mylist.at(mylist.count()-1);
		printf("c0str=[%s]\n", tmpstr.toUtf8().data());
		c0 = tmpstr.remove(0,2).toInt(&ok);
		if (!ok)
		{
			v3d_msg(QString("The file [%1] does not contain a valid color channel index [c0=%2]. Cannot import.").arg(filist.at(0).baseName()).arg(c0));
			return false;
		}

	}

	mylist = filist.at(filist.count()-1).baseName().split(QRegExp("_"));
	if (mylist.at(mylist.count()-2).startsWith("z", Qt::CaseInsensitive)==false ||
	    mylist.at(mylist.count()-1).startsWith("ch", Qt::CaseInsensitive)==false)
	{
		v3d_msg(QString("This file [%1] does not have a legal 'z'-'ch' labels. Cannot import.").arg(filist.at(filist.count()-1).baseName()));
		return false;
	}
	else
	{
		QString tmpstr = mylist.at(mylist.count()-2);
		zplane1 = tmpstr.remove(0,1).toInt(&ok);
		if (!ok)
		{
			v3d_msg(QString("The file [%1] does not contain a valid number index [zplane1=%2]. Cannot import.").arg(filist.at(filist.count()-1).baseName()).arg(zplane1));
			return false;
		}

		tmpstr = mylist.at(mylist.count()-1);
		printf("c0str=[%s]\n", tmpstr.toUtf8().data());
		c1 = tmpstr.remove(0,2).toInt(&ok);
		if (!ok)
		{
			v3d_msg(QString("The file [%1] does not contain a valid color channel index [c1=%2]. Cannot import.").arg(filist.at(filist.count()-1).baseName()).arg(c1));
			return false;
		}
	}

	//then verify the number of files
	V3DLONG nzplanes = zplane1-zplane0+1;
	V3DLONG ncolors = c1-c0+1;
	if (nzplanes<=0 || ncolors<=0 || nzplanes*ncolors!=filist.size())
	{
		v3d_msg(QString("The number of files does not match the product of #zplanes (=%1) and #color-channels (=%2). Cannot import.\n").arg(nzplanes).arg(ncolors));
		return false;
	}

	v3d_msg(QString("Finish pre-scanning. <br><br>To successfully import the Leica tif series, there should be [%1] z-planes and [%2] color channel(s). <br><br>Also this importer assumes the data is 8-bit. <br><br> If for any reason your data is not consistent with these numbers, your importing may fail.").arg(nzplanes).arg(ncolors));

	//now we can simply read files one by one, and arrange them in term of (1) the color channel and (2) z-planes. At this stage we need to verify each plane has the same size, and NOT colored image!!

    V3DLONG curimgwid, curimghei; ImagePixelType cur_datatype;
	V3DLONG * user_specified_ch = new V3DLONG [ncolors];
	bool * b_continue_use_user_specified_ch = new bool [ncolors];
    for (i = 0; i < filist.size(); ++i)
	{
        QFileInfo tmpfileInfo = filist.at(i);
		QString tmpstr = curFilePath + "/" + tmpfileInfo.fileName();
        printf("importing %i file: {%s}\n", i, qPrintable(tmpstr));

		unsigned char * cur_data1d=0;
		V3DLONG * cur_sz=0;
		if (!readSingleImageFile(tmpstr.toUtf8().data(), cur_data1d, cur_sz, cur_datatype))
		{
			v3d_msg(QString("Error occurs in reading the %1 file. Exit importing.").arg(i));
			return false;
		}

		if (!cur_data1d || !cur_sz || cur_sz[2]!=1)
		{
			v3d_msg(QString("The current file (%1 image) is not a 2D image. Exit importing.").arg(i));
			if (cur_data1d) {delete []cur_data1d; cur_data1d=0;}
			if (cur_sz) {delete []cur_sz; cur_sz=0;}
			return false;
		}
		if (cur_sz[3]!=1 && cur_sz[3]!=ncolors)
		{
			bool ok;
			if (!b_continue_use_user_specified_ch[i%ncolors])
			{

#if defined(USE_Qt5)
				user_specified_ch[i%ncolors] = QInputDialog::getInt(0, tr("Specify a channel"),
																	QString("The current file [%1] has [%2] colors which do not match the presumed number of colors/channels [=%3]. <br><br>Please specify a channel of this image you want to import data (start from 0):").arg(tmpstr).arg(cur_sz[3]).arg(ncolors),
																	int(i%ncolors), 0, cur_sz[3]-1, 1, &ok);
#else
				user_specified_ch[i%ncolors] = QInputDialog::getInteger(0, tr("Specify a channel"),
																		QString("The current file [%1] has [%2] colors which do not match the presumed number of colors/channels [=%3]. <br><br>Please specify a channel of this image you want to import data (start from 0):").arg(tmpstr).arg(cur_sz[3]).arg(ncolors),
																		int(i%ncolors), 0, cur_sz[3]-1, 1, &ok);
#endif
				if (ok)
				{

#if defined(USE_Qt5)
					b_continue_use_user_specified_ch[i%ncolors] = QInputDialog::getInt(0, tr("Question"),
																						QString("Do you want to continue using the channel just specified for <br>importing for all remaining images in this series that have conflict? (0 for No, 1 for Yes)"),
																						1, 0, 1, 1, &ok);
#else
					b_continue_use_user_specified_ch[i%ncolors] = QInputDialog::getInteger(0, tr("Question"),
																						   QString("Do you want to continue using the channel just specified for <br>importing for all remaining images in this series that have conflict? (0 for No, 1 for Yes)"),
																						   1, 0, 1, 1, &ok);
#endif
				}
			}
			else
			{
				user_specified_ch[i%ncolors] = -1;
			}

		}

		if (i==0)
		{
			curimgwid = cur_sz[0]; curimghei = cur_sz[1];
			if (imgData->createImage(cur_sz[0], cur_sz[1], nzplanes, ncolors, cur_datatype)==false)
			{
				v3d_msg("Fail to allocate memory for the image stack. Exit importing.");
				return false;
			}
		}
		else
		{
			if (cur_sz[0]!=curimgwid || cur_sz[1]!=curimghei)
			{
				v3d_msg(QString("The current image %1 has a different [width, height]=[%2, %3] from the first section [%4, %5]. Exit importing.").arg(i).arg(cur_sz[0]).arg(cur_sz[1]).arg(curimgwid).arg(curimghei));
				if (cur_data1d) {delete []cur_data1d; cur_data1d=0;}
				if (cur_sz) {delete []cur_sz; cur_sz=0;}
				return false;
			}
		}

		//now copy data of different z planes into the stack
		V3DLONG cur_page = i/ncolors;
		V3DLONG cur_ch = i%ncolors;
		if (user_specified_ch[i%ncolors]>=0 && user_specified_ch[i%ncolors]<=cur_sz[3]-1)
		{
			cur_ch = user_specified_ch[i%ncolors];
		}
		unsigned char * cur_data1d_datapage=0;
		if (cur_sz[3]==1)
			cur_data1d_datapage = cur_data1d;
		else
			cur_data1d_datapage = cur_data1d + cur_ch*curimgwid*curimghei;
		unsigned char * cur_target1d = imgData->getRawData()+(V3DLONG(i%ncolors)*nzplanes + cur_page)*curimgwid*curimghei;
		for (V3DLONG j=0;j<curimgwid*curimghei;j++)
		{
			cur_target1d[j] = cur_data1d_datapage[j];
		}

		//now delete the temporary image data
		if (cur_data1d) {delete []cur_data1d; cur_data1d=0;}
		if (cur_sz) {delete []cur_sz; cur_sz=0;}

    }
	if (user_specified_ch) {delete []user_specified_ch; user_specified_ch=0; }
	if (b_continue_use_user_specified_ch) {delete []b_continue_use_user_specified_ch; b_continue_use_user_specified_ch=0;}
	v3d_msg(QString("Finished importing data. Now img data size = [%1, %2, %3, %4]").arg(imgData->getXDim()).arg(imgData->getYDim()).arg(imgData->getZDim()).arg(imgData->getCDim()));

	//now create all the 4D pointers, etc
	imgData->setupData4D();

	openFileNameLabel = openFileNameLabel + "_import.tif";
	imgData->setFileName((char *)qPrintable(openFileNameLabel));

    if (imgData->getCDim()>=3)
	    Ctype = colorRGB;
    else if (imgData->getCDim()==2)
	    Ctype = colorRG;
	else //==1
	    Ctype = colorRed2Gray;

    imgData->setFlagLinkFocusViews(bLinkFocusViews);
    imgData->setFlagDisplayFocusCross(bDisplayFocusCross);

    //imgData->setFlagImgValScaleDisplay((imgValScaleDisplayCheckBox->checkState()==Qt::Checked) ? true : false); //disabled this 100814, PHC

	//now set the disp_zoom. 081114

	if (imgData->getXDim()>512 || imgData->getYDim()>512 || imgData->getZDim()>512)
	{
		disp_zoom= double(512) / qMax(imgData->getXDim(), qMax(imgData->getYDim(), imgData->getZDim()));
		b_use_dispzoom=true;
	}

	// update the interface

    updateDataRelatedGUI();

	return true;
}


bool readSingleImageFile(char *imgSrcFile, unsigned char * & data1d, V3DLONG * & sz, ImagePixelType & datatype)
{
    datatype = V3D_UNKNOWN;
    int dt = 0;
    if (loadImage(imgSrcFile, data1d, sz,  dt))
    {
        if (dt==1) datatype = V3D_UINT8;
        else if (dt==2) datatype = V3D_UINT16;
        else if (dt==4) datatype = V3D_FLOAT32;
        return true;
    }
    else //use Bioformats IO plugin
    {
        QString outfilename;
        if(!call_bioformats_io(imgSrcFile, outfilename))
            return false;

        if (loadImage((char *)qPrintable(outfilename), data1d, sz,  dt))
        {
            if (dt==1) datatype = V3D_UINT8;
            else if (dt==2) datatype = V3D_UINT16;
            else if (dt==4) datatype = V3D_FLOAT32;
            return true;
        }
        else
            return false;
    }

    return false;
}


