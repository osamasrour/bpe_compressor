// TODO: making a simple script to collect an array using byte-pair-encoding algorithm
// TODO: implement better version of the algo that acts like Wikipedia describes

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
#define MINIMUM_FREQUENCY_ALLOWED (10/100)

typedef DArray Tokens;
typedef struct token{
	String_View word;
	size_t freq;
	size_t unique; // track if it should be deleted when unique = 0
} token;

typedef struct vh{
	size_t freq;
	uint32_t hash;
}info;

typedef struct bucket{
	char* key;
	info value;
}bucket;


typedef struct pair{
	uint32_t l, r;
} pair;

typedef struct pairs_map{
	pair key;
	uint32_t value;
} pairs_map;

typedef enum {
	TK_RP_EQUAL,
	TK_RP_SUB,
	TK_RP_SUPER,
	TK_RP_NOTEQUAL,
}TK_REPETITION;

FILE* f;
TK_REPETITION compare_tokens(token, token);
void free_token(token*);
void log_tokens(bucket*, FILE*);
void DA_from_SV(DArray*, String_View*);
void LLST_from_SV(LLST*, String_View*);
void LLST_from_SV2(LLST*, String_View*);
uint32_t hash_string_djb2(const char *);
uint32_t hash_string_fnv1a(const char *);

// TODO: Add PROFILING to measure how many seconds one iteration takes and the overall performance.
void bpe_v5(LLST*, bucket**);
// TODO: interduce bpe_v6 which uses integer rather than char* to optimize the algo
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
	// LLST_from_SV(&ll_txt, &sv_txt);
	LLST_from_SV2(&ll_txt, &sv_txt);
	printf("Number of elements in the ll_txt = %llu\n", ll_txt.length);
	printf("h = %u\n", (uint32_t)'h');
	// LLNode *current_tk = NULL;
	// current_tk = ll_txt.head;
	// // for(size_t _ = 0; current_tk != NULL; _++){
	// // 	fprintf(stdout, "%u,",*(uint32_t*)(current_tk->data));
	// // 	current_tk = current_tk->next;
	// // }
	bpe_v6(&ll_txt);
	exit(0);
	bucket* tokens = NULL;
	bpe_v5(&ll_txt, &tokens);
	printf("Number of elements in the ll_txt = %llu\n", ll_txt.length);
	FILE* cmp = fopen("cmp_test.txt", "a");
	// LLNode *current_tk = NULL;
	// current_tk = ll_txt.head;
	// for(size_t _ = 0; current_tk != NULL; _++){
	// 	fprintf(stdout, "%s", *(char**)(current_tk->data));
	// 	current_tk = current_tk->next;
	// }
	fclose(cmp);
	log_tokens(tokens, f);
	SV_destroy(&sv_txt);
	fclose(f);
	return 0;
}

TK_REPETITION compare_tokens(token a, token b){
	if(a.word.length == b.word.length){
		if(strcmp(a.word.buffer, b.word.buffer) == 0){ // if they equal
				return TK_RP_EQUAL;
		}else{
			return TK_RP_NOTEQUAL;
		}
	}else if(a.word.length > b.word.length && b.freq <= a.freq){
		for(size_t i = 0; i < (a.word.length - b.word.length + 1); i++){ // check this again
			if(strncmp(a.word.buffer + i, b.word.buffer, b.word.length) == 0){
				return TK_RP_SUPER;
			}
		}
		return TK_RP_NOTEQUAL;
	}else if (a.word.length < b.word.length && a.freq <= b.freq){
		for(size_t i = 0; i < (b.word.length - a.word.length + 1); i++){ // check this again
			if(strncmp(b.word.buffer + i, a.word.buffer, a.word.length) == 0){
				return TK_RP_SUB;
			}
		}
		return TK_RP_NOTEQUAL;
	}else{
		return TK_RP_NOTEQUAL;
	}
}

void free_token(token* tk){
	SV_destroy(&(tk->word));
}

