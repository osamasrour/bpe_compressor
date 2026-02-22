#ifndef UTIL_H_
#define UTIL_H_
#include "da.h"
#include "sv.h"
#include "llst.h"
#include <assert.h>

void DA_from_SV(DArray*, String_View*);
void SV_from_DA(String_View*, DArray*);
void LLST_from_SV(LLST*, String_View*);
void LLSTu32_from_SV2(LLST*, String_View*);
void llst_log(LLST);

#endif // UTIL_H_


#ifdef UTIL_IMPLEMENTATION

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

void SV_from_DA(String_View* sv, DArray* da){
	char *chunk = malloc(da->length * sizeof(char));
	memset(chunk, 0, da->length);
	for(uint32_t i = 0; i < da->length; i++){
		chunk[i] = *(uint8_t*)DA_get_element(da, i);
	}
	SV_merge_parts(sv,chunk, da->length);
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
		memset(_char, 0, sizeof(uint32_t));
		assert(_char != NULL);
		*_char = (uint32_t)(unsigned char)SV_get_by_index(sv, ch);
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


#endif // UTIL_IMPLEMENTATION