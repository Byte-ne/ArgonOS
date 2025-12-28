#ifndef STRING_H
#define STRING_H

#include "types.h"

uint32_t strlen(const char* str);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, uint32_t n);
void strcpy(char* dest, const char* src);
void strncpy(char* dest, const char* src, uint32_t n);
char* strstr(const char* haystack, const char* needle);
void itoa(uint32_t num, char* str);
int atoi(const char* str);
void memset(void* dest, uint8_t val, uint32_t len);
void memcpy(void* dest, const void* src, uint32_t len);

#endif