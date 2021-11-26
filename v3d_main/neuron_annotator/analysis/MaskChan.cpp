#include "MaskChan.h"

const int MaskChan::MAX_LABEL=65535;

MaskChan::MaskChan()
{
  sourceImage=0L;
  labelImage=0L;
  label8=0L;
  label16=0L;
}

bool MaskChan::setSourceImage(My4DImage *image)
{
  sourceImage=image;
  if (sourceImage==0) {
    return false;
  }
  xdim=sourceImage->getXDim();
  ydim=sourceImage->getYDim();
  zdim=sourceImage->getZDim();
  cdim=sourceImage->getCDim();
  return true;
}

bool MaskChan::setLabelImage(My4DImage *image)
{
  labelImage=image;
  if (labelImage==0) {
    return false;
  }
  if (labelImage->getXDim()!=xdim) {
    qDebug() << "source and label xdim do not match";
    return false;
  }
  if (labelImage->getYDim()!=ydim) {
    qDebug() << "source and label ydim do not match";
    return false;
  }
  if (labelImage->getZDim()!=zdim) {
    qDebug() << "source and label zdim do not match";
    return false;
  }

  if (labelImage->getDatatype()==V3D_UINT8) {
    label8=(v3d_uint8*)(labelImage->getRawDataAtChannel(0));
  } else {
    label16=(v3d_uint16*)(labelImage->getRawDataAtChannel(0));
  }

  labelIndex=new long[MAX_LABEL];
  for (int i=0;i<MAX_LABEL;i++) {
    labelIndex[i]=0;
  }

  for (long z=0;z<zdim;z++) {
    for (long y=0;y<ydim;y++) {
      for (long x=0;x<xdim;x++) {
	long offset=z*ydim*xdim + y*xdim + x;
	int labelValue=-1;
	if (label8 != 0L) {
	  labelValue=label8[offset];
	} else {
	  labelValue=label16[offset];
	}
	if (labelValue>=0) {
	  labelIndex[labelValue]++;
	}
      }
    }
  }

  for (int i=1;i<MAX_LABEL;i++) { // ignore 0
    if (labelIndex[i]>0) {
      labelList.append(i);
      qDebug() << "Found label=" << i;
    }
  }

  return true;
}

QList<int> MaskChan::getFragmentListFromLabelStack()
{
  qDebug() << "getFragmentListFromLabelStack zdim=" << zdim << " ydim=" << ydim << " xdim=" << xdim;
  QList<int> labelList;
  long* checkList=new long[256*256]; // 16-bit
  for (int i=0;i<256*256;i++) {
    checkList[i]=0;
  }
  v3d_uint8* label8=0L;
  v3d_uint16* label16=0L;
  if (labelImage->getDatatype()==V3D_UINT8) {
    label8=labelImage->getRawDataAtChannel(0);
  } else {
    label16=(v3d_uint16*)labelImage->getRawDataAtChannel(0);
  }
  for (long z=0;z<zdim;z++) {
    long zOffset=z*ydim*xdim;
    for (long y=0;y<ydim;y++) {
      long yOffset=y*xdim;
      for (long x=0;x<xdim;x++) {
	long offset=zOffset+yOffset+x;
	int labelValue=0;
	if (label8 != 0L) {
	  labelValue=label8[offset];
	} else {
	  labelValue=label16[offset];
	}
	if (labelValue>0) {
	  checkList[labelValue] += 1;
	}
      }
    }
  }
  for (int i=0;i<256*256;i++) {
    if (checkList[i]>0) {
      qDebug() << "Found " << checkList[i] << " voxels for label=" << i;
      labelList.append(i);
    }
  }
  delete [] checkList;
  return labelList;
}

