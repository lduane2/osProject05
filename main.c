/*
Main program for the virtual memory project.
Make all of your modifications to this file.
You may add or rearrange any code or data as you need.
The header files page_table.h and disk.h explain
how to use the page table and disk interfaces.
*/

#include "page_table.h"
#include "disk.h"
#include "program.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>



int numPages;
int numFrames;
int pageFaults;
int  frame;
int  * bits;
const char *rep;
const char *prog;
char * virtmem;
char * physmem;
struct disk *disk;

int pageFaults = 0;
int diskReads = 0;
int diskWrites = 0;
int currPageNumber = -1;
int frameCounter = 0;
int repeatCounter = 0;
int phase = 1;
int *lru;
int lru_swap = 1;


int * queue;

int r = 0;


void page_fault_handler( struct page_table *pt, int page )
{
        pageFaults++;
	physmem = page_table_get_physmem(pt);
	//phase 1
	if(frameCounter == numFrames) {
		phase = 2;
		frameCounter = 0;
	}
	
	page_table_get_entry(pt,page,&frame,bits);
	
	if(phase == 1) { //populating the frames
		if(bits[0] == 0) { //if it is just read
			frame = frameCounter;
			page_table_set_entry(pt,page,frame, PROT_READ);
			disk_read(disk,page,&physmem[frame*PAGE_SIZE]);
                        diskReads++;
			queue[frame] = page;
                        lru[lru_swap] = frame; //adds to the lru structur
                        lru_swap = 1 - lru_swap; //swaps to add to the other index
			frameCounter++;
		} else {
			page_table_set_entry(pt,page,frame, PROT_READ|PROT_WRITE);
		}
	} else { //phase 2
	
		if (!strcmp(rep,"rand")) { //gets random frame
			r = rand()%numFrames;
			frame = r;
		} else if (!strcmp(rep,"fifo")) { //iterates through and resets to 0 at the end to simulate fifo
			frame = frameCounter;
			frameCounter++;
			if (frameCounter == numFrames) {
				frameCounter = 0;
			}
		} else if (!strcmp(rep,"custom")) {
			//make up algorithm -- least recently used
                        frame = lru[lru_swap];
                        lru_swap = 1 - lru_swap;
		} else { //error handling for improper algorithm input
                    printf("You do not have a valid algorithm\n");
		    printf("use: virtmem <npages> <nframes> <rand|fifo|lru|custom> <sort|scan|focus>\n");
		    exit(1);
	        }          
		
		currPageNumber = queue[frame]; //gives the page of the frame to be evicted
		
		if (bits[0] == 0) { //nothing there
			disk_write(disk, currPageNumber, &physmem[frame*PAGE_SIZE]);
                        diskWrites++;
			disk_read(disk, page, &physmem[frame*PAGE_SIZE]);
                        diskReads++;
			page_table_set_entry(pt,page,frame,PROT_READ);
			page_table_set_entry(pt,currPageNumber,0,0);
			queue[frame] = page;
		} else if (bits[1] == 0) { //read but not write
			page_table_get_entry(pt,page,&frame,bits);
			page_table_set_entry(pt,page,frame, PROT_READ|PROT_WRITE);
		}
		
	}
        
}

int main( int argc, char *argv[] )
{
	srand(time(NULL));
	if(argc!=5) {
		printf("use: virtmem <npages> <nframes> <rand|fifo|lru|custom> <sort|scan|focus>\n");
		return 1;
	}
        int npages, nframes;
	if(atoi(argv[1]) && atoi(argv[2])){
            npages = atoi(argv[1]);
	    nframes = atoi(argv[2]);
        } else {
            printf("Your pages or frames are not numbers\n");
            exit(1);
        }
	const char *repAlg = argv[3];
	const char *program = argv[4];
	
    	bits = malloc(3*sizeof(int));
        int b;
        for(b = 0; b < 3; b++){
            bits[b] = 0; 
        }

	
	numPages = npages;
	numFrames = nframes;
	if (numFrames > numPages) { //handles instance where numFrames is greater than numPages
		numFrames = numPages;
	}
        //allocate and populate lru
        lru = malloc(2*sizeof(int));
        lru[0] = 0;
        lru[1] = 1;
	
	queue = malloc(numPages*sizeof(int));
	int i;
	for (i = 0; i < numPages; i += 1) {
		queue[i] = 0;
	}
	rep = repAlg;
	prog = program;

	disk = disk_open("myvirtualdisk",npages);
	if(!disk) {
		fprintf(stderr,"couldn't create virtual disk: %s\n",strerror(errno));
		return 1;
	} 


	struct page_table *pt = page_table_create( npages, nframes, page_fault_handler );
	if(!pt) {
		fprintf(stderr,"couldn't create page table: %s\n",strerror(errno));
		return 1;
	}

	
	virtmem = page_table_get_virtmem(pt);
	physmem = page_table_get_physmem(pt);

	if(!strcmp(program,"sort")) {
		sort_program(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"scan")) {
		scan_program(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"focus")) {
		focus_program(virtmem,npages*PAGE_SIZE);

	} else {
		fprintf(stderr,"unknown program: %s\n",argv[3]);
		return 1;
	}

	page_table_delete(pt);
	disk_close(disk);

	free(queue);
	free(bits);
        free(lru);
        printf("Page Faults: %d\nDisk Reads: %d\nDisk Writes: %d\n", pageFaults, diskReads, diskWrites);
	return 0;
}

