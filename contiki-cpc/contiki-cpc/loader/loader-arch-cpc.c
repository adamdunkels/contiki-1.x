#include "loader-arch.h"
#include "rel.h"
#include <stddef.h>
#include <malloc.h>

extern void *progend;

struct prg_hdr {
	char *relocatedata;
	char arch[8];
	char version[8];
	char initfunc[1];
};

struct dsc_hdr {
	char *relocatedata;
	struct dsc dscdata;
};

unsigned char loader_arch_load(const char *name, char *arg)
{
	char *loadaddr;	
	struct prg_hdr *prghdr;
	int length;

	/* get length of file */
	length = get_file_length(name);
	if (length==0)
		return LOADER_ERR_OPEN;

	/* allocate memory */
	loadaddr = malloc(length);
	if (loadaddr==NULL)
		return LOADER_ERR_MEM;
	
	/* load the file */
	load_file(name,loadaddr);

	prghdr = (struct prg_hdr *)loadaddr;

	/* relocate it */
	relocate(prghdr->relocatedata,loadaddr);

	((void (*)(char *))prghdr->initfunc)(arg);

	return LOADER_OK;
}

struct dsc *loader_arch_load_dsc(const char *name)
{
	char *loadaddr;
	struct dsc_hdr *dschdr;
	int length;

	/* get length of file */
	length = get_file_length(name);
	if (length==0)
		return NULL;

	/* allocate memory */
	loadaddr = malloc(length);
	if (loadaddr==NULL)
		return NULL;

	/* load the file */
	load_file(name, loadaddr);
	
	dschdr = (struct dsc_hdr *)loadaddr;
	/* relocate it */
	relocate(dschdr->relocatedata, loadaddr);

	return &dschdr->dscdata;
}

void loader_arch_free(void *loadaddr)
{
	/* free module */
	/* we're given the start of 'arch' member of the prg_hdr,
	calculate the real start address and then free the block */
	void *header = (void *)((char *)loadaddr - offsetof(struct prg_hdr,arch));
	free(header);
}

void loader_arch_free_dsc(struct dsc *dscdata)
{
	/* we're given the start of 'dsc' member of the dsc_hdr,
	calculate the real start address and then free the block */
	void *header = (void *)((char *)dscdata - 2);
//offsetof(struct 
//dsc_hdr,dscdata));
	free(header);
}