bool MaskChan::createMaskChanForLabel(int label, const QString& maskFullPath, const QString& channelFullPath, QReadWriteLock* mutex)
{
  QList<long> pairCountList; // x=0, y=1, z=2
  QList<MaskRay*> xRayList;
  QList<MaskRay*> yRayList;
  QList<MaskRay*> zRayList;

  QList<long> x0List;
  QList<long> x1List;
  QList<long> y0List;
  QList<long> y1List;
  QList<long> z0List;
  QList<long> z1List;

  for (int direction=0;direction<3;direction++) {

    QList<MaskRay*> * rayList;
    long pairCount=0L;
    long countCheck=0L;

    if (direction==0) {
      rayList=&xRayList;
    } else if (direction==1) {
      rayList=&yRayList;
    } else {
      rayList=&zRayList;
    }

    long x0,x1,y0,y1,z0,z1;

    axisTracer(direction, label, rayList, pairCount, countCheck, x0, x1, y0, y1, z0, z1);

    x0List.append(x0);
    x1List.append(x1);
    y0List.append(y0);
    y1List.append(y1);
    z0List.append(z0);
    z1List.append(z1);

    pairCountList.append(pairCount);
    pairCount=0L;

    if (countCheck!=labelIndex[label]) {
      qDebug() << "Count check failed : direction=" << direction << " countCheck=" << countCheck << " labelIndex=" << labelIndex[label];
      return false;
    } else {
      //qDebug() << "Direction " << direction << " passed voxel count check";
    }

  }

  // We have computed the ray set for each axis, we will save the one with the
  // smallest size.

  for (int s=0;s<pairCountList.size();s++) {
    //qDebug() << "pairCount " << s << " : " << pairCountList[s];
  }

  unsigned char smallestSize=0;
  if (pairCountList[1]<pairCountList[smallestSize]) {
    smallestSize=1;
  }
  if (pairCountList[2]<pairCountList[smallestSize]) {
    smallestSize=2;
  }

  long x0=x0List[smallestSize];
  long x1=x1List[smallestSize];
  long y0=y0List[smallestSize];
  long y1=y1List[smallestSize];
  long z0=z0List[smallestSize];
  long z1=z1List[smallestSize];

  long totalVoxels=labelIndex[label];

  // allocate space for data
  long unitsNeeded=totalVoxels*cdim;
  void* data=0L;
  if (sourceImage->getDatatype()==V3D_UINT8) {
    data=(void*) new v3d_uint8[unitsNeeded];
  } else {
    data=(void*) new v3d_uint16[unitsNeeded];
  }

  // re-run axis-tracer and populate channel intensity data
  QList<MaskRay*> * rayList;
  long pairCount=0L;
  long countCheck=0L;

  if (smallestSize==0) {
    rayList=&xRayList;
  } else if (smallestSize==1) {
    rayList=&yRayList;
  } else {
    rayList=&zRayList;
  }

  //qDebug() << "calling axisTracer 2nd pass data=" << data;

  rayList->clear(); // will get re-populated

  axisTracer(smallestSize, label, rayList, pairCount, countCheck, x0, x1, y0, y1, z0, z1, data, totalVoxels);

  if (countCheck!=totalVoxels) {
    qDebug() << "In second pass of axisTracer, countCheck=" << countCheck << " totalVoxels=" << totalVoxels;
    exit(1);
  }

  //qDebug() << "Using axis " << smallestSize;

  //qDebug() << "Writing to file and locking with QMutex" << maskFullPath;

  if (mutex!=0L) {
    QWriteLocker locker(mutex); // Will be deleted from stack
  }

  QFile maskFile(maskFullPath);


  ////////////////////////////////////////////////////////////////////////

  //    FILE* fid = fopen(maskFullPath.toAscii().data(), "wb");
  //    if (!fid) {
  //        qDebug() << "Could not open file " << maskFullPath << " to write";
  //        return false;
  //    }

  maskFile.open( QIODevice::WriteOnly );
  QDataStream maskOut(&maskFile);

  //qDebug() << "Writing xdim=" << xdim << " ydim=" << ydim << " zdim=" << zdim;

  maskOut.writeRawData( (const char*) &xdim, sizeof(long));
  maskOut.writeRawData( (const char *)&ydim, sizeof(long));
  maskOut.writeRawData( (const char *)&zdim, sizeof(long));

  float xMicrons=0.0;
  float yMicrons=0.0;
  float zMicrons=0.0;

  maskOut.writeRawData( (const char *)&xMicrons, sizeof(float));
  maskOut.writeRawData( (const char *)&yMicrons, sizeof(float));
  maskOut.writeRawData( (const char *)&zMicrons, sizeof(float));

  maskOut.writeRawData( (const char *)&x0, sizeof(long));
  maskOut.writeRawData( (const char *)&x1, sizeof(long));
  maskOut.writeRawData( (const char *)&y0, sizeof(long));
  maskOut.writeRawData( (const char *)&y1, sizeof(long));
  maskOut.writeRawData( (const char *)&z0, sizeof(long));
  maskOut.writeRawData( (const char *)&z1, sizeof(long));

  maskOut.writeRawData( (const char *)&totalVoxels, sizeof(long));
  maskOut.writeRawData( (const char *)&smallestSize, sizeof(unsigned char));

  if (smallestSize==0) {
    writeMaskList(maskOut, xRayList);
  } else if (smallestSize==1) {
    writeMaskList(maskOut, yRayList);
  } else {
    writeMaskList(maskOut, zRayList);
  }
  //    fflush(fid);
  //    fclose(fid);
  //    fid=0L;
  maskFile.close();

  qDebug() << "Wrote " << maskFullPath;

  // Write out the channel file

  // First, clear the previous masks
  for (int d=0;d<3;d++) {
    QList<MaskRay*> * rayList;
    if (d==0) {
      rayList=&xRayList;
    } else if (d==1) {
      rayList=&yRayList;
    } else {
      rayList=&zRayList;
    }
    for (int i=0;i<rayList->size();i++) {
      MaskRay* ray = (*rayList)[i];
      delete ray;
    }
    rayList->clear();
  }

  QFile channelFile(channelFullPath);
  channelFile.open( QIODevice::WriteOnly );

  //qDebug() << "Writing to file " << channelFullPath;

  //    fid = fopen(channelFullPath.toAscii().data(), "wb");
  //    if (!fid) {
  //        qDebug() << "Could not open file " << channelFullPath << " to write";
  //        return false;
  //    }

  QDataStream chanOut(&channelFile);

  chanOut.writeRawData( (const char *)&totalVoxels, sizeof(long));
  unsigned char numChannels=sourceImage->getCDim();
  chanOut.writeRawData( (const char *)&numChannels, sizeof(unsigned char));

  unsigned char recRed=0;
  unsigned char recGreen=1;
  unsigned char recBlue=2;

  chanOut.writeRawData( (const char *)&recRed, sizeof(unsigned char));
  chanOut.writeRawData( (const char *)&recGreen, sizeof(unsigned char));
  chanOut.writeRawData( (const char *)&recBlue, sizeof(unsigned char));

  unsigned char datatype=1; // 8-bit
  if (sourceImage->getDatatype()==V3D_UINT16) {
    datatype=2; // 16-bit
  }
  chanOut.writeRawData( (const char *)&datatype, sizeof(unsigned char));


  chanOut.writeRawData( (const char *)data, totalVoxels*cdim*datatype);
  //    fflush(fid);
  //    fclose(fid);
  channelFile.close();

  qDebug() << "Wrote " << channelFullPath;
  return true;
}

