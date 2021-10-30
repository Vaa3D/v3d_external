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




// TestComboDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TestCombo.h"
#include "TestComboDlg.h"
#include "combination.h"
#include<utility>
#include<map>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace stdcomb;

int func(char* begin,char* end);

char n[]="ABCDEFGHIJKLMNOPQRST";
int char_count=0;
int global_num=0;

/////////////////////////////////////////////////////////////////////////////
// CTestComboDlg dialog

CTestComboDlg::CTestComboDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTestComboDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTestComboDlg)
	m_n_seq = 0;
	m_r_seq = 0;
	m_complete = _T("");
	m_radio = -1;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTestComboDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTestComboDlg)
	DDX_Control(pDX, IDC_PROGRESS1, c_progressbar);
	DDX_Text(pDX, IDC_N_SEQ, m_n_seq);
	DDV_MinMaxInt(pDX, m_n_seq, 1, 20);
	DDX_Text(pDX, IDC_R_SEQ, m_r_seq);
	DDV_MinMaxInt(pDX, m_r_seq, 1, 20);
	DDX_Text(pDX, IDC_EDIT3, m_complete);
	DDX_Radio(pDX, IDC_RADIO1, m_radio);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTestComboDlg, CDialog)
	//{{AFX_MSG_MAP(CTestComboDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDSTART, OnStart)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestComboDlg message handlers

BOOL CTestComboDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	m_n_seq=20;
	m_r_seq=6;
	m_complete="";
	c_progressbar.SetPos(0);

	UpdateData(FALSE);
	
		
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTestComboDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTestComboDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CTestComboDlg::OnStart() 
{
	// TODO: Add your control notification handler code here
	if(UpdateData(TRUE)==FALSE) return;
  if(m_n_seq<m_r_seq)
  {
    MessageBox("n is lesser than r","Error",MB_OK|MB_ICONERROR);
    return;
  }
  if(m_n_seq<1||m_n_seq>20||m_n_seq<1||m_n_seq>20)
    return;
  if(m_radio<0)
  {
    MessageBox("No algorithm is selected","Error",MB_OK|MB_ICONERROR);
    return;
  }
	if(m_radio==0||m_radio==4)
	{
		m_complete="Processing Next Combinations...";
		UpdateData(FALSE);
    UpdateWindow();
		__int64 num=NumOfCombo();
		int count=0;
		__int64 increment=0;
		c_progressbar.SetPos(0);
		InitialiseMap();
		//Initialising r sequence
		char *r=new char[m_r_seq+1];
		for(int cnt=0;cnt<m_r_seq;++cnt)
		{
			r[cnt]=n[cnt];
		}
		r[m_r_seq]=0;
		do
		{
			++count;
			for(int i=0;i<m_r_seq;++i)
			{
				++mc[r[i]];
			}
			if(count*10/num>increment)
			{
				++increment;
				c_progressbar.OffsetPos(10);
			}
		}
		while(next_combination(n, n+m_n_seq,  r, r+m_r_seq));
		c_progressbar.SetPos (100);
    delete [] r;
		m_complete="Complete";
		UpdateData(FALSE);
		if(CheckMap()==false || count!=NumOfCombo()) 
			MessageBox("Error"); 
		else if(CheckMap()==true && count==NumOfCombo())
		{
			char buf[1000];
			sprintf(buf,"Total Combinations is %d\nEach char appears %d",
				count,NumOfCombo()*m_r_seq/m_n_seq);
			MessageBox(buf,"Next Combinations");
		}
	}
	if(m_radio==1||m_radio==4)
	{
		m_complete="Processing Previous Combinations...";
		UpdateData(FALSE);
    UpdateWindow();
		__int64 num=NumOfCombo();
		int count=0;
		__int64 increment=0;
		c_progressbar.SetPos(0);
		InitialiseMap();
		//Initialising r sequence
		char *r=new char[m_r_seq+1];
		for(int cnt=0;cnt<m_r_seq;++cnt)
		{
			r[cnt]=n[m_n_seq-m_r_seq+cnt];
		}
		r[m_r_seq]=0;
		do
		{
			++count;
			for(int i=0;i<m_r_seq;++i)
			{
				++mc[r[i]];
			}
			if(count*10/num>increment)
			{
				++increment;
				c_progressbar.OffsetPos(10);
			}
		}
		while(prev_combination(n, n+m_n_seq,r, r+m_r_seq));
		c_progressbar.SetPos (100);
		delete [] r;
		m_complete="Complete";
		UpdateData(FALSE);
		if(CheckMap()==false || count!=NumOfCombo()) 
			MessageBox("Error"); 
		else if(CheckMap()==true && count==NumOfCombo())
		{
			char buf[1000];
			sprintf(buf,"Total Combinations is %d\nEach char appears %d",
				count,NumOfCombo()*m_r_seq/m_n_seq);
			MessageBox(buf,"Previous Combinations");
		}
	}
	
	if(m_radio==2||m_radio==4)
	{
		m_complete="Processing Recursive Combinations...";
		UpdateData(FALSE);
    UpdateWindow();
		__int64 num=NumOfCombo();
		__int64 increment=0;
		c_progressbar.SetPos(0);
		InitialiseMap();
		//Initialising r sequence
		char *r=new char[m_r_seq+1];
		for(int cnt=0;cnt<m_r_seq;++cnt)
		{
			r[cnt]=n[cnt];
		}
		r[m_r_seq]=0;
		char_count=0;
		
		recursive_combination(n,n+m_n_seq,0,r,r+m_r_seq,0,m_n_seq-m_r_seq, func);
		c_progressbar.SetPos (100);
		delete [] r;
		m_complete="Complete";
		UpdateData(FALSE);
		if(CheckMap()==false || char_count!=NumOfCombo()) 
			MessageBox("Error"); 
		else if(CheckMap()==true && char_count==NumOfCombo())
		{
			char buf[1000];
			sprintf(buf,"Total Combinations is %d\nEach char appears %d",
				char_count,NumOfCombo()*m_r_seq/m_n_seq);
			MessageBox(buf,"Recursive Combinations");
		}
	}
	if(m_radio==3||m_radio==4)
	{
		m_complete="Processing Character Combinations...";
		UpdateData(FALSE);
    UpdateWindow();
		__int64 num=NumOfCombo();
		__int64 increment=0;
		c_progressbar.SetPos(0);
		InitialiseMap();
		//Initialising r sequence
		char *r=new char[m_r_seq+1];
		for(int cnt=20-m_r_seq;cnt<m_r_seq;++cnt)
		{
			r[cnt]=n[cnt];
		}
		r[m_r_seq]=0;
		char_count=0;
		global_num=NumOfCombo();
		char_combination(n,0,r,m_r_seq,0,m_n_seq-m_r_seq);
		delete [] r;
		c_progressbar.SetPos (100);
		m_complete="Complete";
		UpdateData(FALSE);
		if(CheckMap()==false || char_count!=NumOfCombo()) 
			MessageBox("Error"); 
		else if(CheckMap()==true && char_count==NumOfCombo())
		{
			char buf[1000];
			sprintf(buf,"Total Combinations is %d\nEach char appears %d",
				char_count,NumOfCombo()*m_r_seq/m_n_seq);
			MessageBox(buf,"Char Combinations");
		}
	}
}

