/*
 ============================================================================
 Name        : program3.c
 Date        : April 20,2017
 Authors     : Marbo Cheng
 Description : A program that acts as a virtual memory management system
 ============================================================================
 Compile code: gcc -o program3 program3.c
 run: ./program3 addresses.txt
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define totalNumberOfFrames 256
#define pageTablesize 256
#define frameSize 256
#define tlbSize 16
#define addressMask  0xFFFF
#define offsetMask  0xFF

int pageTableNum[pageTablesize];
int pageTableFrames[pageTablesize];

int TLBPageNum[tlbSize];
int TLBFrameNum[tlbSize];

int physicalMemory[totalNumberOfFrames][frameSize]; // physical memory 2D array
int pageFaults = 0;   // page faults counter
int TLBHits = 0;      // TLB hits counter
int firstAvailableFrame = 0;  // keep track the first available frame
int firstAvailablePageTableNumber = 0;  // keep track the first available page table entry
int numOfTLBEntries = 0;             // counter for number of entries in the TLB


#define bufferSize         10

// number of bytes to read
#define data               256

//address file and backstore file
FILE    *addressFile;
FILE    *backingStore;


char    address[bufferSize];
int     logical_address;

//buffer with reads from backing store
signed char     buffer[data];

// the value of the byte (signed char) in memory
signed char     value;



void TLBInsert(int pageNumber, int frameNumber);
void retrievePage(int address);
void storeRetrieve(int pageNumber);

/*-------------------------------------------------------------------
 * Function:    main
 * Purpose:     open files and get physical address and value for each
                logical address in addresses.txt
 * In args:     integer arg
                char *argv[] - reads in file from command line
 * Outputs:
 */
int main(int argc, char *argv[])
{

    // open backing store file
    backingStore = fopen("BACKING_STORE.bin", "rb");

    // open the file containing the logical addresses
    addressFile = fopen(argv[1], "r");

    int totalNumOfTranslatedAddresses = 0;
    // read through the address file and output each logical address
    while ( fgets(address, bufferSize, addressFile) != NULL) {
        logical_address = atoi(address);

        // get physical address and value stored at that address
        retrievePage(logical_address);
        totalNumOfTranslatedAddresses++;
    }

    // calculate page fault rate and TLB hit rate
    double pageFaultRate = pageFaults / (double)totalNumOfTranslatedAddresses;
    double TLBRate = TLBHits / (double)totalNumOfTranslatedAddresses;

    //printf("Total # of translated addresses: %d\n", totalNumOfTranslatedAddresses);
    //printf("Page Faults = %d\n", pageFaults);
    printf("Page Fault Rate = %.3f\n",pageFaultRate);
    //printf("TLB Hits = %d\n", TLBHits);
    printf("TLB Hit Rate = %.3f\n", TLBRate);

    fclose(addressFile);
    fclose(backingStore);

    return 0;
}


/*-------------------------------------------------------------------
 * Function:    TLB insert
 * Purpose:     takes in pageNumber and frameNumber and insert into TLB with
                First In First Out replacement method
 * In args:     integer pageNumber
                integer frameNumber
 * Outputs:
 */

