#include <stdio.h>

#define DA_IMPLEMENTATION
#define SV_IMPLEMENTATION
#define LLST_IMPLEMENTATION
#define UTIL_IMPLEMENTATION
#include "util.h"
#define BPE_IMPLEMENTATION
#define STB_DS_IMPLEMENTATION
#include "bpe.h"

int main(int argc, char const *argv[]){
	if(argc < 2){
		fprintf(stderr, "[INFO]: USAGE: <%s> <input.txt>\n", argv[0]);
		exit(1);
	}
	const char* input_path =  argv[1];
	// const char* output_path = argv[2];
	bpe user = {0};
	bpe_init(&user);
	assert(bpe_unpack(&user, input_path) == 0);
	String_View text = SV_create("");
	bpe_decode(&user, &text);
	return 0;

}


int main2(int argc, char const *argv[]){
	if(argc < 3){
		fprintf(stderr, "[INFO]: USAGE: <%s> <input.txt> <output.txt>\n", argv[0]);
		exit(1);
	}
	const char* input_path =  argv[1];
	const char* output_path = argv[2];
	printf("[INFO] Excuting BPE on %s\n", input_path);
	FILE* f2 = fopen(input_path, "r");
	if(f2 == NULL){
		fprintf(stderr, "%s\n", strerror(errno));
		return 1;
	}

	String_View sv_txt = SV_create("");
	SVEx_read_file(&sv_txt, f2);
	fclose(f2);
	printf("Number Of Chars in the FILE = %llu\n", sv_txt.length);
	LLST ll_txt = LLST_create(sizeof(uint32_t));
	LLSTu32_from_SV2(&ll_txt, &sv_txt);
	printf("Number of elements in the ll_txt = %u\n", ll_txt.length);
#define LOG 0
#if LOG
	llst_log(ll_txt);
#endif
	bpe user = {0};
	memcpy(user.header, 
			(char[BPE_HEADER_CAP]){'B', 'P', 'E', '0', 'L', 'E'}, 
			BPE_HEADER_CAP);
	printf("[INFO]: HEADER => %6s\n", user.header);
	user.compressed = ll_txt;
	user.freqs = DA_create_array(sizeof(freq), 256, 256);
	bpe_encode(&user);
	printf("[INFO] user.compressed.length = %u\n", user.compressed.length);
	printf("[INFO] sizeof(freq) = %llu\n", sizeof(freq));
	assert(bpe_pack(&user, output_path) == 0);
#if LOG
	llst_log(ll_txt);
#endif
	SV_destroy(&sv_txt);
	return 0;
}

