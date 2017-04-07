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

int * frametable; //indicates which frames have been used
int * tqueue;

void rand(struct page_table, int);
void fifo(struct page_table, int);
void custom(struct page_table, int);



void page_fault_handler( struct page_table *pt, int page )
{
	printf("page fault on page #%d\n",page);
	
	if (strcmp(prog,"rand")) {
		rand(*pt, page);
	}
	
	if (strcmp(prog,"fifo")) {
		fifo(*pt, page);
	}
	
	if (strcmp(prog,"custom")) {
		custom(*pt,page);
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

void rand(struct page_table *pt, int page) {
	page_table_get_entry(pt,page,&frame,&bits);
	if(bits[0] == 0) { //nothing there
		//access queue of pages
		int i = 0;
		while(table[i] || i != numFrames) i++;
		if(i != numFrames) table[i] = 1;
		//else do random removal
		frame = i;
		//tqueue.push_back(frame);
		page_table_set_entry(pt,page,frame, PROT_READ);
		disk_read(gdisk,page,&physmem(frame*PAGE_SIZE))
	} else if (bits[1] == 0) { //read but not write
		page_table_set_entry(pt,page,frame, PROT_READ|PROT_WRITE);
	}
	
	//add to back of queue
	//generate random number
	//get value from queue at ptr*rand number
	
}

void fifo(struct page_table, int page) {
	//add to
	//push back function or something?
}

void custom(struct page_table, int page) {
	
}