void TLBInsert(int pageNumber, int frameNumber){

    int i;  // if it's already in the TLB, break
    for(i = 0; i < numOfTLBEntries; i++){
        if(TLBPageNum[i] == pageNumber){
            break;
        }
    }

    // if the number of entries is equal to the index
    if(i == numOfTLBEntries){

        //if TLB has space insert page and frame on the end
        if(numOfTLBEntries < tlbSize){
            TLBPageNum[numOfTLBEntries] = pageNumber;
            TLBFrameNum[numOfTLBEntries] = frameNumber;
        }

        //else move everything and insert page/frame at the end
        else{
            for(i = 0; i < tlbSize - 1; i++){
                TLBPageNum[i] = TLBPageNum[i + 1];
                TLBFrameNum[i] = TLBFrameNum[i + 1];
            }
            TLBPageNum[numOfTLBEntries-1] = pageNumber;
            TLBFrameNum[numOfTLBEntries-1] = frameNumber;
        }
    }

    // if the index is not equal to the number of entries
    else{
      //go through the array and move one over
        for(i = i; i < numOfTLBEntries - 1; i++){
            TLBPageNum[i] = TLBPageNum[i + 1];
            TLBFrameNum[i] = TLBFrameNum[i + 1];
        }

        //if there is space put page/frame at the end
        if(numOfTLBEntries < tlbSize){
            TLBPageNum[numOfTLBEntries] = pageNumber;
            TLBFrameNum[numOfTLBEntries] = frameNumber;
        }
        //else pt page/frame number on # of entries -1
        else{
            TLBPageNum[numOfTLBEntries-1] = pageNumber;
            TLBFrameNum[numOfTLBEntries-1] = frameNumber;
        }
    }
    //if there is space in array, increase number of entries
    if(numOfTLBEntries < tlbSize){
        numOfTLBEntries++;
    }
}

/*-------------------------------------------------------------------
 * Function:    retrievePage
 * Purpose:   takes logical address to get physical address
              and value stored at the address
 * In args:     integer logical_address
 * Outputs:     print logical address, physical address, and value
 */
void retrievePage(int logical_address){

    // obtain the page number and offset from the logical address
    int pageNumber = ((logical_address & addressMask)>>8);
    int offset = (logical_address & offsetMask);

    // first try to get page from TLB
    int frameNumber = -1;

    int i;  // look through TLB for a match
    /* look for match in TLB
    */
    for(i = 0; i < tlbSize; i++){
        if(TLBPageNum[i] == pageNumber){   // if the TLB index is equal to the page number
            frameNumber = TLBFrameNum[i];  // then the frame number is extracted
                TLBHits++;                // and the TLBHit counter is increased
        }
    }

    // if frameNumber is not found
    if(frameNumber == -1){
        int i;
        //go through page table
        for(i = 0; i < firstAvailablePageTableNumber; i++){
            //if page is found then get frameNumber
            if(pageTableNum[i] == pageNumber){
                frameNumber = pageTableFrames[i];
            }
        }
        /*if page is not found, there is a page fault so we need to get frame
        into physical memory and page table
        */
        if(frameNumber == -1){
            storeRetrieve(pageNumber);
            pageFaults++;                          // increase # of page faults
            frameNumber = firstAvailableFrame - 1;  // set frameNumber to current firstAvailableFrame index
        }
    }

    //insert page number and page frame into TLB
    TLBInsert(pageNumber, frameNumber);

    // frame number and offset used to get the signed value stored at that address
    value = physicalMemory[frameNumber][offset];

  //printf("offset: %d\n", offset);
  printf("Virtual address: %d Physical address: %d Value: %d\n", logical_address, (frameNumber << 8) | offset, value);
}

/*-------------------------------------------------------------------
 * Function:    storeRetrieve
 * Purpose:    Read from backing store and bring frame into physical memory
               and page table
 * In args:     integer pageNumber
 * Outputs:
 */

void storeRetrieve(int pageNumber){
    // first seek to byte data in the backing store
    // SEEK_SET in fseek() seeks from the beginning of the file
    if (fseek(backingStore, pageNumber * data, SEEK_SET) != 0) {
        fprintf(stderr, "Error seeking data in backing store\n");
    }

    // now read data bytes from the backing store to the buffer
    if (fread(buffer, sizeof(signed char), data, backingStore) == 0) {
        fprintf(stderr, "Error reading data from backing store\n");
    }

    // load the bits into first available frame in the physical memory 2D array
    int i;
    for(i = 0; i < data; i++){
        physicalMemory[firstAvailableFrame][i] = buffer[i];
    }

     //load the frame number into the page table in first available frame
    pageTableNum[firstAvailablePageTableNumber] = pageNumber;
    pageTableFrames[firstAvailablePageTableNumber] = firstAvailableFrame;

    // increase the counters to keep track of the next available frames
    firstAvailableFrame++;
    firstAvailablePageTableNumber++;
}
