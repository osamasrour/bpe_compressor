#ifndef STRING_VIEW_H_
#define STRING_VIEW_H_
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>

#define SV_fmt "%s"
#define SV_arg(sv) sv.buffer
#define SVDEF static inline

typedef struct String_View
{
    char* buffer;
    size_t length;
} String_View;


SVDEF String_View SV_create(char String_litral[]);
SVDEF size_t SV_len(String_View * sv);
SVDEF void SV_lower(String_View * sv);
SVDEF void SV_upper(String_View * sv);
SVDEF void SV_setchar(String_View * sv, uint32_t index, char c);
SVDEF double SV_find(String_View * sv, const char sub_string[]);
SVDEF int SV_setstr(String_View *sv, uint8_t index, char sub_string[]);
SVDEF String_View SV_merge_cstr(String_View *sv, const char* string);
SVDEF void SV_merge_parts(String_View *sv, const char* chunk, size_t len);
SVDEF void SV_to_cstr(String_View* sv, char cstr[]);
// TODO: add this func to the main file
SVDEF String_View SV_concat(String_View* sv1, String_View* sv2);
SVDEF char SV_get_by_index(String_View *sv, double index);
SVDEF bool SV_isdigit(String_View *sv);
SVDEF String_View SV_slice(String_View *sv, size_t from, size_t to);
SVDEF String_View SV_trim_left(String_View *sv);
SVDEF String_View SV_trim_right(String_View *sv);
SVDEF String_View SV_trim(String_View *sv);
SVDEF bool SV_starts_with(String_View *sv, const char* prefix);
SVDEF bool SV_ends_with(String_View *sv, const char* suffix);
SVDEF void SV_clear(String_View *sv);
SVDEF void SV_destroy(String_View * sv);
SVDEF void SVEx_zfill(char buffer[], size_t buffer_size, int num);
SVDEF size_t SVEx_read_file(String_View *sv, FILE* f);

#endif // STRING_VIEW_H_

#if defined(SV_IMPLEMENTATION)

SVDEF String_View SV_create(char String_litral[]){
    size_t i = strlen(String_litral);
    String_View sv = {
        .buffer = NULL,
        .length = 0
    };
    sv.buffer = (char *)malloc((i + 1) * sizeof(char));
    if(sv.buffer == NULL){
        fprintf(stderr, "ERROR: couldn't allocating memory for SV buffer\n");
        exit(1);
    }
    strcpy(sv.buffer, String_litral);
    // (sv.buffer[i + 1]) = '\0'; // Wrong
    sv.length = i; // not containing the null char '\0' in the lenght
    return sv;
}

SVDEF size_t SV_len(String_View * sv){
    return sv->length;
}

SVDEF void SV_lower(String_View * sv){
    for(size_t i= 0; i < sv->length; i++){
        if ((90 >= (*sv).buffer[i]) && ((*sv).buffer[i] >= 65)){
            ((*sv).buffer[i]) = ((*sv).buffer[i] + 32);
        }
    }
}

SVDEF void SV_upper(String_View * sv){
    for(size_t i= 0; i < sv->length; i++){
        if ((122 >= (*sv).buffer[i]) && ((*sv).buffer[i] >= 97)){
            ((*sv).buffer[i]) = ((*sv).buffer[i] - 32);
        }
    }
}



SVDEF void SV_setchar(String_View * sv, uint32_t index, char c){
    if (sv->length < index){
        fprintf(stderr, "ERROR: index out of range\n");
        exit(1);
    }
    sv->buffer[index] = c;
}

// TODO: Fix that in the main file
double SV_find(String_View * sv, const char sub_string[]){
    /*
    return the index of the first mach between
    the substing and the string_view if excist,
    if not found return -1
    */
    size_t j = 0, i = 0;
    size_t sub_stringl = strlen(sub_string);
    if (sub_stringl > sv->length) return -1;
    if (strcmp(sub_string, sv->buffer) == 0) return i;
    for(i= 0; i <= sv->length - sub_stringl; i++){
        j = 0;
        for(; j <= sub_stringl; j++){
            if(sub_string[j] != sv->buffer[i + j]) break;
        }
        if(j == sub_stringl) return i;
    }
    return -1;
}

int SV_setstr(String_View *sv, uint8_t index, char sub_string[]){
    size_t sub_stringl = strlen(sub_string);
    if (sv->length < (sub_stringl + index + 1)){
        fprintf(stderr, "ERROR: (index + string length) out of range\n");
        exit(1);
    }
    for(uint8_t i = 0;i < sub_stringl; index++){
        sv->buffer[index] = sub_string[i];
        i++;
    }
    return 0;
}

String_View SV_merge_cstr(String_View *sv, const char* string){
    size_t string_len = strlen(string);
    sv->buffer = (char*) realloc(sv->buffer, string_len + sv->length +1);
    if(sv->buffer == NULL){
        fprintf(stderr, "ERROR: couldn't reallocating memory for SV buffer\n");
        exit(1);
    }
    strcpy(sv->buffer + sv->length, string);
    sv->length += string_len;
    return *sv;
}

SVDEF void SV_merge_parts(String_View *sv, const char* chunk, size_t len){
    sv->buffer = (char*) realloc(sv->buffer, len + sv->length +1);
    if(sv->buffer == NULL){
        fprintf(stderr, "ERROR: couldn't reallocating memory for SV buffer\n");
        exit(1);
    }
    memcpy(sv->buffer + sv->length, chunk, len);
    sv->length += len;
    sv->buffer[sv->length] = '\0';
    // return *sv;
}


SVDEF void SV_to_cstr(String_View* sv, char cstr[]){
    strcpy(cstr, sv->buffer);
    // cstr[sv->length + 1] = '\0'; // Wrong
}

