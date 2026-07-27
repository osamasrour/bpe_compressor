#ifndef STRING_BUILDER_H_
#define STRING_BUILDER_H_
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>


#define SB_fmt "%s"
#define SB_arg(sb) sb.buffer
#define SBDEF static inline

typedef struct String_Builder
{
    char* buffer;
    size_t length;
} String_Builder;


SBDEF String_Builder SB_create(char String_litral[]);
SBDEF size_t SB_len(String_Builder * sb);
SBDEF void SB_lower(String_Builder * sb);
SBDEF void SB_upper(String_Builder * sb);
SBDEF void SB_setchar(String_Builder * sb, uint8_t index, char c);
SBDEF size_t SB_find(String_Builder * sb, const char sub_string[]);
SBDEF int SB_setstr(String_Builder *sb, uint8_t index, char sub_string[]);
SBDEF String_Builder SB_merge_cstr(String_Builder *sb, const char* string);
SBDEF void SB_merge_parts(String_Builder *sb, const char* chunk, size_t len);
SBDEF void SB_to_cstr(String_Builder* sb, char cstr[]);
SBDEF String_Builder SB_concat(String_Builder* sb1, String_Builder* sb2);
SBDEF char SB_get_by_index(String_Builder *sb, double index);
SBDEF bool SB_isdigit(String_Builder *sb);
SBDEF String_Builder SB_slice(String_Builder *sb, size_t from, size_t to);
SBDEF String_Builder SB_trim_left(String_Builder *sb);
SBDEF String_Builder SB_trim_right(String_Builder *sb);
SBDEF String_Builder SB_trim(String_Builder *sb);
SBDEF bool SB_starts_with(String_Builder *sb, const char* prefix);
SBDEF bool SB_ends_with(String_Builder *sb, const char* suffix);
SBDEF void SB_clear(String_Builder *sb);
SBDEF void SB_destroy(String_Builder * sb);
SBDEF void SBEx_zfill(char buffer[], size_t buffer_size, int num);
SBDEF size_t SBEx_read_file(String_Builder *sb, FILE* f);
SBDEF size_t SBEx_write_file(String_Builder *sb, FILE* f);

#endif // STRING_BUILDER_H_

#if defined(SB_IMPLEMENTATION)

SBDEF String_Builder SB_create(char String_litral[]){
    size_t i = strlen(String_litral);
    String_Builder sb = {
        .buffer = NULL,
        .length = 0
    };
    sb.buffer = (char *)malloc((i + 1) * sizeof(char));
    if(sb.buffer == NULL){
        fprintf(stderr, "ERROR: couldn't allocating memory for Sb buffer\n");
        exit(1);
    }
    strcpy(sb.buffer, String_litral);
    sb.length = i; // not containing the null char '\0' in the lenght
    return sb;
}

SBDEF size_t SB_len(String_Builder * sb){
    return sb->length;
}

SBDEF void SB_lower(String_Builder * sb){
    for(size_t i= 0; i < sb->length; i++){
        if ((90 >= (*sb).buffer[i]) && ((*sb).buffer[i] >= 65)){
            ((*sb).buffer[i]) = ((*sb).buffer[i] + 32);
        }
    }
}

SBDEF void SB_upper(String_Builder * sb){
    for(size_t i= 0; i < sb->length; i++){
        if ((122 >= (*sb).buffer[i]) && ((*sb).buffer[i] >= 97)){
            ((*sb).buffer[i]) = ((*sb).buffer[i] - 32);
        }
    }
}



SBDEF void SB_setchar(String_Builder * sb, uint8_t index, char c){
    if (sb->length < index){
        fprintf(stderr, "ERROR: index out of range\n");
        exit(1);
    }
    sb->buffer[index] = c;
}

size_t SB_find(String_Builder * sb, const char sub_string[]){
    /*
    return the index of the first mach between
    the substing and the string_Builder if excist,
    if not found return -1
    */
    size_t j = 0, i = 0;
    size_t sub_stringl = strlen(sub_string);
    if (sub_stringl > sb->length) return -1;
    if (strcmp(sub_string, sb->buffer) == 0) return i;
    for(i= 0; i <= sb->length - sub_stringl; i++){
        j = 0;
        for(; j <= sub_stringl; j++){
            if(sub_string[j] != sb->buffer[i + j]) break;
        }
        if(j == sub_stringl) return i;
    }
    return -1;
}

