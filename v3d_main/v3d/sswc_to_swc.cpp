/*added by KLS */
#include "sswc_to_swc.h"
#include <iostream>
#include "../basic_c_fun/basic_surf_objs.h"
#include <sstream>


NeuronTree sswc_to_swc::readSSWC_file(const QString& filename)
{	 
	NeuronTree nt;
    nt.file = QFileInfo(filename).absoluteFilePath();
	QFile qf(filename);
	if (! qf.open(QIODevice::ReadOnly | QIODevice::Text))
	{
#ifndef DISABLE_V3D_MSG
		v3d_msg(QString("open file [%1] failed!").arg(filename));
#endif
		return nt;
	}

	int count = 0;
	int steps = 10;
	int l = 0;
	int kl = 0;
    QList <NeuronSWC> listNeuron;
	QHash <int, int>  hashNeuron;
	listNeuron.clear();
	hashNeuron.clear();
	QString name = "";
	QString comment = "";
	QString branch = "";
	QSettings settings("HHMI", "Vaa3D");
	QString step_tex_default = "";
        if(!settings.value("step size").toString().isEmpty())
            step_tex_default = settings.value("step size").toString();
        bool ok2;
        QString step_tex = QInputDialog::getText(0, "parameter : step",
                                             "Please enter parameter: step size", QLineEdit::Normal,
                                             step_tex_default, &ok2);

		if(ok2 && !step_tex.isEmpty())
			settings.setValue("step size", step_tex);
		steps= step_tex.toInt();

    qDebug("-------------------------------------------------------");
    while (! qf.atEnd())
    {
        char _buf[1000], *buf;
        qf.readLine(_buf, sizeof(_buf));
        for (buf=_buf; (*buf && *buf==' '); buf++); //skip space

        //  add #name, #comment, #Branch
        if (buf[0]=='\0')	continue;
        if (buf[0]=='#')
        {
        	if (buf[1]=='n'&&buf[2]=='a'&&buf[3]=='m'&&buf[4]=='e'&&buf[5]==' ')
        		name = buf+6;
        	if (buf[1]=='c'&&buf[2]=='o'&&buf[3]=='m'&&buf[4]=='m'&&buf[5]=='e'&&buf[6]=='n'&&buf[7]=='t'&&buf[8]==' ')
        		comment = buf+9;
			if (buf[1]=='B'&&buf[2]=='r'&&buf[3]=='a'&&buf[4]=='n'&&buf[5]=='c'&&buf[6]=='h'&&buf[7]==' ')
        		branch = buf+8;
        	continue;
       	}
        QStringList qsl = QString(buf).trimmed().split(" ",QString::SkipEmptyParts);
        if (qsl.size()==0)   continue;
		
		/*added by KLS*/
		if (qsl.size()==7)
		{
			kl++;
		}
		if (qsl.size()==10)
		{
			l++;
		}
    }
	QFile qf1(filename);
	if (! qf1.open(QIODevice::ReadOnly | QIODevice::Text))
	{
#ifndef DISABLE_V3D_MSG
		v3d_msg(QString("open file [%1] failed!").arg(filename));
#endif
		return nt;
	}
	vector<float> px(l),py(l),pz(l),pr(l),tx(l),ty(l),tz(l),tr(l),k0(l),k1(l),knodetype(kl),knodex(kl),knodey(kl),knodez(kl);
	int length = 0;
	int klength = 0;
	 while (! qf1.atEnd())
    {
        char _buf[1000], *buf;
        qf1.readLine(_buf, sizeof(_buf));
        for (buf=_buf; (*buf && *buf==' '); buf++); //skip space
        //  add #name, #comment, #Branch
        if (buf[0]=='\0')	continue;
        if (buf[0]=='#')
        {
        	if (buf[1]=='n'&&buf[2]=='a'&&buf[3]=='m'&&buf[4]=='e'&&buf[5]==' ')
        		name = buf+6;
        	if (buf[1]=='c'&&buf[2]=='o'&&buf[3]=='m'&&buf[4]=='m'&&buf[5]=='e'&&buf[6]=='n'&&buf[7]=='t'&&buf[8]==' ')
        		comment = buf+9;
			if (buf[1]=='B'&&buf[2]=='r'&&buf[3]=='a'&&buf[4]=='n'&&buf[5]=='c'&&buf[6]=='h'&&buf[7]==' ')
        		branch = buf+8;
        	continue;
       	}
        QStringList qsl = QString(buf).trimmed().split(" ",QString::SkipEmptyParts);
        if (qsl.size()==0)   continue;
		/*added by KLS*/
		if (qsl.size()==10){
			for (int i = 0; i < qsl.size(); i++)
			{
				qsl[i].truncate(99);
					if (i==0) px[length] = qsl[i].toFloat();
					else if (i==1) py[length] = qsl[i].toFloat();
					else if (i==2) pz[length] = qsl[i].toFloat();
					else if (i==3) pr[length] = qsl[i].toFloat();
					else if (i==4) tx[length] = qsl[i].toFloat();
					else if (i==5) ty[length] = qsl[i].toFloat();
					else if (i==6) tz[length] = qsl[i].toFloat();
					else if (i==7) tr[length] = qsl[i].toFloat();
					else if (i==8) k0[length] = qsl[i].toFloat();
					else if (i==9) k1[length] = qsl[i].toFloat();
			}
			if(length<l)
			length++;
		}
		if(qsl.size() == 7){
			for (int i = 1; i < 5; i++){
				qsl[i].truncate(99);
				if (i==1) knodetype[klength] = qsl[i].toFloat();
				else if (i==2) knodex[klength] = qsl[i].toFloat();
        		else if (i==3) knodey[klength] = qsl[i].toFloat();
        		else if (i==4) knodez[klength] = qsl[i].toFloat();
			}
			if(klength<kl)
				klength++;
		}
	}
	NeuronSWC S;
	int number = 0;
	int color = 1;
	for (int i = 0; i < l; i++)
	{	
		if (i == 0)
		{
			S.n = 1;
			S.type = knodetype[0];
			S.x = px[i];
			S.y = py[i];
			S.z = pz[i];
			S.r = pr[i];
			S.pn = -1;
			listNeuron.append(S);
			hashNeuron.insert(S.n, listNeuron.size()-1);
		}
		bool T = false;
		for(int k =0;k<kl;k++){
			if(px[i] == knodex[k]){
				T = true;
			}
		}
		int xnum=0;
		for(int test=0;test<l;test++){
			if((px[i] == px[test])&&(py[i] == py[test]))
				xnum++;
		}
		//if ((px[i] != px[i+1])&&(i+1<=l))
		if ((i==0)||!((xnum==1)&&T)&&(px[i]!=px[i+1])&&(py[i]!=py[i+1])&&(pz[i]!=pz[i+1]))
		{
			for (int j = 1; j <= steps; j++)
			{
				float delta = 1.0/steps;
				float t = j * delta;
				float t1 = 2 * pow(t, 3) - 3 * pow(t, 2) + 1;
				float t2 = -2 * pow(t, 3) + 3 * pow(t, 2);
				float t3 = pow(t, 3) - 2 * pow(t, 2) + t;
				float t4 = pow(t, 3) - pow(t, 2);
				S.n = steps*number+j+1;
				S.type = knodetype[color];                   //test
				//S.type =1;
				S.x = px[i]*t1+px[i+1]*t2+tx[i]*t3*k1[i]+tx[i+1]*t4*k0[i+1];	
				S.y = py[i]*t1+py[i+1]*t2+ty[i]*t3*k1[i]+ty[i+1]*t4*k0[i+1];
				S.z = pz[i]*t1+pz[i+1]*t2+tz[i]*t3*k1[i]+tz[i+1]*t4*k0[i+1];
				S.r = pr[i]*t1+pr[i+1]*t2+tr[i]*t3*k1[i]/2+tr[i+1]*t4*k0[i+1]/2;
				if(j == 1){
					for(int listlength = 0;listlength<listNeuron.size();listlength++)
					{
						if(listNeuron.at(listlength).x == px[i])
							S.pn = listlength+1;
					}
				}else{
					S.pn = steps*number+j;
				}
				listNeuron.append(S);
				hashNeuron.insert(S.n, listNeuron.size()-1);
			}
			number++;
		}else{color++;}
	}
	qDebug("---------------------read %d lines, %d remained lines", count, listNeuron.size());
    if (listNeuron.size()<1)
    	return nt;
	//now update other NeuronTree members
    nt.n = 1; //only one neuron if read from a file
    nt.listNeuron = listNeuron;
    nt.hashNeuron = hashNeuron;
    nt.color = XYZW(0,0,0,0); /// alpha==0 means using default neuron color, 081115
    nt.on = true;
    nt.name = name.remove('\n'); if (nt.name.isEmpty()) nt.name = QFileInfo(filename).baseName();
    nt.comment = comment.remove('\n');

	return nt;
}
