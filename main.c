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



//struct disk *disk = disk_open("myvirtualdisk",npages);
int numPages;
int numFrames;
int pageFaults;
int  frame;
int  * bits;
const char *rep;
const char *prog;
char * virtmem;
char * physmem;
//struct disk *gdisk;
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
/*
int find_least_used(){
        int i;
        int min = -10000000; int curr; int minframe = 0;
        for(i = 0; i < numFrames; i = i + 1) {
                curr = queue[i];
                if(lfu[curr] > min ) {
                    min = lfu[curr];
                    minframe = i;
                }
        }
        //printf("%d\n", minframe);
        return minframe;
}
*/


void page_fault_handler( struct page_table *pt, int page )
{
        pageFaults++;
	//printf("page fault on page #%d\n",page);
	//virtmem = page_table_get_virtmem(pt);
	physmem = page_table_get_physmem(pt);
	//phase 1
	//page_table_get_entry(pt,page,&frame,&bits);
	if(frameCounter == numFrames) {
		//printf("We are here \n");
		phase = 2;
		frameCounter = 0;
	}
	
	page_table_get_entry(pt,page,&frame,bits);
	
	if(phase == 1) {
		if(bits[0] == 0) {
			frame = frameCounter;
			page_table_set_entry(pt,page,frame, PROT_READ);
			disk_read(disk,page,&physmem[frame*PAGE_SIZE]);
                        diskReads++;
			queue[frame] = page;
                        lru[lru_swap] = frame;
                        lru_swap = 1 - lru_swap;
			//printf("queue[%d] = %d \n",frame, page);
			frameCounter++;
		} else {
			page_table_set_entry(pt,page,frame, PROT_READ|PROT_WRITE);
		}
	} else { //phase 2
		//page_table_set_entry(pt,page,frame, PROT_READ|PROT_WRITE);
		//return 0;
	
		//printf("%s \n", rep);
		if (!strcmp(rep,"rand")) {
			//srand(time(NULL));
			r = rand()%numFrames;
			frame = r;
			//printf("frame: %d \n",r);
		} else if (!strcmp(rep,"fifo")) {
			frame = frameCounter;
			//printf("frame: %d \n", frame);
			frameCounter++;
			if (frameCounter == numFrames) {
				frameCounter = 0;
			}
		} else if (!strcmp(rep,"custom")) {
			//make up algorithm
                        //printf("frame: %d \n", frame);
                        frame = lru[lru_swap];
                        lru_swap = 1 - lru_swap;
		} else {
		    printf("use: virtmem <npages> <nframes> <rand|fifo|lru|custom> <sort|scan|focus>\n");
		    exit(1);
	        }          
		
		//printf("here \n");
		
		currPageNumber = queue[frame];
		
		if (bits[0] == 0) { //nothing there
			disk_write(disk, currPageNumber, &physmem[frame*PAGE_SIZE]);
                        diskWrites++;
			disk_read(disk, page, &physmem[frame*PAGE_SIZE]);
                        diskReads++;
			page_table_set_entry(pt,page,frame,PROT_READ);
			page_table_set_entry(pt,currPageNumber,0,0);
			queue[frame] = page;
			//printf("queue[%d] = %d \n",frame, page);
		} else if (bits[1] == 0) { //read but not write
			//printf("entered \n");
			page_table_get_entry(pt,page,&frame,bits);
			page_table_set_entry(pt,page,frame, PROT_READ|PROT_WRITE);
		}
		
	}
        
	//exit(1);
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
	
	numPages = npages;
	numFrames = nframes;
	if (numFrames > numPages) {
		numFrames = numPages;
	}
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

	//struct disk *disk = disk_open("myvirtualdisk",npages);
	disk = disk_open("myvirtualdisk",npages);
	if(!disk) {
		fprintf(stderr,"couldn't create virtual disk: %s\n",strerror(errno));
		return 1;
	} 
	/*
	else {
		gdisk = disk;	
	}*/


	struct page_table *pt = page_table_create( npages, nframes, page_fault_handler );
	if(!pt) {
		fprintf(stderr,"couldn't create page table: %s\n",strerror(errno));
		return 1;
	}

	
	//char * virtmem = page_table_get_virtmem(pt);
	//char * physmem = page_table_get_physmem(pt);
	
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
        printf("Page Faults: %d\nDisk Reads: %d\nDisk Writes: %d\n", pageFaults, diskReads, diskWrites);
	return 0;
}

