
#include <stdio.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#define DA_IMPLEMENTATION
#include "da.h"
#define SV_IMPLEMENTATION
#include "sv.h"
#define LLST_IMPLEMENTATION
#include "llst.h"
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"



#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error "TODO: this only work on a little-indian maichine\n"
#endif //! __ORDER_LITTLE_ENDIAN__

#define MINIMUM_REPET_COST 6


typedef struct pair{
	uint32_t l, r;
} pair;

typedef struct pairs_map{
	pair key;
	uint32_t value;
} pairs_map;

typedef struct freq{
	uint32_t node;
	pair leaf;
}freq;


#define BPE_HEADER_CAP 6
typedef struct bpe{
	char header[BPE_HEADER_CAP]; // => BPE0LE -> BPE[version](LE | BE)
	LLST compressed;
	uint32_t highest_id;
	DArray freqs;
}bpe;


FILE* f;
void DA_from_SV(DArray*, String_View*);
void LLST_from_SV(LLST*, String_View*);
void LLSTu32_from_SV2(LLST*, String_View*);
void llst_log(LLST);
// TODO(#2): make it compress algo - encode and decode functions - to compress and decompress the files
// to emprove storage methods, dump the pairs in the hardware with the compressed file so you can
// decompress it back
void bpe_encode(bpe*);

int bpe_store(bpe*, const char*);

int main(int argc, char const *argv[]){
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
	// TODO: we need to dump this array into a file with the compressed text.
	user.freqs = DA_create_array(sizeof(freq), 256, 256);
	bpe_encode(&user);
	printf("[INFO] user.compressed.length = %u\n", user.compressed.length);
	assert(bpe_store(&user, output_path) == 0);
#if LOG
	llst_log(ll_txt);
#endif
	SV_destroy(&sv_txt);
	fclose(f);
	return 0;
}

// Returns a dynamic array of `char*`.
// To cast its elements to `char*`, you need to do: *(char**)DA_get_element(DArray* da, double index)
// It needs to be freed after finishing.
void DA_from_SV(DArray* da, String_View* sv){
	assert(da->length == 0);
	da->capacity = sv->length;
	da->element_size = sizeof(char*);
	da->elements = (char*)realloc(da->elements, sizeof(char*) * da->capacity);
	for(size_t ch = 0; ch < sv->length; ch++){
		char* chars = (char*)malloc(2*sizeof(char));
		assert(chars != NULL);
		chars[0] = SV_get_by_index(sv, ch);
		chars[1] = '\0';
		char** dchars = &chars;
		assert(memcpy(da->elements + (ch * da->element_size), dchars, da->element_size) != NULL);
		da->length++;
	}
}


void LLST_from_SV(LLST* llst, String_View* sv){
	assert(llst->length == 0);
	llst->element_size = sizeof(char*);
	for(size_t ch = 0; ch < sv->length; ch++){
		char* chars = (char*)malloc(2*sizeof(char));
		assert(chars != NULL);
		chars[0] = SV_get_by_index(sv, ch);
		chars[1] = '\0';
		char** dchars = &chars;
		LLST_append(llst, dchars);
	}
}

void LLSTu32_from_SV2(LLST* llst, String_View* sv){
	assert(llst->length == 0);
	llst->element_size = sizeof(uint32_t);
	for(size_t ch = 0; ch < sv->length; ch++){
		uint32_t* _char = (uint32_t*)malloc(sizeof(uint32_t));
		assert(_char != NULL);
		*_char = (uint32_t)SV_get_by_index(sv, ch);
		LLST_append(llst, _char);
	}
}

void llst_log(LLST ll){
	LLNode* current_tk = NULL;
	current_tk = ll.head;
	for(size_t _ = 0; current_tk != NULL; _++){
		fprintf(stdout, "%u,",*(uint32_t*)(current_tk->data));
		current_tk = current_tk->next;
	}
}

