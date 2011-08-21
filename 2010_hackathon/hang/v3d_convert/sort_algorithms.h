#ifndef _SORT_ALGORITHMS_H_H_
#define _SORT_ALGORITHMS_H_H_

#include <vector>
#include <cmath>
using namespace std;

//#define __SORT_DEBUG__

#ifdef __SORT_DEBUG__
#include <iostream>
#endif

// only suitable for integer data
template<class T> bool bucket_sort(T* outimg1d, T* inimg1d, long tol_sz)
{
	if(inimg1d == 0 || tol_sz <= 0) return false;
	if(outimg1d == 0)outimg1d = new T[tol_sz];
	// forbid sort of double and float data
	//if(sizeof(T) == sizeof(double) || sizeof(T) == sizeof(float)) return false;
	struct Node{
		T value;
		Node * next;
#ifndef __LONG_POSITION__
		int pos;
#else
		long pos;
#endif
	};
	struct Bucket
	{
		Node * head;
		Node * last;
	};
	int numBuckets = 256; // 16
	int offset = 8; // 4
	Node * sortdata = new Node[tol_sz];
	Bucket* buckets = new Bucket[numBuckets]; for(int i = 0; i < numBuckets; i++){ buckets[i].head = 0; buckets[i].last = 0;}
	T max_value = 0;
	for(int i=0;i<tol_sz;i++)
	{
		sortdata[i].value = inimg1d[i];
		sortdata[i].next = sortdata + i+1;
		sortdata[i].pos = i;
		if(inimg1d[i] > max_value)max_value = inimg1d[i];
		if(inimg1d[i] < 0) 
		{
			delete sortdata;
			delete buckets;
#ifdef __SORT_DEBUG__
			cerr<<"not suitable for negate intensity value"<<endl;
#endif
			return false;
		}
	}
	sortdata[tol_sz -1].next = 0;

	int nloops = (int)(log(max_value)/log(numBuckets)) + 1;

	Node * global_head = sortdata;
	Node * itr = 0;
	for(int loop =1; loop <= nloops; loop++)
	{
		int i = 0;
		itr = global_head;
		while(itr)
		{
			Node * cur_node = itr;
			itr = itr->next;

			int new_buck_id = cur_node->value % numBuckets;
			if(buckets[new_buck_id].head == 0)
			{
				cur_node->next = 0;// buckets[new_buck_id].head;
				buckets[new_buck_id].head = cur_node;
				buckets[new_buck_id].last = cur_node;
			}
			else
			{
				//cur_node->next = buckets[new_buck_id].head;
				//buckets[new_buck_id].head = cur_node;
				buckets[new_buck_id].last->next = cur_node;
				cur_node->next = 0;
				buckets[new_buck_id].last = cur_node;
			}
			cur_node->value >>= offset;
		}
#ifdef __SORT_DEBUG__
		for(i = 0; i < numBuckets; i++)
		{
			if(buckets[i].head)
			{
				cout<<"bucket "<<i<<" : ";
				Node * it = buckets[i].head;
				while(it)
				{
					cout<<inimg1d[it->pos]<<"\t";
					it = it->next;
				}
				cout<<endl;
			}
		}
#endif
		i = 0;
		while(buckets[i].last == 0) {i++; continue;}
		//buckets[i].last != 0 => buckets[i].head != 0
		global_head = buckets[i].head;

		// connect adjacent link
		while(i < numBuckets - 1)
		{
			int j = i + 1;
			while(buckets[j].head == 0 && j < numBuckets) j++;
			if(buckets[j].head) buckets[i].last->next = buckets[j].head;
			else break; // j >= numBuckets
			i = j;
		}
		for(i = 0; i < numBuckets; i++) {buckets[i].head = 0; buckets[i].last = 0;}
	}
#ifdef __SORT_DEBUG__
	itr = global_head;
	while(itr)
	{
		cout<<itr->pos<<"\t";
		itr = itr->next;
	}
	cout<<endl;
	itr = global_head;
	while(itr)
	{
		cout<<inimg1d[itr->pos]<<"\t";
		itr = itr->next;
	}
#endif
	itr = global_head;
	long i = 0;
	while(itr)
	{
		outimg1d[i++] = inimg1d[itr->pos];
		itr = itr->next;
	}
	if(sortdata) {delete [] sortdata; sortdata = 0; }
	if(buckets)  {delete [] buckets; buckets = 0;}
	return true;
}