void log_tokens(bucket* kv, FILE *file){
	for (size_t i = 0; i < shlenu(kv); ++i)
	{
		char* key = kv[i].key;
		fprintf(file, "%llu:word|", i);
		for(size_t ch = 0; ch < strlen(key); ch++){
			if (key[ch] == '\n') fprintf(file, "\\<n>");
			else fprintf(file, "%c", key[ch]);
		}
		fprintf(file, "|, freq = %llu\n", kv[i].value.freq);
	}
	fprintf(f, "-------------------------------------------------\n");
	fprintf(f, "-------------------------------------------------\n");
	fflush(f);
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

uint32_t hash_string_djb2(const char *str) {
    uint32_t hash = 5381;
    int c;

    while ((c = *str++)) {
        // hash * 33 + c
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

uint32_t hash_string_fnv1a(const char *str) {
    uint32_t hash = 2166136261u;
    while (*str) {
        hash ^= (uint8_t)*str++;
        hash *= 16777619u;
    }
    return hash;
}


void bpe_v5(LLST* ll_txt, bucket** tokens){

	assert(ll_txt->length > 1);
	size_t max_vocb_size = (size_t)((double)(ll_txt->length) / 156.25f);
	size_t max_freq_allowed = (size_t)((double)(ll_txt->length) / 500000.0f) >= 2 ? (size_t)((double)(ll_txt->length) / 500000.0f) : 2;
	printf("[INFO] max_vocb_size: %llu\n", max_vocb_size);
	printf("[INFO] max_freq_allowed: %llu\n", max_freq_allowed);

	while(ll_txt->length >= max_vocb_size){
		size_t old_pairs_count = (size_t)ll_txt->length;

		// Genrate the pairs_voc table
		char* biggest_freq_key = NULL;
		uint32_t biggest_freq_hash = 0;
		LLNode* current_node = ll_txt->head;

		while(current_node != NULL && current_node->next != NULL){
			char* pair_l = *(char**)(current_node->data);
			assert(pair_l != NULL);
			char* pair_r = *(char**)(current_node->next->data);
			assert(pair_r != NULL);
			size_t temp_cap = strlen(pair_l) + strlen(pair_r);
			char temp[temp_cap + 1];
			strncpy(temp, pair_l, strlen(pair_l));
			strncpy(temp + strlen(pair_l), pair_r, strlen(pair_r));
			temp[temp_cap] = '\0';
			bucket* kv = shgetp_null(*tokens, temp);
			if(kv == NULL){
				info l = {.freq = 1, .hash = hash_string_fnv1a(temp)};
				shput(*tokens, strdup(temp), l);
			}else{
				kv->value.freq++;
				if (biggest_freq_key == NULL || kv->value.freq > shgetp(*tokens, biggest_freq_key)->value.freq){
					biggest_freq_key = kv->key;
					biggest_freq_hash = kv->value.hash;
				}

			}
			current_node = current_node->next;
		}

		if(shgets(*tokens, biggest_freq_key).value.freq <= max_freq_allowed) {
			printf("\n[EXIT] the tokens length didn't change from the last iter: %llu\n", old_pairs_count);
			printf("[INFO] most freq token value: %llu\n", shgets(*tokens, biggest_freq_key).value.freq);
			break;
		}

		current_node = ll_txt->head;
		LLNode *next_node = NULL;
		while(current_node != NULL && current_node->next != NULL){
			char* pair_l = *(char**)(current_node->data);
			assert(pair_l != NULL);
			assert(current_node->next != NULL);
			char* pair_r = *(char**)(current_node->next->data);
			assert(pair_r != NULL);
			size_t temp_cap = strlen(pair_l) + strlen(pair_r);
			char temp_cmp[temp_cap + 1];
			strncpy(temp_cmp, pair_l, strlen(pair_l));
			strncpy(temp_cmp + strlen(pair_l), pair_r, strlen(pair_r));
			temp_cmp[temp_cap] = '\0';
			uint32_t temp_cmp_hash = hash_string_fnv1a(temp_cmp);

			if(temp_cmp_hash == biggest_freq_hash){
				char* new_pair_ptr = strdup(biggest_freq_key);
				free(*(char**)(current_node->data));
				*(char**)current_node->data = new_pair_ptr;
				assert(current_node->next != NULL);
				next_node = current_node->next;
				if(ll_txt->tail == next_node){
					ll_txt->tail = current_node;
				}
				current_node->next = next_node->next;
				free(*(char**)(next_node->data));
				free(next_node->data);
				free(next_node);
				ll_txt->length--;
			}else{
				current_node = current_node->next;
			}
		}


		if(old_pairs_count == (size_t)ll_txt->length){
			printf("\n[EXIT] the tokens length didn't change from the last iter: %llu\n", old_pairs_count);
			printf("[INFO] most freq token value: %llu\n", shgets(*tokens, biggest_freq_key).value.freq);
			break;
		}
		else{
			printf("\r[INFO] Update pairs number from %zu to %llu, changed by %.2lf%%, most freq value: %llu     ",
				old_pairs_count, ll_txt->length,
				((double)ll_txt->length / (double)old_pairs_count) * 100, shgets(*tokens, biggest_freq_key).value.freq);
			old_pairs_count = (size_t)ll_txt->length;

			for(size_t i = 0; i < shlenu(*tokens); i++){
				assert(shgetp_null(*tokens, (*tokens)[i].key) != NULL);
				free((*tokens)[i].key);
			}
			shfree(*tokens);
			// tokens = NULL;
		}
	}
}

void LLST_from_SV2(LLST* llst, String_View* sv){
	assert(llst->length == 0);
	llst->element_size = sizeof(uint32_t);
	for(size_t ch = 0; ch < sv->length; ch++){
		uint32_t* _char = (uint32_t*)malloc(sizeof(uint32_t));
		assert(_char != NULL);
		*_char = (uint32_t)SV_get_by_index(sv, ch);
		LLST_append(llst, _char);
	}
}

void print_map(pairs_map* map){
	for(int i = 0; i < hmlen(map); i++){
		if(map[i].key.l == 133) fprintf(stdout, "key: l = %u, r = %u\n", map[i].key.l, map[i].key.r);
	}
}

// TODO: do more test and inhance the code
void bpe_v6(LLST* ll_txt){

	static uint32_t iter =0;
	uint32_t higgest_trecker_value = 0;
	

	LLNode *current_node = NULL;
	LLNode *next_node = NULL;
	LLNode *prev_node = NULL;
	current_node = ll_txt->head;
	for(size_t _ = 0; current_node != NULL; _++){
		if(higgest_trecker_value < *(uint32_t*)current_node->data){
			higgest_trecker_value = *(uint32_t*)current_node->data;
		}
		current_node = current_node->next;
	}

	printf("higgest_trecker_value = %u\n", higgest_trecker_value);
	pairs_map* merge_hash_map_p = NULL;
	pairs_map most_repated_pair = {0};

	current_node = ll_txt->head;
	while(current_node != NULL && current_node->next != NULL){
		uint32_t pair_l = *(uint32_t*)(current_node->data);
		uint32_t pair_r = *(uint32_t*)(current_node->next->data);
		pair test_pair = {.l = pair_l, .r = pair_r};

		pairs_map* pair_p = hmgetp_null(merge_hash_map_p, test_pair);
		if(pair_p == NULL){
			hmput(merge_hash_map_p, test_pair, 1);
		}else{
			pair_p->value++;
			if(most_repated_pair.value < pair_p->value){
				most_repated_pair = (pairs_map){
					.key = {
						.l = pair_p->key.l,
						.r = pair_p->key.r
					},
					.value = pair_p->value
				};
			}
		}
		current_node = current_node->next;
	}

	while(1){

		if(most_repated_pair.value <= 2){
			printf("\n[EXIT]: %llu\n", ll_txt->length);
			break;
		}

		current_node = ll_txt->head;
		next_node = current_node->next;
		prev_node = NULL;
		
		++higgest_trecker_value;
		while(current_node != NULL && current_node->next != NULL){
			uint32_t pair_l = *(uint32_t*)(current_node->data);
			uint32_t pair_r = *(uint32_t*)(current_node->next->data);

			if(pair_l == most_repated_pair.key.l && pair_r == most_repated_pair.key.r){

				*(uint32_t*)current_node->data = higgest_trecker_value; // palce the replacing value in the current node
				assert(higgest_trecker_value < UINT32_MAX - 100);
				next_node = current_node->next;
				if(ll_txt->tail == next_node){
					ll_txt->tail = current_node;
				}
				current_node->next = next_node->next;
				free(next_node->data);
				free(next_node);
				ll_txt->length--;
				// TODO: check the adding logic, make sure you add every new pair => "the bug is here"
				if(prev_node != NULL){
					uint32_t before_pair_l = *(uint32_t*)(prev_node->data);
					pair test_pair = {.l = before_pair_l, .r = higgest_trecker_value};
					pairs_map* pair_p = hmgetp_null(merge_hash_map_p, test_pair);
					if(pair_p == NULL){
						hmput(merge_hash_map_p, test_pair, 1);
						assert(test_pair.l != 133 || test_pair.r != 158); // should triger
					}else{
						pair_p->value++;
					}
				}
				if(current_node->next != NULL){
					uint32_t after_pair_r = *(uint32_t*)(current_node->next->data);
					pair test_pair = {.l = higgest_trecker_value, .r = after_pair_r};
					pairs_map* pair_p = hmgetp_null(merge_hash_map_p, test_pair);
					if(pair_p == NULL){
						hmput(merge_hash_map_p, test_pair, 1);
						assert(test_pair.l != 133 || test_pair.r != 158); // should triger
						
					}else{
						pair_p->value++;
					}
				}

			}else{
				if (current_node != ll_txt->head)
				{
					prev_node = current_node;
				}
				current_node = current_node->next;
			}

		}

		printf("Iteration %4d: Most frequent pair (ID %4u, ID %4u) occurred %4d times. Merging into ID %4u      \r",
			++iter, most_repated_pair.key.l, most_repated_pair.key.r, most_repated_pair.value, higgest_trecker_value);

		most_repated_pair = (pairs_map){0};
		current_node = ll_txt->head;
		while(current_node != NULL && current_node->next != NULL){
			pair test_pair = (pair){
				.l = *(uint32_t*)current_node->data, 
				.r = *(uint32_t*)current_node->next->data
			};
			pairs_map* pair_p = hmgetp_null(merge_hash_map_p, test_pair);
			// this shouldn't happend.
			if(pair_p == NULL){
				printf("\nl = %u, r =%u, max = %u\n", test_pair.l, test_pair.r, higgest_trecker_value);
				print_map(merge_hash_map_p);
				exit(1);
			}
			if(most_repated_pair.value < pair_p->value){
				most_repated_pair = (pairs_map){
					.key = {
						.l = pair_p->key.l,
						.r = pair_p->key.r
					},
					.value = pair_p->value
				};
			}
			current_node = current_node->next;
		}
		
	}
}