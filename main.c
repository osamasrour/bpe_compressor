#include <stdio.h>
#include <string.h>
#define DA_IMPLEMENTATION
#define SV_IMPLEMENTATION
#define LLST_IMPLEMENTATION
#define UTIL_IMPLEMENTATION
#include "util.h"
#define BPE_IMPLEMENTATION
#define STB_DS_IMPLEMENTATION
#include "bpe.h"

enum OPTION{
	OPT_NONE  = -1,
	OPT_UNZIP = 0,
	OPT_ZIP   = 1
};


void usage(FILE* f){
	fprintf(f, "[USAGE]: OPTION[zip | unzip]  <input> <output>\n");
}

void opt_flag_parse(enum OPTION* opt, char const **argv[]){
	const char* opt_p = (*argv)[1];
	if(strcmp(opt_p , "zip") == 0){
		*opt = OPT_ZIP;
		return;
	}
	if(strcmp(opt_p , "unzip") == 0){
		*opt = OPT_UNZIP;
		return;
	}
	*opt = OPT_NONE;
}


int main(int argc, char const *argv[]){
	if(argc < 4){ // file.exe option input output
		usage(stderr);
		exit(1);
	}
	enum OPTION opt;
	opt_flag_parse(&opt, &argv);
	const char* input_path =  argv[2];
	const char* output_path = argv[3];
	if(opt == OPT_UNZIP){
		bpe user = {0};
		bpe_init(&user);
		int ret = bpe_unpack(&user, input_path);
		assert(ret == 0);
		String_View text = SV_create("");
		bpe_decode(&user, &text);
		bpe_destroy(&user);
		FILE* out = fopen(output_path, "w");
		SVEx_write_file(&text, out);
		fclose(out);
		SV_destroy(&text);
	}else if(opt == OPT_ZIP){
		FILE* in = fopen(input_path, "r");
		String_View sv_txt = SV_create("");
		SVEx_read_file(&sv_txt, in);
		fclose(in);
		bpe user = {0};
		bpe_init(&user);
		LLSTu32_from_SV2(&user.compressed, &sv_txt);
		SV_destroy(&sv_txt);
		bpe_encode(&user);
		int ret = bpe_pack(&user, output_path);
		assert(ret == 0);
		bpe_destroy(&user);
	}
	return 0;

}
