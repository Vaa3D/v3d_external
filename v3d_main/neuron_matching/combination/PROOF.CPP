#include<cstring>
#include<iostream>
#include<string>
#include<vector>
#include<set>
#include<algorithm>
#include"combination.h"

using namespace std;
using namespace stdcomb;


set<string> s1;
pair< set<string>::iterator, bool > pr;
unsigned int errors=0;

void processit(char* a)
{
  
  string str(a);
  try
  {
  pr=s1.insert(str);
  if (pr.second==false)
    ++errors;
  }
  catch ( exception &e ) 
  {
      cerr << "An error has occurred" << endl;
      cerr << "Caught: " << e.what( ) << endl;
      cerr << "Type: " << typeid( e ).name( ) << endl;
  };

}

int main()
{

  char n[]="123456";
  char r[]="1234";
  char r1[]="1234";
  unsigned int count=0;

  do
  {
    ++count;
    strncpy(r1,r,4);
    processit(r);
    while(next_permutation(r1,r1+4))
    {
      ++count;
      processit(r1);
    }
  }
  while(next_combination(n,n+6,r,r+4));

  
  //Printing
  //for(set<string>::const_iterator it=s1.begin();it!=s1.end();++it)
  //      cout<<*it<<endl;
  
  
  cout<<"errors = "<<errors<<endl;
  cout<<"Total number of unique permutations : "<<count-errors<<endl;

  return 0;
}