#ifndef __LONG_ORDER__
template<class T> bool bucket_sort_orders(int * outorder1d, T* inimg1d, long tol_sz)
#else
template<class T> bool bucket_sort_orders(long * outorder1d, T* inimg1d, long tol_sz)
#endif
{
	if(inimg1d == 0 || tol_sz <= 0) return false;
#ifndef __LONG_ORDER__
	if(outorder1d == 0)outorder1d = new int[tol_sz];
#else
	if(outorder1d == 0)outorder1d = new long[tol_sz];
#endif
	// forbid sort of double and float data
	//if(sizeof(T) == sizeof(double) || sizeof(T) == sizeof(float)) return false;
	struct Node{
		T value;
		Node * next;
#ifndef __LONG_ORDER__
		int pos;
#else
		long pos;
#endif
	};
	struct Bucket
	{
		Node * head;
		Node * last;
	};
	int numBuckets = 256; // 16
	int offset = 8; // 4
	Node * sortdata = new Node[tol_sz];
	Bucket* buckets = new Bucket[numBuckets]; for(int i = 0; i < numBuckets; i++){ buckets[i].head = 0; buckets[i].last = 0;}
	T max_value = 0;
	for(int i=0;i<tol_sz;i++)
	{
		sortdata[i].value = inimg1d[i];
		sortdata[i].next = sortdata + i+1;
		sortdata[i].pos = i;
		if(inimg1d[i] > max_value)max_value = inimg1d[i];
		if(inimg1d[i] < 0) 
		{
			delete sortdata;
			delete buckets;
#ifdef __SORT_DEBUG__
			cerr<<"not suitable for negate intensity value"<<endl;
#endif
			return false;
		}
	}
	sortdata[tol_sz -1].next = 0;

	int nloops = (int)(log(max_value)/log(numBuckets)) + 1;

	Node * global_head = sortdata;
	Node * itr = 0;
	for(int loop =1; loop <= nloops; loop++)
	{
		int i = 0;
		itr = global_head;
		while(itr)
		{
			Node * cur_node = itr;
			itr = itr->next;

			int new_buck_id = cur_node->value % numBuckets;
			if(buckets[new_buck_id].head == 0)
			{
				cur_node->next = 0;// buckets[new_buck_id].head;
				buckets[new_buck_id].head = cur_node;
				buckets[new_buck_id].last = cur_node;
			}
			else
			{
				//cur_node->next = buckets[new_buck_id].head;
				//buckets[new_buck_id].head = cur_node;
				buckets[new_buck_id].last->next = cur_node;
				cur_node->next = 0;
				buckets[new_buck_id].last = cur_node;
			}
			cur_node->value >>= offset;
		}
#ifdef __SORT_DEBUG__
		for(i = 0; i < numBuckets; i++)
		{
			if(buckets[i].head)
			{
				cout<<"bucket "<<i<<" : ";
				Node * it = buckets[i].head;
				while(it)
				{
					cout<<inimg1d[it->pos]<<"\t";
					it = it->next;
				}
				cout<<endl;
			}
		}
#endif
		i = 0;
		while(buckets[i].last == 0) {i++; continue;}
		//buckets[i].last != 0 => buckets[i].head != 0
		global_head = buckets[i].head;

		// connect adjacent link
		while(i < numBuckets - 1)
		{
			int j = i + 1;
			while(buckets[j].head == 0 && j < numBuckets) j++;
			if(buckets[j].head) buckets[i].last->next = buckets[j].head;
			else break; // j >= numBuckets
			i = j;
		}
		for(i = 0; i < numBuckets; i++) {buckets[i].head = 0; buckets[i].last = 0;}
	}
#ifdef __SORT_DEBUG__
	itr = global_head;
	while(itr)
	{
		cout<<itr->pos<<"\t";
		itr = itr->next;
	}
	cout<<endl;
	itr = global_head;
	while(itr)
	{
		cout<<inimg1d[itr->pos]<<"\t";
		itr = itr->next;
	}
#endif
	itr = global_head;
	long i = 0;
	while(itr)
	{
		outorder1d[i++] = itr->pos;
		itr = itr->next;
	}
	if(sortdata) {delete [] sortdata; sortdata = 0; }
	if(buckets)  {delete [] buckets; buckets = 0;}
	return true;
}
#endif