// Concatenate two SVs to a new one without freeing/replacing any of the previous SVs
SVDEF String_View SV_concat(String_View* sv1, String_View* sv2){
    char buff_1[sv1->length];
    strcpy(buff_1, sv1->buffer);
    char buff_2[sv2->length];
    strcpy(buff_2, sv2->buffer);
    char buff[sv1->length + sv2->length];
    int ret = snprintf(buff, sv1->length + sv2->length + 1, "%s%s", buff_1, buff_2);
    if(ret < 0 || ret > (int)(sv1->length + sv2->length)){
        fprintf(stderr, "somteing wrong happend with snprintf\n");
        exit(1);
    }
    buff[sv1->length + sv2->length] = '\0';
    String_View new_sv = SV_create(buff);
    return new_sv;
}

char SV_get_by_index(String_View *sv, double index){
    if (index >= sv->length || ((index < 0) && (0 - index) > sv->length)){
        fprintf(stderr, "ERROR: index %.0lf out of range\n", index);
        exit(1);
    }
    size_t i;
    if(index < 0){
        i = sv->length + index;
    }
    else{
        i = index;
    }
    return *(sv->buffer + i);
}

SVDEF bool SV_isdigit(String_View *sv){
    for(size_t i = 0; i < sv->length; i++){
        if (*(sv->buffer + i) < 48 || *(sv->buffer + i) > 57)
        {
            return false;
        }
        
    }
    return true;
}

SVDEF String_View SV_slice(String_View *sv, size_t from, size_t to){
    if (to > sv->length || from >= to){
        fprintf(stderr, "ERROR: SV_slice: unvalid parametars from: %llu, to: %llu, and len: %llu\nFollow the rule (0 < from < to < SV_len(&sv))", from, to, sv->length);
        exit(1);
    }
    String_View new_sv = SV_create("");
    char cstr[to - from + 1];
    for(size_t i = 0; (i + from) >= from && (i + from) < to; i++){
        cstr[i] = *(sv->buffer + (from + i));
    }
    cstr[to - from] = '\0';
    SV_merge_cstr(&new_sv, cstr);
    return new_sv;
}

SVDEF String_View SV_trim_left(String_View *sv){
    size_t i = 0;
    while(i < sv->length && isspace(SV_get_by_index(sv, (double)i))){
        i++;
    }
    String_View nsv = SV_slice(sv, i, sv->length);
    SV_destroy(sv);
    return nsv;
}

String_View SV_trim_right(String_View *sv){
    int i = SV_len(sv) - 1;
    while(i >= 0 && isspace(SV_get_by_index(sv, (double)i))){
        i--;
    }
    String_View nsv = SV_slice(sv, 0, i + 1);
    SV_destroy(sv);
    return nsv;
}

SVDEF String_View SV_trim(String_View *sv){
    bool chfrom;
    bool chto;
    int from = 0;
    int to = SV_len(sv) - 1;
    do{
		chto = false;
        chfrom = false;
		if(isspace(SV_get_by_index(sv, (double)from))){
            from++;
            chfrom = true;
        }
        
        if(isspace(SV_get_by_index(sv, (double)to))){
            to--;
            chto = true;
        }
    }while(chfrom || chto);

    String_View nsv = SV_slice(sv, from, to + 1);
    SV_destroy(sv);
    return nsv;
}

SVDEF bool SV_starts_with(String_View *sv, const char* prefix){
    size_t len = strlen(prefix);
    char copy[len + 1];
    strncpy(copy, sv->buffer, len);
    copy[len] = '\0';
    return (strncmp(copy, prefix, len) == 0);
}

SVDEF bool SV_ends_with(String_View *sv, const char* suffix){
    size_t len = strlen(suffix);
    char copy[len + 1];
    strncpy(copy, sv->buffer + (SV_len(sv) - len), len);
    copy[len] = '\0';
    return (strncmp(copy, suffix, len) == 0);
}

SVDEF void SV_clear(String_View *sv){
    sv->buffer = (char *)realloc(sv->buffer, 1 * sizeof(char));
    if(sv->buffer == NULL){
        fprintf(stderr, "ERROR: couldn't reallocating memory for SV buffer\n");
        exit(1);
    }
    (*sv).buffer[0] = '\0';
    sv->length = 0;
}

SVDEF void SV_destroy(String_View * sv){
    free(sv->buffer);
    sv = NULL;
}

// the buffer must be Null-Terminated
SVDEF void SVEx_zfill(char buffer[], size_t zfill_num, int num){
    size_t numLength = 0;
    int n = num;
    while(n > 0){
        int d = n % 10;
        n = (n - d) / 10;
        numLength++;
    }
    if(zfill_num < numLength) {
        fprintf(stderr, "ERROR: SVEx_zfill => The number is bigger than the buff limit.\n");
        exit(1);
    }
    n = num;
    for(int i = zfill_num - 1; i >= 0; i--){
        int d = n % 10;
        n = (n - d) / 10;
        char ch = '0' + d;
        buffer[i] = ch;
    }
    buffer[zfill_num] = '\0';
}

SVDEF size_t SVEx_read_file(String_View *sv, FILE* f){
    size_t chars_num = 0;
    char c;
    fseek(f, 0, SEEK_END);
    long int len = ftell(f);
    fseek(f, 0, SEEK_SET);
    sv->buffer = (char*)realloc(sv->buffer, (len + 1) * sizeof(char));
    sv->length += len;
    memset(sv->buffer, 0, (len + 1) * sizeof(char));
    while((c = fgetc(f)) != EOF){
        if(isprint(c) || c == '\n'){
            sv->buffer[chars_num] = c;
            chars_num++;
        }
    }
    sv->buffer[sv->length] = '\0';
    return chars_num;
}

#endif // SV_IMPLEMENTATION