// the algo complexity: f(x) = 0.00226*(x*x), which x: size of the file in KB, and f(x): number of seconds
void bpe_encode(bpe* user){
    static uint32_t iter = 0;
    uint32_t higgest_trecker_value = 0;
    
    
    // 1. Initial Scan for Highest ID
    LLNode *current_node = (&user->compressed)->head;
    while(current_node != NULL){
        if(higgest_trecker_value < *(uint32_t*)current_node->data){
            higgest_trecker_value = *(uint32_t*)current_node->data;
        }
        current_node = current_node->next;
    }

    user->highest_id = higgest_trecker_value; // The value of the highest sample we reached in the article.
    printf("\nThe higgest ID: %u\n\n", user->highest_id);

    pairs_map* merge_hash_map_p = NULL;
    pairs_map most_repated_pair = {0};

    // 2. Initial Map Generation
    current_node = (&user->compressed)->head;
    while(current_node != NULL && current_node->next != NULL){
        pair test_pair = {
            .l = *(uint32_t*)(current_node->data), 
            .r = *(uint32_t*)(current_node->next->data)
        };
        pairs_map* pair_p = hmgetp_null(merge_hash_map_p, test_pair);
        if(pair_p == NULL){
            hmput(merge_hash_map_p, test_pair, 1);
        }else{
            pair_p->value++;
        }
        current_node = current_node->next;
    }

    while(1){
        // Find the most frequent pair in the map
        most_repated_pair = (pairs_map){0};
        int hashmap_length = hmlen(merge_hash_map_p);
        for(int i = 0; i < hashmap_length; ++i) {
            if(merge_hash_map_p[i].value > most_repated_pair.value) {
                most_repated_pair = merge_hash_map_p[i];
            }
        }

        if(most_repated_pair.value <= MINIMUM_REPET_COST) {
        	printf("\n\n[EXIT]tokens count: %lld\n", hmlen(merge_hash_map_p));
        	break;
        };

        // 3. SEPARATE MERGE LOOP
        // This loop only handles replacing the pairs in the linked list
        ++higgest_trecker_value;
        current_node = (&user->compressed)->head;
        while(current_node != NULL && current_node->next != NULL){
            uint32_t l = *(uint32_t*)(current_node->data);
            uint32_t r = *(uint32_t*)(current_node->next->data);

            if(l == most_repated_pair.key.l && r == most_repated_pair.key.r){
                *(uint32_t*)current_node->data = higgest_trecker_value;
                LLNode *next_node = current_node->next;
                current_node->next = next_node->next;
                if((&user->compressed)->tail == next_node) (&user->compressed)->tail = current_node;
                
                free(next_node->data);
                free(next_node);
                (&user->compressed)->length--;
            } else {
                current_node = current_node->next;
            }
        }

        // append the freq node to the freqs
        freq higgest_freq = {.node = higgest_trecker_value,
    				.leaf = {
    					.l = most_repated_pair.key.l,
    					.r = most_repated_pair.key.r
    				}};
        DA_append(&(user->freqs), (void*)&higgest_freq);

        int start = 0;
        if (start >= 0){
        	// if this assertion passed this means that:
        	// 	(freq index in the freqs array) = freq.node - (user->highest_id+1)
        	assert(((freq*)DA_get_element(&(user->freqs), start))->node == user->highest_id+1+start);
        	start++;
        }

        // 4. SEPARATE MAP UPDATE LOOP
        // This loop wipes the map and re-calculates everything from the new list
        hmfree(merge_hash_map_p);
        merge_hash_map_p = NULL;
        current_node = (&user->compressed)->head;
        while(current_node != NULL && current_node->next != NULL){
            pair test_pair = {
                .l = *(uint32_t*)current_node->data, 
                .r = *(uint32_t*)current_node->next->data
            };
            pairs_map* pair_p = hmgetp_null(merge_hash_map_p, test_pair);
            if(pair_p == NULL) hmput(merge_hash_map_p, test_pair, 1);
            else pair_p->value++;
            current_node = current_node->next;
        }

        printf("Iteration %.4d: Merged (%.4u, %.4u) into %.4u, tokens count: %.4lld   \r", 
                ++iter, most_repated_pair.key.l, most_repated_pair.key.r, higgest_trecker_value, hmlen(merge_hash_map_p));
    }

    printf("\n");
}


int bpe_store(bpe* ctx, const char* file_path){

	FILE *file = fopen(file_path, "wb");
    if (file == NULL) {
        return -1;
    }

    // Writing the header
    int ret;
    ret = fwrite(&ctx->header, sizeof(char), BPE_HEADER_CAP, file);
    assert(ret == BPE_HEADER_CAP);

    // Writing the ctx->compressed.length
    printf("[INFO] ctx->compressed.length = %u\n", ctx->compressed.length);
    ret = fwrite(&ctx->compressed.length, sizeof(uint32_t), 1, file);
    assert(ret == 1);

    // Writing the ctx->compressed
    LLNode* current = NULL;
    current = ctx->compressed.head;
    ret = 0;
    for(size_t _ = 0; current != NULL; _++){
		ret += fwrite(current->data, sizeof(uint32_t), 1, file);
		current = current->next;
	}
	assert(ret == (int)ctx->compressed.length);
    
    // Writing the ctx->highest_id
    printf("[INFO] ctx->highest_id = %u\n", ctx->highest_id);
    ret = fwrite(&ctx->highest_id, sizeof(uint32_t), 1, file);
    assert(ret == 1);

    // Writing the ctx->highest_id
    printf("[INFO] ctx->freqs.length = %u\n", ctx->freqs.length);
    ret = fwrite(&ctx->freqs.length, sizeof(uint32_t), 1, file);
    assert(ret == 1);

	// Writing the ctx->freqs
	ret = 0;
	for(uint32_t i = 0; i < ctx->freqs.length; i++){
		ret += fwrite(DA_get_element(&ctx->freqs, i), sizeof(freq), 1, file);
	}
	assert(ret == (int)ctx->freqs.length);

	fclose(file);

    return 0;

}

