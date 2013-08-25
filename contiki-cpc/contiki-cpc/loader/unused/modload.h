void mod_free(struct dsc *);

struct mod_ctrl
{ 
	void *read;
	void *module;
	int callerdata;
}; 

#define MLOAD_OK 0

int mod_load(struct mod_ctrl *);