int func(char* begin,char* end)
{
	CTestComboDlg* cDlg=(CTestComboDlg*)AfxGetMainWnd();

  cDlg->temp(begin,end);
  

	return 0;
}
void CTestComboDlg::temp(char *begin,char* end)
{
  ++char_count;
	int num=NumOfCombo();
	static int increment1=0;
	if(char_count==1) increment1=0;
	for(int i=0;i<m_r_seq;++i)
	{
		++mc[begin[i]];
	}
	if(char_count*10/num>increment1)
	{
		++increment1;
		c_progressbar.OffsetPos(10);
	}
}
bool CTestComboDlg::CheckMap()
{
	std::map<char,int>::const_iterator it=mc.begin ();
	int num=NumOfCombo()*m_r_seq/m_n_seq;
	for(;it!=mc.end();++it)
	{
		if(it->second!=num)
		return false;
	}
	return true;
}
void CTestComboDlg::char_combination(char n[],int n_column,
            char r[], int r_size, int r_column, int loop)
{
	int localloop=loop;
	int local_n_column=n_column;
	static int increment2=0;
	if(char_count==0) increment2=0;
	///////Display the string code/////////
	if(r_column>(r_size-1))
	{
		++char_count;
		for(int i=0;i<m_r_seq;++i)
		{
			++mc[r[i]];
		}
		if(char_count*10/global_num>increment2)
		{
			++increment2;
			c_progressbar.OffsetPos(10);
		}
		return;
	}
	/////End of displaying string code//////
	
	for(int i=0;i<=loop;++i)
	{
		r[r_column]=n[n_column+i];
		++local_n_column;
		char_combination(n,local_n_column,r,r_size,r_column+1,localloop);
		--localloop;
	}
}

void CTestComboDlg::InitialiseMap()
{
	if(mc.empty ()==false)
	{
		mc.clear();
	}
	for(int cnt=0;cnt<m_n_seq;++cnt)
	{
		std::pair<char,int> pa;
		pa.first  =n[cnt];
		pa.second =0;
		mc.insert (pa);
	}	


} 

__int64 CTestComboDlg::fact(int num)
{
	__int64 result=1;
	for(int i=2;i<=num;++i)
	{
		result*=i;

	}
	return result;
}

__int64 CTestComboDlg::NumOfCombo()
{
	UpdateData(TRUE);
	return fact(m_n_seq)/(fact(m_r_seq)*fact(m_n_seq-m_r_seq));
}
