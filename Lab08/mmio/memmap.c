/*
 * Example of using mmap. Taken from Advanced Programming in the Unix
 * Environment by Richard Stevens.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h> /* mmap() is defined in this header */
#include <fcntl.h>
#include <unistd.h>
#include <string.h>  /* memcpy */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

void err_quit (const char * mesg)
{
	printf ("%s\n", mesg);
	exit(1);
}

void err_sys (const char * mesg)
{
	perror(mesg);
	exit(errno);
}

int main (int argc, char *argv[])
{
	int fdin, fdout, i;
	char *src, *dst, buf[256];
	struct stat statbuf;

	src = dst = NULL;

	if (argc != 3)
		err_quit ("usage: memmap <fromfile> <tofile>");

	/* 
	 * open the input file 
	 */
	if ((fdin = open (argv[1], O_RDONLY)) < 0) {
		sprintf(buf, "can't open %s for reading", argv[1]);
		perror(buf);
		exit(errno);
	}

	/* 
	 * open/create the output file 
	 */
	//if ((fdout = open (argv[2], O_RDWR | O_CREAT | O_TRUNC, 0644)) < 0) {
	//	sprintf (buf, "can't create %s for writing", argv[2]);
	//	perror(buf);
	//	exit(errno);
	//}

	/* 
	 * 1. find size of input file 
	 */
	// File information is accesible using the fstat call.
	// The stat structure holds metrics and statistics for any given file.
	// When we call fstat, the pointer passed in is set to look at the
	// struct.
	if(0 > fstat(fdin, &statbuf)){
		sprintf(buf, "can't access fstat info for %s", argv[2]);
		perror(buf);
		exit(errno);
	}

	/* 
	 * 2. go to the location corresponding to the last byte 
	 */
	// We have our file descriptors open, but there is a problem if we try
	// to memory map immediately.  We opened the file descriptor with the
	// O_TRUNC flag to make sure there isn't any extraneous data present.
	// In doing so, all the bytes are designated as invlaid by the
	// filesystem, so trying to access them will not yet work.  Even if we
	// do the memory mapping, it is independent of the writeable state of
	// the disk; the individual bytes are still considered invalid.  We
	// need to manually find the end of the file and write a dubby byte,
	// or use ftruncate to adjust the valid size to the desired file size.
	// This was a surprisingly hard bit of explanation to find.
	
	// lseek lets us navigate a "cursor" through our file handle.
	// SEEK_SET macro designates that we want an absolute offset from the
	// beginning of the file, as opposed to a relative offset.  In the
	// case of a relative offset, the third argument is "from whence," a
	// comical application of old english that means with respect to here.
	if(0 > lseek(fdout, statbuf.st_size, SEEK_SET)){
		sprintf(buf, "failed to seek properly on output file");
		perror(buf);
		exit(errno);
	}

	/* 
	 * 3. write a dummy byte at the last location 
	 */
	if(0 > write(fdout, "", 1)){
		sprintf(buf, "failed to initialize write in output file");
		perror(buf);
		exit(errno);
	}

	/* 
	 * 4. mmap the input file 
	 */

	// mmap signature: (physical start address, size of map, RW
	// permissions, flags, file descriptor, offset in bytes)
	//
	// NULL is typically passed to let the OS find the spot where the
	// memory will live in the page table.  That said, according to the
	// man pages, any address you pass will be used as a suggestion, so it
	// may be difficult to pin down exactly where it will land a priori
	//
	// MAP_SHARED is one of many specifications for whether or not updates
	// made in this context should be visible to other processes (as a
	// part of a copy on write scheme).  Here, we use shared, indicating
	// that changes should be carried out on disk as well as in memory
	if(0 > (src = mmap(NULL, statbuf.st_size, PROT_READ, MAP_SHARED, fdin, 0) ) ){
		sprintf(buf, "failed to map input file to DRAM");
		perror(buf);
		exit(errno);
	}

	/* 
	 * 5. mmap the output file 
	 */
	if(0 > (dst = mmap(NULL, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fdout, 0) ) ){
		sprintf(buf, "failed to map input file to DRAM");
		perror(buf);
		exit(errno);
	}
	close(fdin);
	close(fdout);


	/* 
	 * 6. copy the input file to the output file 
	 */
		/* Memory can be dereferenced using the * operator in C.  This line
		 * stores what is in the memory location pointed to by src into
		 * the memory location pointed to by dest.
		 */
		//*dst = *src;
		//
		// This results in a segfault, because there is no information
		// bounding what is written

	// One solution is to copy byte by byte...
	//for(int i = 0; i<statbuf.st_size; ++i){
	//	*(dst + i) = *(src + i);
	//}
	
	// Another is to use memcopy...
	memcpy(dst, src, statbuf.st_size);
}


