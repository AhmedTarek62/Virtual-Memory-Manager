// Group4.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <fstream>
#include<iostream>
using namespace std;
const int framesize = 256;
const int logicaladdresssize = 16;
const int physicaladdresssize = 16;
const int logicaladdressspace = 65536;//maximum possible number 2^16..I will use the same variable for logical and physical address space
//as they have the same size
const int offsetsize = 8;
const int pagenumbersize = 8;
const int pagetablesize = 256;
int logicaladdresses[logicaladdressspace];//Array of logical addresses
int physicaladdresses[logicaladdressspace];//Array of physical addresses where the ith element is the physical address of the ith logical address
int pagenumbers[logicaladdressspace];//array of page numbers where the ith element is the page number of the ith logical address
int offsets[logicaladdressspace];//array of offsets where the ith element is the offset of the ith logical address
char *diskcontent = new char[logicaladdressspace];//a buffer for reading the binary file
int pagemask = (1 - pow(2,logicaladdresssize) )/ (1 - 2) - (1 - pow(2 ,offsetsize) )/ (1 - 2);
int offsetmask= (1 - pow(2,offsetsize))/ (1 - 2);
//I used the formula of the summation of the geometric series to calculate the offset since a binary number can be written as
//a geometric series with ratio 2
struct frame {
	int framenumber;
	bool validity;
};
const int TLBSize = 16;
struct TLBStructure {
	int framenumber[TLBSize];
	int pagenumber[TLBSize];
};
frame pagetable[pagetablesize];//an array of structures of type frame 
bool emptyframes[pagetablesize];//a boolean array to keep track of empty frames if emptyframes[i]=1 then the ith frame is empty 
int physicalmemory[logicaladdressspace];//an array to represent the physical memory where physicalmemory[i] is value in the i-th byte
TLBStructure TLB;
int main()
{
	ifstream AddressesFile;
	ifstream MemoryFile;
	ofstream ContentFile;
	AddressesFile.open("C:\\Users\\ahmed\\source\\repos\\Group4\\addresses.txt");//opens the logical addresses file
	//please change file path before running
	if (!AddressesFile) {
		cerr << "Unable to open file addresses.txt";
		exit(1);   // call system to stop
	}
	int temp;//a swapping temporary variable
	int i = 0;//counter to get the array size
	while (AddressesFile >> temp) {
		logicaladdresses[i] = temp;
		i++;
	}//assigns each logical address to an element of the array
	AddressesFile.close();
	int currentnumberofaddresses = i;//logicaladdresses array size tp avoid looping on the whole logical address space in the future
	AddressesFile.close();
	for (int i = 0; i < currentnumberofaddresses; i++) {
		pagenumbers[i] = (logicaladdresses[i] & pagemask) >> offsetsize;
	}//masks each element in the logical addresses array and shifts it to the right by 'offsetsize' bits then assigns it to the pagenumbers array

	for (int i = 0; i < currentnumberofaddresses; i++) {
		offsets[i] = logicaladdresses[i] & offsetmask;
	}//masks each element in the logical addresses array

	MemoryFile.open("C:\\Users\\ahmed\\source\\repos\\Group4\\BACKING_STORE.bin", ios::binary);//opens the binary file
	//please change the file path before running
	if (!MemoryFile) {
		cerr << "Unable to open file BACKING_STORE.bin ";
		exit(1);   // call system to stop
	}
	MemoryFile.read(diskcontent, logicaladdressspace);//reads the binary file and writes it to diskcontent buffer
	MemoryFile.close();

	for (int i = 0; i < pagetablesize; i++) {
		pagetable[i].framenumber = -1;
		pagetable[i].validity = 0;
	}//initializes the page table 
	for (int i = 0; i < pagetablesize; i++) {
		emptyframes[i] = 1;
	}//initializes the frames to be all empty
	for (int i = 0; i < TLBSize; i++) {
		TLB.framenumber[i] = -1;
		TLB.pagenumber[i] = -1;
	}//initializes the frames and page numbers to -1
	int counter = 0;//to count the number of replacements that took place in the TLB
	int TLBHitRate = 0;
	int PageFaultRate = 0;
	for (int i = 0; i < currentnumberofaddresses; i++) {
		for (int j = 0; j < TLBSize; j++) {
			if (TLB.pagenumber[j] == pagenumbers[i]) {
				TLBHitRate++;
				physicaladdresses[i] = TLB.framenumber[j] * framesize + offsets[i];
				goto label;
			}//checks if the page is in the TLB using linear search and if the page is found it accesses the physical memory
		}
		if (pagetable[pagenumbers[i]].validity == 0) {
			PageFaultRate++;
			for (int k = 0; k < pagetablesize; k++) {
				if (emptyframes[k] == 1) {
					emptyframes[k] = 0;
					pagetable[pagenumbers[i]].validity = 1;
					pagetable[pagenumbers[i]].framenumber = k;
					physicaladdresses[i] = pagetable[pagenumbers[i]].framenumber * framesize + offsets[i];
					for (int m = 0; m < TLBSize; m++) {
						if (TLB.pagenumber[m] == -1) {
							TLB.framenumber[m] = k;
							TLB.pagenumber[m] = pagenumbers[i];
							goto label;
						}//searches for empty entries in the TLB to assign the page
					}
					TLB.framenumber[counter % 16] = k;
					TLB.pagenumber[counter % 16] = pagenumbers[i];
					counter++;//replaces the first page to enter the TLB
					goto label;

				}//assigns the first empty frame to the the page number if it the validity bit is 0 which means that no frames has been
				 //assigned to this page and adds the page and the frame number to the TLB in the first empty slot..If there is no empty
				 //slots it replaces the first page to enter the TLB

			}
		}
		else {
			TLB.framenumber[counter % 16] = pagetable[pagenumbers[i]].framenumber;
			TLB.pagenumber[counter % 16] = pagenumbers[i];
			counter++;//replaces the first page to enter the TLB
			physicaladdresses[i] = pagetable[pagenumbers[i]].framenumber * framesize + offsets[i];
		}
		label:		physicalmemory[physicaladdresses[i]] = (int)diskcontent[logicaladdresses[i]]; //copies the content from the disk to the memory in the
				//newly assigned physical address 

	}

		for (int i = 0; i < currentnumberofaddresses; i++) {
			cout << "Virtual Address is " << logicaladdresses[i] << " Physical Address is " << physicaladdresses[i] << " Value is " << physicalmemory[physicaladdresses[i]] << endl;
		}//output to cmd

		ContentFile.open("C:\\Users\\ahmed\\source\\repos\\Group4\\output.txt");//opens the output file..Please the change the file path before running
		for (int i = 0; i < currentnumberofaddresses; i++) {
			ContentFile << "Virtual Address is " << logicaladdresses[i] << " Physical Address is " << physicaladdresses[i] << " Value is " << physicalmemory[physicaladdresses[i]] << endl;
		}
		//output to the output file
		ContentFile.close();
		cout << "TLB Hit Rate is " << TLBHitRate / 10 << "% " << endl << "Page Fault Rate is " << PageFaultRate / 10 << "% " << endl;
		return 0;
	}