void MaskChan::axisTracer(int direction, int label, QList<MaskRay*> * rayList, long& pairCount, long& voxelCount,
			  long& x0, long& x1, long& y0, long& y1, long& z0, long& z1, void* data, long assumedVoxelCount)
{

  bool debug=false;
  long skipCount=0L;
  long start=-1;
  long end=-1;
  MaskRay* ray=0L;
  long D0, D1, D2;

  x0=LONG_MAX;
  x1=0L;
  y0=LONG_MAX;
  y1=0L;
  z0=LONG_MAX;
  z1=0L;

  if (direction==0) { // X-axis
    D0=ydim;
    D1=zdim;
    D2=xdim;
  } else if (direction==1) { // Y-axis
    D0=xdim;
    D1=zdim;
    D2=ydim;
  } else if (direction==2) { // Z-axis
    D0=xdim;
    D1=ydim;
    D2=zdim;
  }

  long cdim=sourceImage->getCDim();
  long cOffset=sourceImage->getTotalUnitNumberPerChannel();
  v3d_uint8* source8=0L;
  v3d_uint16* source16=0L;
  v3d_uint8* data8=0L;
  v3d_uint16* data16=0L;
  if (sourceImage->getDatatype()==V3D_UINT8) {
    source8=(v3d_uint8*)sourceImage->getRawData();
    data8=(v3d_uint8*)data;
  } else {
    source16=(v3d_uint16*)sourceImage->getRawData();
    data16=(v3d_uint16*)data;
  }

  //qDebug() << "Inside axisTracer data8=" << data8 << " source8=" << source8 <<" data16=" << data16 << " source16=" << source16;


  voxelCount=0L;
  pairCount=0L;
  long dataPosition=0L;

  const int START_STATE_BEGIN=0;
  const int START_STATE_JUST_STARTED=1;
  const int START_STATE_STARTED_IN_LAST_POSITION=2;

  long xdimydim=xdim*ydim;

  for (long d0=0;d0<D0;d0++) {
    long d0xdim=d0*xdim;
    for (long d1=0;d1<D1;d1++) {

      start=-1;
      end=-1;
      long d1Offset=d1*xdimydim;
      long d1xdim=d1*xdim;
      long offset=0L;

      // do label check for this ray
      bool foundLabelInThisRay=false;
      if (label8 != 0L) {
	if (direction==0) {
	  for (long d2=0;d2<D2;d2++) {
	    offset=d1Offset + d0xdim + d2;
	    if (label8[offset]==label) {
	      foundLabelInThisRay=true;
	      break;
	    }
	  }
	} else if (direction==1) {
	  for (long d2=0;d2<D2;d2++) {
	    offset=d1Offset + d2*xdim + d0;
	    if (label8[offset]==label) {
	      foundLabelInThisRay=true;
	      break;
	    }
	  }
	} else { // direction==2
	  for (long d2=0;d2<D2;d2++) {
	    offset=d2*xdimydim + d1xdim + d0;
	    if (label8[offset]==label) {
	      foundLabelInThisRay=true;
	      break;
	    }
	  }
	}
      } else { // label16
	if (direction==0) {
	  for (long d2=0;d2<D2;d2++) {
	    offset=d1Offset + d0xdim + d2;
	    if (label16[offset]==label) {
	      foundLabelInThisRay=true;
	      break;
	    }
	  }
	} else if (direction==1) {
	  for (long d2=0;d2<D2;d2++) {
	    offset=d1Offset + d2*xdim + d0;
	    if (label16[offset]==label) {
	      foundLabelInThisRay=true;
	      break;
	    }
	  }
	} else { // direction==2
	  for (long d2=0;d2<D2;d2++) {
	    offset=d2*xdimydim + d1xdim + d0;
	    if (label16[offset]==label) {
	      foundLabelInThisRay=true;
	      break;
	    }
	  }
	}
      }

      if (foundLabelInThisRay) {

	for (long d2=0;d2<D2;d2++) {
	  int startState=START_STATE_BEGIN;
	  offset=0L;
	  if (direction==0) {
	    offset=d1Offset + d0xdim + d2;
	  } else if (direction==1) {
	    offset=d1Offset + d2*xdim + d0;
	  } else { // direction==2
	    offset=d2*xdimydim + d1xdim + d0;
	  }
	  int labelValue=-1;
	  if (label8 != 0L) {
	    labelValue=label8[offset];
	  } else {
	    labelValue=label16[offset];
	  }
	  if (start==-1 && labelValue==label) {
	    startState=START_STATE_JUST_STARTED;
	    if (ray==0L) {
	      ray=new MaskRay();
	      ray->skipCount=skipCount;
	      skipCount=0L;
	    }
	    start=d2;
	    end=-1;
	    ray->startList.append(start);
	    if (d2==(D2-1)) {
	      startState=START_STATE_STARTED_IN_LAST_POSITION;
	    }
	  }
	  if (startState==START_STATE_STARTED_IN_LAST_POSITION ||
	      (startState==START_STATE_BEGIN && (start>=0 && ( labelValue!=label || ( labelValue==label && d2==(D2-1) ) ) ) ) ) {
	    end=d2;
	    if (labelValue==label && d2==(D2-1)) { // handle last-case
	      end=D2;
	    }
	    ray->endList.append(end);
	    voxelCount+=(end-start);
	    if (debug) {
	      qDebug() << "start=" << start << " end=" << end << " voxelCount=" << voxelCount;
	    }
	    if (data != 0L) {
	      if (voxelCount > assumedVoxelCount) {
		qDebug() << "Error: assumedVoxelCount greater than voxelCount";
		exit(1);
	      }
	      for (int i=start;i<end;i++) {
		long sourcePosition=0L;
		if (direction==0) {
		  sourcePosition=d1Offset + d0xdim + i;
		} else if (direction==1) {
		  sourcePosition=d1Offset + i*xdim + d0;
		} else { // direction==2
		  sourcePosition=i*xdimydim + d1xdim + d0;
		}
		for (long c=0;c<cdim;c++) {
		  long cof=c*cOffset;
		  long dof=c*assumedVoxelCount;
		  //qDebug() << "dataPosition=" << dataPosition << " c=" << c << " dof=" << dof << " cof=" << cof << " voxelCount=" << voxelCount;
		  if (data8 != 0L) {
		    data8[dataPosition+dof]=source8[sourcePosition+cof];
		  } else {
		    data16[dataPosition+dof]=source16[sourcePosition+cof];
		  }
		}
		dataPosition++;
	      }
	    }
	    start=-1;
	    end=-1;
	  }
	}

      } // if foundLabelInThisRay


      if (ray != 0L) {
	pairCount+=ray->endList.size();
	// Update max values - we will just check first and last values
	long start=ray->startList[0];
	long end=ray->endList[ray->endList.size()-1];
	if (direction==0) { // x=d2 y=d0 z=d1
	  if (start<x0) x0=start;
	  if (end>x1) x1=end;
	  if (d0<y0) y0=d0;
	  if (d0>y1) y1=d0;
	  if (d1<z0) z0=d1;
	  if (d1>z1) z1=d1;
	} else if (direction==1) { // x=d0 y=d2 z=d1
	  if (d0<x0) x0=d0;
	  if (d0>x1) x1=d0;
	  if (start<y0) y0=start;
	  if (end>y1) y1=end;
	  if (d1<z0) z0=d1;
	  if (d1>z1) z1=d1;
	} else if (direction==2) { // x=d0 y=d1 z=d2
	  if (d0<x0) x0=d0;
	  if (d0>x1) x1=d0;
	  if (d1<y0) y0=d1;
	  if (d1>y1) y1=d1;
	  if (start<z0) z0=start;
	  if (end>z1) z1=end;
	}
	rayList->append(ray);
	ray=0L;
      } else {
	skipCount++;
      }
    }
  }

}

