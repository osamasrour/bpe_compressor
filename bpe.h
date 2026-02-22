#ifndef BPE_H_
#define BPE_H_

#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include "stb_ds.h"


#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error "[ERROR]: this only work on a little-indian maichine\n"
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

void bpe_init(bpe*);
// TODO(#2): make it compress algo - encode and decode functions - to compress and decompress the files
// to emprove storage methods, dump the pairs in the hardware with the compressed file so you can
// decompress it back
void bpe_encode(bpe*);
int bpe_pack(bpe*, const char*);
int bpe_unpack(bpe*, const char*);
void bpe_decode(bpe*, String_View*);


#endif // BPE_H_


#ifdef BPE_IMPLEMENTATION

void bpe_init(bpe* ctx){
	ctx->compressed = LLST_create(sizeof(uint32_t));
	ctx->freqs = DA_create_array(sizeof(freq), 256, 256);
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


int bpe_pack(bpe* ctx, const char* file_path){

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

    // Writing the ctx->freqs.length
    printf("[INFO] ctx->freqs.length = %u\n", ctx->freqs.length);
    ret = fwrite(&ctx->freqs.length, sizeof(uint32_t), 1, file);
    assert(ret == 1);

	// Writing the ctx->freqs
	ret = 0;
	for(uint32_t i = 0; i < ctx->freqs.length; i++){
		// log freqs
		freq* freq_ptr = DA_get_element(&ctx->freqs, i);
		printf("   /|%u\n", ((freq*)freq_ptr)->leaf.l);
		printf("%u\n", ((freq*)freq_ptr)->node);
		printf("   \\|%u\n\n", ((freq*)freq_ptr)->leaf.r);

		ret += fwrite(&freq_ptr->node, sizeof(uint32_t), 1, file);
		fwrite(&freq_ptr->leaf.l, sizeof(uint32_t), 1, file);
		fwrite(&freq_ptr->leaf.r, sizeof(uint32_t), 1, file);
	}
	assert(ret == (int)ctx->freqs.length);

	fclose(file);

    return 0;

}

int bpe_unpack(bpe* ctx, const char* file_path){
	FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
        return -1;
    }

    int ret;
    ret = fread(&(ctx->header), sizeof(char), BPE_HEADER_CAP, file);
    assert(ret == BPE_HEADER_CAP);
    // TODO: handle the version here


    uint32_t compressed_length = 0;
    uint32_t* compressed_length_ptr = &compressed_length;
    ret = fread(compressed_length_ptr, sizeof(uint32_t), 1, file);
    assert(ret > 0);

    // Read ctx->compressed
    uint32_t data;
    for(uint32_t _ = 0; _ < compressed_length; _++){
    	ret = fread(&data, sizeof(uint32_t), 1, file);
    	assert(ret > 0);
    	// TODO: make a progress bar here
    	LLST_append(&(ctx->compressed), &data);
    }
    assert(compressed_length == ctx->compressed.length);

	// Read bpe.higthest_id
	ret = fread(&(ctx->highest_id), sizeof(uint32_t), 1, file);
	assert(ret > 0);

	// Read ctx->freqs.length
	uint32_t freqs_length;
	ret = fread(&freqs_length, sizeof(uint32_t), 1, file);
	assert(ret > 0);

	// Read ctx->freqs
	freq a;
	for(uint32_t i = 0; i < freqs_length; i++){
		ret = fread(&a, sizeof(freq), 1, file);
		DA_append(&(ctx->freqs), (void*)&a);
	}
	assert(freqs_length == ctx->freqs.length);

	// make sure we reached the end of the file
	assert(fread(&ret, sizeof(char), 1, file) == 0);

    fclose(file);
    return 0;
}

void traverse(DArray* arr, uint32_t val, bpe* ctx){
	if (val <= ctx->highest_id){
		assert(val == (uint8_t)val);
		DA_append(arr, (void*)&val);
		return;
	}else{

		uint32_t index = val - (ctx->highest_id + 1);
		// if (index >= ctx->freqs.length) {printf("the value = %u\n", val); exit(69);}
		freq f = *(freq*)DA_get_element(&ctx->freqs, index);
		assert(DA_get_element(&ctx->freqs, val - (ctx->highest_id + 1)) != NULL);
		traverse(arr, f.leaf.l, ctx);
		traverse(arr, f.leaf.r, ctx);

	}
}

void bpe_decode(bpe* ctx, String_View* out){
	assert(ctx->compressed.length > 0);
	LLNode *current = ctx->compressed.head;
	DArray decompressed = DA_create_array(sizeof(uint8_t), 256, 256);
	size_t current_idx = 0;
	while(current != NULL){
		if (*(uint32_t*)(current->data) <= ctx->highest_id){
			uint8_t* ch = (uint8_t*)current->data;
			assert(*ch == *(uint32_t*)current->data);
			DA_append(&decompressed, ch);
		}else{
			traverse(&decompressed, *(uint32_t*)current->data, ctx);
		}
		current_idx++;
		current = current->next;
	}
	SV_from_DA(out, &decompressed);
	DA_destroy(&decompressed);
}

#endif // BPE_IMPLEMENTATION