int SB_setstr(String_Builder *sb, uint8_t index, char sub_string[]){
    size_t sub_stringl = strlen(sub_string);
    if (sb->length < (sub_stringl + index + 1)){
        fprintf(stderr, "ERROR: (index + string length) out of range\n");
        exit(1);
    }
    for(uint8_t i = 0;i < sub_stringl; index++){
        sb->buffer[index] = sub_string[i];
        i++;
    }
    return 0;
}

String_Builder SB_merge_cstr(String_Builder *sb, const char* string){
    size_t string_len = strlen(string);
    sb->buffer = (char*) realloc(sb->buffer, string_len + sb->length +1);
    if(sb->buffer == NULL){
        fprintf(stderr, "ERROR: couldn't reallocating memory for Sb buffer\n");
        exit(1);
    }
    strcpy(sb->buffer + sb->length, string);
    sb->length += string_len;
    return *sb;
}

SBDEF void SB_merge_parts(String_Builder *sb, const char* chunk, size_t len){
    sb->buffer = (char*) realloc(sb->buffer, len + sb->length +1);
    if(sb->buffer == NULL){
        fprintf(stderr, "ERROR: couldn't reallocating memory for SB buffer\n");
        exit(1);
    }
    memcpy(sb->buffer + sb->length, chunk, len);
    sb->length += len;
    sb->buffer[sb->length] = '\0';
    // return *sb;
}

SBDEF void SB_to_cstr(String_Builder* sb, char cstr[]){
    strcpy(cstr, sb->buffer);
    cstr[sb->length + 1] = '\0';
}

// Concatenate two SBs to a new one without freeing/replacing any of the previous SBs
SBDEF String_Builder SB_concat(String_Builder* sb1, String_Builder* sb2){
    char buff_1[sb1->length];
    strcpy(buff_1, sb1->buffer);
    char buff_2[sb2->length];
    strcpy(buff_2, sb2->buffer);
    char buff[sb1->length + sb2->length];
    int ret = snprintf(buff, sb1->length + sb2->length + 1, "%s%s", buff_1, buff_2);
    if(ret < 0 || ret > (int)(sb1->length + sb2->length)){
        fprintf(stderr, "somteing wrong happend with snprintf\n");
        exit(1);
    }
    buff[sb1->length + sb2->length] = '\0';
    String_Builder new_sb = SB_create(buff);
    return new_sb;
}

char SB_get_by_index(String_Builder *sb, double index){
    if (index >= sb->length || ((index < 0) && (0 - index) > sb->length)){
        fprintf(stderr, "ERROR: index %.0lf out of range\n", index);
        exit(1);
    }
    size_t i;
    if(index < 0){
        i = sb->length + index;
    }
    else{
        i = index;
    }
    return *(sb->buffer + i);
}

SBDEF bool SB_isdigit(String_Builder *sb){
    for(size_t i = 0; i < sb->length; i++){
        if (*(sb->buffer + i) < 48 || *(sb->buffer + i) > 57)
        {
            return false;
        }
        
    }
    return true;
}

SBDEF String_Builder SB_slice(String_Builder *sb, size_t from, size_t to){
    if (to > sb->length || from >= to){
        fprintf(stderr, "ERROR: unvalid parametars\nFollow the rule (0 < from < to < SB_len(&sb))");
        exit(1);
    }
    String_Builder new_sb = SB_create("");
    char cstr[to - from + 1];
    for(size_t i = 0; (i + from) >= from && (i + from) < to; i++){
        cstr[i] = *(sb->buffer + (from + i));
    }
    cstr[to - from] = '\0';
    SB_merge_cstr(&new_sb, cstr);
    return new_sb;
}

