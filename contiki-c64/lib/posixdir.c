/*
 * Copyright (c) 2004, Adam Dunkels.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 *
 * This file is part of the Contiki operating system.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: posixdir.c,v 1.2 2004/09/12 20:30:04 adamdunkels Exp $
 */
/* posixdir.c written by groepaz */


// define to compile test program
//#define TEST
// define to compile test program for linux :=P
//#define LINUX

/*
	first test for posix directory routines for the c64
	kludges:
	-   currently uses cbm_open, which conflicts with standard i/o,
		which in turn makes it infact kindof unuseable. this can
		be easily changed however, since the only reason not to use
		open/read was that it currently appends ,u,r to filenames
	-   the offset in current dir stream should better be calculated
		from the values returned by "read".
	-   the type flag isnt filled in atm.	
	-   scandir/alphasort/versionsort is missing
	-   some bits are currently untested (ie, unused in the testprogram)
	27/02/2003 gpz	
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#if defined(LINUX)
#include <sys/types.h>
#include <dirent.h>
#else

/*
	contents of dirent.h ...
*/

#include <cbm.h>

/* 16 will only be enough if NO subdirs are supported! */
#define NAME_MAX 16

struct dirent
{
	/* POSIX */
	char d_name[NAME_MAX+1];
	/* linux/bsd extensions */
//	off_t          d_off;
	unsigned short d_off;
	unsigned short d_reclen;
	unsigned char  d_type;

	/* bsd extensions */
	unsigned char  d_namlen;

};

/* fields in the dir struct shouldnt be accessed by user */
typedef struct 
{
	unsigned char fd;
	//off_t		  off;
	unsigned short off;
	char name[NAME_MAX+1];
} DIR;

DIR *opendir(const char *name);
struct dirent *readdir(DIR *dir);
int closedir(DIR *dir);
off_t telldir(DIR *dir);
int dirfd(DIR *dir);
void seekdir(DIR *dir, off_t offset);
void rewinddir(DIR *dir);

/*

int scandir(const char *dir, struct dirent ***namelist,
              int(*select)(const struct dirent *),
              int(*compar)(const struct dirent **, const struct dirent **));

int alphasort(const void *a, const void *b);
int versionsort(const void *a, const void *b);

*/

//-------------------------------------------------------------------------------------------
// functions access the directory via reading the "$" file to provide max. compatibility
//
// disc header:
//
// $07      ignored
// $01      quote
// $10      volume/subdir name
// $01      quote
// $06      disc id
// $01      = $00
//
// file entry:
//
// $02      ignored (basic-line link)
// $02      file length in 254-byte blocks (basic-line nr)
// $xx      spaces
//
// $01      quote if file-entry...                   | = 'b' if "blocks free"...
//          $00-$10  file-name                       |   $xx ignored
//          $01      quote                           |
//          $01      file-closed attrib (' 'or'*')   |   => file-length is free blocks
//          $03      file-type                       |
//          $01      file-protected attr. (' 'or'<') |
//          $xx      ignored                         |
//          $01      = $00 (basic-line end)          |
//-------------------------------------------------------------------------------------------

DIR *opendir(const char *name){
unsigned char buffer[(8+16+1+7)];
unsigned char fd;
DIR *dir;

	dir=(DIR*) malloc(sizeof(DIR));
	if(dir!=NULL)
	{
		/* handle dir name, go into
		   subdirs on 1581 etc... */
		name=name;
		
		// open dir "file"
//		if((fd=dir->fd=open("$",O_RDONLY))==-1)
		if(cbm_open(2, 8, CBM_READ, "$")!=0)
		{
			return(NULL);
		}

		fd=dir->fd=2;
		dir->off=0;
		
		// skip the disc header
		
		//read(fd,buffer,0x08);    // some info
		//read(fd,buffer,0x10);    // disc name
		//read(fd,buffer,0x01);    // quote
		//read(fd,buffer,0x07);    // disc id + $00
	
		cbm_read(fd,buffer,(8+16+1+7));  

	}
	return(dir);
	
}

