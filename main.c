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

//struct disk *disk = disk_open("myvirtualdisk",npages);
int numPages;
int numFrames;
int pageFaults;
int  frame;
int * bits;
const char *rep;
const char *prog;
struct disk *gdisk;


int frameCounter = 0;
int phase = 1;

void evict(int);
void load(int);

struct mapTable {
	int VPN = 0;
	int PF = 0;
	//more metadata
}

void page_fault_handler( struct page_table *pt, int page )
{
	printf("page fault on page #%d\n",page);
	
	//phase 1
	//page_table_get_entry(pt,page,&frame,&bits);
	if(frameCounter == numFrames) phase = 2;
	if(phase == 1) {
		frame = fameCounter;
		page_table_set_entry(pt,page,frame, PROT_READ);
		disk_read(gdisk,page,&physmem(frame*PAGE_SIZE));
		frameCounter++;
	} else { //phase 2
		page_table_get_entry(pt,page,&frame,&bits);
		if (strcmp(prog,"rand")) {
			if (bits[1] == 0) { //read but not write
				//get random number
				//get page and frame of that number
				//evict(randpage,randframe);
				//load(page,randframe)
				page_table_set_entry(pt,page,frame, PROT_READ|PROT_WRITE);
			} else { //read and write
				
			}
		}
	
		if (strcmp(prog,"fifo")) {
			if (bits[1] == 0) { //read but not write
				//get frameCounter number
				//evict(frameCounter frame)
				//load(page.frameCounter frame)
				page_table_set_entry(pt,page,frame, PROT_READ|PROT_WRITE);
				frameCounter++;
				if (frameCounter == numFrames) frameCounter = 0;
			}
		}
	
		if (strcmp(prog,"custom")) {
			if (bits[1] == 0) { //read but not write
				page_table_set_entry(pt,page,frame, PROT_READ|PROT_WRITE);
			}
		}
	}

	
	exit(1);
}

int main( int argc, char *argv[] )
{
	if(argc!=5) {
		printf("use: virtmem <npages> <nframes> <rand|fifo|lru|custom> <sort|scan|focus>\n");
		return 1;
	}

	int npages = atoi(argv[1]);
	int nframes = atoi(argv[2]);
	const char *repAlg = argv[3];
	const char *program = argv[4];
	
	numPages = npages;
	numFrames = nframes;
	if (numFrames > numPages) {
		numFrames = numPages;
	}
	
	table = malloc(numPages*sizeof(int));
	int i;
	for (i = 0; i < numPages; i += 1) {
		table[i] = 0;
	}
	rep = repAlg;
	prog = program;

	struct disk *disk = disk_open("myvirtualdisk",npages);
	if(!disk) {
		fprintf(stderr,"couldn't create virtual disk: %s\n",strerror(errno));
		return 1;
	} else {
		gdisk = disk;	
	}


	struct page_table *pt = page_table_create( npages, nframes, page_fault_handler );
	if(!pt) {
		fprintf(stderr,"couldn't create page table: %s\n",strerror(errno));
		return 1;
	}

	char *virtmem = page_table_get_virtmem(pt);

	char *physmem = page_table_get_physmem(pt);

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

	return 0;
}

void evict(int page, int frame) {
	disk_write(gdisk,page, &physmem[frame*PAGE_SIZE]);
	//go into mapTable and update info
}

void load(int page, int frame) {
	disk read();
	//go into mapTable and update info
}

