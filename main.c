
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


FILE* f;
void DA_from_SV(DArray*, String_View*);
void LLST_from_SV(LLST*, String_View*);
void LLSTu32_from_SV2(LLST*, String_View*);
// TODO(#2): make it compress algo - encode and decode functions - to compress and decompress the files
// to emprove storage methods, dump the pairs in the hardware with the compressed file so you can
// decompress it back
void bpe_v6(LLST*);


int main(int argc, char const *argv[]){
	if(argc < 3){
		fprintf(stderr, "[INFO]: USAGE: <%s> <input.txt> <output.txt>\n", argv[0]);
		exit(1);
	}
	const char* input_path =  argv[1];
	const char* output_path = argv[2];
	printf("[INFO] Excuting BPE on %s\n", input_path);
	f = fopen(output_path, "w");
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
	printf("Number of elements in the ll_txt = %llu\n", ll_txt.length);
#define LOG 0
#if LOG
	LLNode *current_tk = NULL;
	current_tk = ll_txt.head;
	for(size_t _ = 0; current_tk != NULL; _++){
		fprintf(stdout, "%u,",*(uint32_t*)(current_tk->data));
		current_tk = current_tk->next;
	}
#endif
	bpe_v6(&ll_txt);
#if LOG
	current_tk = NULL;
	current_tk = ll_txt.head;
	for(size_t _ = 0; current_tk != NULL; _++){
		fprintf(stdout, "%u,",*(uint32_t*)(current_tk->data));
		current_tk = current_tk->next;
	}
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


// the algo complexity: f(x) = 0.00226*(x*x), which x: size of the file in KB, and f(x): number of seconds
void bpe_v6(LLST* ll_txt){
    static uint32_t iter = 0;
    uint32_t higgest_trecker_value = 0;
    // TODO: we need to dump this array into a file with the compressed text.
    DArray freqs = DA_create_array(sizeof(freq), 256, 256);

    // 1. Initial Scan for Highest ID
    LLNode *current_node = ll_txt->head;
    while(current_node != NULL){
        if(higgest_trecker_value < *(uint32_t*)current_node->data){
            higgest_trecker_value = *(uint32_t*)current_node->data;
        }
        current_node = current_node->next;
    }

    uint32_t highest_sample_id = higgest_trecker_value; // The value of the highest sample we reached in the article.
    printf("\nThe higgest ID: %u\n\n", highest_sample_id);

    pairs_map* merge_hash_map_p = NULL;
    pairs_map most_repated_pair = {0};

    // 2. Initial Map Generation
    current_node = ll_txt->head;
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

        if(most_repated_pair.value <= 2) { // TODO: replace '2' with calculated cost value
        	printf("\n\n[EXIT]tokens count: %lld\n", hmlen(merge_hash_map_p));
        	break;
        };

        // 3. SEPARATE MERGE LOOP
        // This loop only handles replacing the pairs in the linked list
        ++higgest_trecker_value;
        current_node = ll_txt->head;
        while(current_node != NULL && current_node->next != NULL){
            uint32_t l = *(uint32_t*)(current_node->data);
            uint32_t r = *(uint32_t*)(current_node->next->data);

            if(l == most_repated_pair.key.l && r == most_repated_pair.key.r){
                *(uint32_t*)current_node->data = higgest_trecker_value;
                LLNode *next_node = current_node->next;
                current_node->next = next_node->next;
                if(ll_txt->tail == next_node) ll_txt->tail = current_node;
                
                free(next_node->data);
                free(next_node);
                ll_txt->length--;
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
        DA_append(&freqs, (void*)&higgest_freq);

        int start = 0;
        if (start >= 0){
        	// if this assertion passed this means that:
        	// 	(freq index in the freqs array) = freq.node - (highest_sample_id+1)
        	assert(((freq*)DA_get_element(&freqs, start))->node == highest_sample_id+1+start);
        	start++;
        }

        // 4. SEPARATE MAP UPDATE LOOP
        // This loop wipes the map and re-calculates everything from the new list
        hmfree(merge_hash_map_p);
        merge_hash_map_p = NULL;
        current_node = ll_txt->head;
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
