int cpc_load(char *name, char *arg);
void cpc_unload();
struct dsc *cpc_load_dsc(char *name);
void cpc_unload_dsc(struct dsc *dsc);

#define LOADER_LOAD(name,arg) \
	cpc_load(name, arg)

#define LOADER_UNLOAD() \
	cpc_unload()

#define LOADER_LOAD_DSC(name) \
	cpc_load_dsc(name)

#define LOADER_UNLOAD_DSC(dsc) \
	cpc_unload_dsc(dsc)