int closedir(DIR *dir)
{
	cbm_close(dir->fd);
	free(dir);
}

int dirfd(DIR *dir)
{
	return(dir->fd);
}

off_t telldir(DIR *dir)
{
	return((off_t)(dir->off));
}

void seekdir(DIR *dir, off_t offset)
{
static unsigned char ch;
unsigned char fd;
char name[NAME_MAX+1];
	
	if(offset<dir->off)
	{
		/* close and re-open */
		strcpy(name,dir->name);
		closedir(dir);
		dir=opendir(name);
		
	}
	
	fd=dir->fd;
	
	/* scan forward */
	while(dir->off!=offset)
	{
		cbm_read(fd,&ch,0x01);
		dir->off=dir->off+1;    
	}
	
}

void rewinddir(DIR *dir)
{
	seekdir(dir,(off_t)0);
}

struct dirent *readdir(DIR *dir)
{

unsigned char buffer[0x40];
unsigned char temp;
unsigned char i,ii;

static struct dirent entry;
unsigned char fd;
static unsigned char ch;

	 	fd=dirfd(dir);
		entry.d_off=dir->off;

		// basic line-link / file-length
		cbm_read(fd,buffer,0x04);
		dir->off=dir->off+4;    
		/*
		len=(buffer[2]+(buffer[3]<<8));
		blen=len*254;
		*/
		entry.d_reclen=254*(buffer[2]+(buffer[3]<<8));

		// read file entry
		i=0;do
		{
			cbm_read(fd,&ch,0x01);    
			buffer[i]=ch;
			++i;

		} while (ch!=0);
			
		dir->off=dir->off+i;    

		// skip until either quote (file) or b (blocks free => end)
		i=0;ii=0;
		while(i==0){
			temp=buffer[ii];ii++;
			if(ii>16){
				/* something went wrong...this shouldnt happen! */
				return(NULL);
			}
			else if(temp=='\"') i++;
			else if(temp=='b') {
				/* "blocks free" */
				return(NULL);
			}
		}

		/* process file entry */

		i=0;  temp=buffer[ii];ii++;
		while(temp!='\"'){
			entry.d_name[i]=temp;
			i++;
			temp=buffer[ii];ii++;
		}
		entry.d_name[i]=0;
		entry.d_namlen=i;

		/*
		ii=ii+(16-i);                           // pad
		cattr=buffer[ii];ii++;                  // file closed attribute
		ext[0]=buffer[ii];ii++;                 // filetype (text)
		ext[1]=buffer[ii];ii++;
		ext[2]=buffer[ii];ii++;
		pattr=buffer[ii];ii++;                  // file protected attribute
		*/
		
		/* set type flag */

		return(&entry);

}
#endif

#if defined (TEST)
int main(void)
{
char mydirname[NAME_MAX+1]=".";
DIR *mydir;
struct dirent *mydirent;

	mydir=opendir(mydirname);
	if(mydir==NULL)
	{
		printf("error:\n");
	}
	else
	{
		printf("contents of \"%s\" dirfd:%d\n",mydirname,dirfd(mydir));
		while((mydirent=readdir(mydir))!=NULL)
		{
			printf("dirent.d_name[] : \"%s\"\n",mydirent->d_name);
			#if !defined(LINUX)
			printf("dirent.d_namlen : %10d\n",mydirent->d_namlen);
			#endif
			printf("dirent.d_reclen : %10d\n",mydirent->d_reclen);
			printf("dirent.d_type   : %10d\n",mydirent->d_type);
			printf("dirent.d_off    : %10d\n",(unsigned short)mydirent->d_off);
			printf("telldir()       : %10d\n",(unsigned short)telldir(mydir));
			printf("---\n");
		} 
		closedir(mydir);
	}

} 
#endif