SBDEF String_Builder SB_trim_left(String_Builder *sb){
    size_t i = 0;
    while(i < sb->length && isspace(SB_get_by_index(sb, (double)i))){
        i++;
    }
    String_Builder nsb = SB_slice(sb, i, sb->length);
    SB_destroy(sb);
    return nsb;
}

String_Builder SB_trim_right(String_Builder *sb){
    int i = SB_len(sb) - 1;
    while(i >= 0 && isspace(SB_get_by_index(sb, (double)i))){
        i--;
    }
    String_Builder nsb = SB_slice(sb, 0, i + 1);
    SB_destroy(sb);
    return nsb;
}

SBDEF String_Builder SB_trim(String_Builder *sb){
    bool chfrom;
    bool chto;
    int from = 0;
    int to = SB_len(sb) - 1;
    do{
		chto = false;
        chfrom = false;
		if(isspace(SB_get_by_index(sb, (double)from))){
            from++;
            chfrom = true;
        }
        
        if(isspace(SB_get_by_index(sb, (double)to))){
            to--;
            chto = true;
        }

    }while(chfrom || chto);

    String_Builder nsb = SB_slice(sb, from, to + 1);
    SB_destroy(sb);
    return nsb;
}

SBDEF bool SB_starts_with(String_Builder *sb, const char* prefix){
    size_t len = strlen(prefix);
    char copy[len + 1];
    strncpy(copy, sb->buffer, len);
    copy[len] = '\0';
    return (strncmp(copy, prefix, len) == 0);
}

SBDEF bool SB_ends_with(String_Builder *sb, const char* suffix){
    size_t len = strlen(suffix);
    char copy[len + 1];
    strncpy(copy, sb->buffer + (SB_len(sb) - len), len);
    copy[len] = '\0';
    return (strncmp(copy, suffix, len) == 0);
}

SBDEF void SB_clear(String_Builder *sb){
    sb->buffer = (char *)realloc(sb->buffer, 1 * sizeof(char));
    if(sb->buffer == NULL){
        fprintf(stderr, "ERROR: couldn't reallocating memory for Sb buffer\n");
        exit(1);
    }
    (*sb).buffer[0] = '\0';
    sb->length = 0;
}

SBDEF void SB_destroy(String_Builder * sb){
    free(sb->buffer);
    sb = NULL;
}

// the buffer must be Null-Terminated
SBDEF void SBEx_zfill(char buffer[], size_t zfill_num, int num){
    size_t numLength = 0;
    int n = num;
    while(n > 0){
        int d = n % 10;
        n = (n - d) / 10;
        numLength++;
    }
    if(zfill_num < numLength) {
        fprintf(stderr, "ERROR: SbEx_zfill => The number is bigger than the buff limit.\n");
        exit(1);
    }
    /*
    int digit = 5;
    char char_digit = '0' + digit;
    // char_digit will now hold '5'
    */
    n = num;
    for(int i = zfill_num - 1; i >= 0; i--){
        int d = n % 10;
        n = (n - d) / 10;
        char ch = '0' + d;
        // printf("%c", ch);
        buffer[i] = ch;
    }
    buffer[zfill_num] = '\0';
}

SBDEF size_t SBEx_read_file(String_Builder *sb, FILE* f){
    // TODO: Determine the FILE* is Readable
    size_t chars_num = 0;
    char c;
    fseek(f, 0, SEEK_END);
    long int len = ftell(f);
    fseek(f, 0, SEEK_SET);
    sb->buffer = (char*)realloc(sb->buffer, (len + 1) * sizeof(char));
    sb->length = len; // NOTE: should be `=` not `+=`
    memset(sb->buffer, 0, (len + 1) * sizeof(char));
    while((c = fgetc(f)) != EOF){
        if(isprint(c) || c == '\n'){
            sb->buffer[chars_num] = c;
            chars_num++;
        }
    }
    sb->buffer[sb->length] = '\0';
    return chars_num;
}

SBDEF size_t SBEx_write_file(String_Builder *sb, FILE* f){
    // TODO: Determine the FILE* is writable
    int chars_num = fwrite(sb->buffer, sizeof(char), sb->length, f);
    return chars_num;
}

#endif // SB_IMPLEMENTATION