void MaskChan::writeMaskList(QDataStream& dataOut, QList<MaskRay*>& list)
{
    for (int i=0;i<list.size();i++) {
        dataOut.writeRawData( (const char *)&(list[i]->skipCount), sizeof(long));
        long pairListSize=list[i]->endList.size();
        dataOut.writeRawData( (const char *)&pairListSize, sizeof(long));
        //qDebug() << "skip=" << list[i]->skipCount << " pairs=" << pairListSize;
        for(int j=0;j<list[i]->endList.size();j++) {
            long start=list[i]->startList[j];
            dataOut.writeRawData( (const char *)&start, sizeof(long));
            long end=list[i]->endList[j];
            dataOut.writeRawData( (const char *)&end, sizeof(long));
            //qDebug() << "start=" << start << " end=" << end;
        }
    }
}

My4DImage* MaskChan::createImageFromMaskFiles(QStringList& maskFilePaths)
{
    My4DImage* outputStack = 0L;

    // Use the first mask file to dictate the dimensions
    FILE* fid=0L;
    xdim=-1;
    ydim=-1;
    zdim=-1;
    cdim=-1;

    v3d_uint8** data8=0L;
    v3d_uint16** data16=0L;

    for (int i=0;i<maskFilePaths.size();i++) {
        QString maskFilePath=maskFilePaths[i];
        // According to the Qt docs using toAscii and its ilk
        // has a memory leak, which apparently can corrupt the pointer on Windows
        // and is deprecated in Qt5
        QByteArray ba = maskFilePath.toUtf8();
        const char* filepath = ba.constData();
        if ( (fid=fopen(filepath, "rb"))==0) {
            qDebug() << "Could not open file " << filepath << " to read";
            return NULL;
        }

        long chanVoxels=0L;
        unsigned char chanChannelCount=0;
        unsigned char chanRecRed=0;
        unsigned char chanRecGreen=0;
        unsigned char chanRecBlue=0;
        unsigned char chanBytesPerChannel=0;
        unsigned char* chanData=0L;
        long chanTotalDataBytes=0L;
        QString chanFilePath=QString("");
        int maskFilePrefixPosition=maskFilePath.lastIndexOf(".");
        if (maskFilePrefixPosition > 0) {
            chanFilePath.append(maskFilePath.left(maskFilePrefixPosition));
            chanFilePath.append(".chan");
            FILE* fid2=0L;
            ba = chanFilePath.toUtf8();
            filepath = ba.constData();
            if ( (fid2=fopen(filepath, "rb")) != 0) {
                fread(&chanVoxels, sizeof(long), 1, fid2);
                fread(&chanChannelCount, sizeof(unsigned char), 1, fid2);
                fread(&chanRecRed, sizeof(unsigned char), 1, fid2);
                fread(&chanRecGreen, sizeof(unsigned char), 1, fid2);
                fread(&chanRecBlue, sizeof(unsigned char), 1, fid2);
                fread(&chanBytesPerChannel, sizeof(unsigned char), 1, fid2);
		//                qDebug() << "File " << chanFilePath << " has chanBytesPerChannel=" << chanBytesPerChannel;
                chanTotalDataBytes=chanBytesPerChannel*chanVoxels*chanChannelCount;
                chanData=new unsigned char[chanTotalDataBytes];
                fread(chanData, sizeof(unsigned char), chanTotalDataBytes, fid2);
                fclose(fid2);
                if (cdim==-1) {
                    cdim=chanChannelCount;
                } else {
                    if (cdim!=chanChannelCount) {
                        qDebug() << "Expected channel counts to match between mask files, cdim=" << cdim << " chanChannelCount=" << chanChannelCount;
                        return NULL;
                    }
                }
            }
        }

	if (data8==0L) {
	  data8=new v3d_uint8*[cdim];
	}
	if (data16==0L) {
	  data16=new v3d_uint16*[cdim];
	}

        long xsize=0L;
        long ysize=0L;
        long zsize=0L;
        fread(&xsize, sizeof(long), 1, fid);
        fread(&ysize, sizeof(long), 1, fid);
        fread(&zsize, sizeof(long), 1, fid);

        if (xdim==-1) {
            xdim=xsize;
        } else {
            if (xsize!=xdim) {
                qDebug() << "xsize " << xsize << " in file " << maskFilePath << " does not match " << xdim;
            }
        }

        if (ydim==-1) {
            ydim=ysize;
        } else {
            if (ysize!=ydim) {
                qDebug() << "ysize " << ysize << " in file " << maskFilePath << " does not match " << ydim;
            }
        }

        if (zdim==-1) {
            zdim=zsize;
        } else {
            if (zsize!=zdim) {
                qDebug() << "zsize " << zsize << " in file " << maskFilePath << " does not match " << zdim;
            }
        }

        if (outputStack==0L) {
            outputStack=new My4DImage();
            if (chanBytesPerChannel==0 || chanBytesPerChannel==1) {
                outputStack->loadImage(xdim, ydim, zdim, cdim, V3D_UINT8);
            } else if (chanBytesPerChannel==2) {
                outputStack->loadImage(xdim, ydim, zdim, cdim, V3D_UINT16);
            } else {
                qDebug() << "Do not understand chanBytesPerChannel=" << chanBytesPerChannel;
                return NULL;
            }
        }

        float xmicrons=0.0;
        float ymicrons=0.0;
        float zmicrons=0.0;

        // we ignore these for now
        fread(&xmicrons, sizeof(float), 1, fid);
        fread(&ymicrons, sizeof(float), 1, fid);
        fread(&zmicrons, sizeof(float), 1, fid);

        long x0;
        long x1;
        long y0;
        long y1;
        long z0;
        long z1;

        // bounding box
        fread(&x0, sizeof(long), 1, fid);
        fread(&x1, sizeof(long), 1, fid);
        fread(&y0, sizeof(long), 1, fid);
        fread(&y1, sizeof(long), 1, fid);
        fread(&z0, sizeof(long), 1, fid);
        fread(&z1, sizeof(long), 1, fid);

        long totalVoxels;
        fread(&totalVoxels, sizeof(long), 1, fid);

        if (chanVoxels>0L && chanVoxels!=totalVoxels) {
            qDebug() << "Mask and Chan totalVoxel counts do not match";
            return NULL;
        }

        unsigned char axis;
        fread(&axis, sizeof(unsigned char), 1, fid);

        long readVoxels=0L;
        long planePosition=0L;


        for (int c=0;c<cdim;c++) {
            if (chanBytesPerChannel==0 || chanBytesPerChannel==1) {
                data8[c]=outputStack->getRawDataAtChannel(c);
                data16[c]=0L;
            } else {
                data8[c]=0L;
                data16[c]=(v3d_uint16*)outputStack->getRawDataAtChannel(c);
            }
        }

        v3d_uint16* chanData16=(v3d_uint16*)chanData;

        while(readVoxels<totalVoxels) {
            // Start new ray
            long skip=0L;
            fread(&skip, sizeof(long), 1, fid);
            planePosition+=skip;
            long pairs=0L;
            fread(&pairs, sizeof(long), 1, fid);
            for (long p=0;p<pairs;p++) {
                long start=0L;
                long end=0L;
                fread(&start, sizeof(long), 1, fid);
                fread(&end, sizeof(long), 1, fid);
                if (axis==0) { // yz(x)
                    long y=planePosition/zdim;
                    long z=planePosition-(zdim*y);
                    for (int x=start;x<end;x++) {
                        long offset=z*ydim*xdim+y*xdim+x;
                        if (chanBytesPerChannel==0) {
                            for (int c=0;c<cdim;c++) {
                                data8[c][offset]=255;
                            }
                        } else if (chanBytesPerChannel==1) {
                            for (int c=0;c<cdim;c++) {
                                data8[c][offset]=chanData[readVoxels+(x-start)+c*chanVoxels];
                            }
                        } else if (chanBytesPerChannel==2) {
                            for (int c=0;c<cdim;c++) {
                                data16[c][offset]=chanData16[readVoxels+(x-start)+c*chanVoxels];
                            }
                        }
                    }
                } else if (axis==1) { // xz(y)
                    long x=planePosition/zdim;
                    long z=planePosition-(zdim*x);
                    for (int y=start;y<end;y++) {
                        long offset=z*ydim*xdim+y*xdim+x;
                        if (chanBytesPerChannel==0) {
                            for (int c=0;c<cdim;c++) {
                                data8[c][offset]=255;
                            }
                        } else if (chanBytesPerChannel==1) {
                            for (int c=0;c<cdim;c++) {
                                data8[c][offset]=chanData[readVoxels+(y-start)+c*chanVoxels];
                            }
                        } else if (chanBytesPerChannel==2) {
                            for (int c=0;c<cdim;c++) {
                                data16[c][offset]=chanData16[readVoxels+(y-start)+c*chanVoxels];
                            }
                        }
                    }
                } else if (axis==2) { // xy(z)
                    long x=planePosition/ydim;
                    long y=planePosition-(ydim*x);
                    for (int z=start;z<end;z++) {
                        long offset=z*ydim*xdim+y*xdim+x;
                        if (chanBytesPerChannel==0) {
                            for (int c=0;c<cdim;c++) {
                                data8[c][offset]=255;
                            }
                        } else if (chanBytesPerChannel==1) {
                            for (int c=0;c<cdim;c++) {
                                data8[c][offset]=chanData[readVoxels+(z-start)+c*chanVoxels];
                            }
                        } else if (chanBytesPerChannel==2) {
                            for (int c=0;c<cdim;c++) {
                                data16[c][offset]=chanData16[readVoxels+(z-start)+c*chanVoxels];
                            }
                        }
                    }
                }
                readVoxels+=(end-start);
            }
            planePosition++;
        }
        fclose(fid);

        if (chanData!=0L) {
            delete [] chanData;
        }

    }

    if (data8!=0L) {
      delete [] data8;
    }

    if (data16!=0L) {
      delete [] data16;
    }

    return outputStack;
}

