//
// utils.h
//
// Copyright (c) 2017 Hurzhii Artem, Demicev Alexandr, Denisov Artem, Chufarov Evgeny
//

#ifndef IFJ17_UTIL_H
#define IFJ17_UTIL_H

#include <stdio.h>
#include <sys/stat.h>

size_t
file_size(FILE *handle);

char *
file_read(const char *filename);

char *
read_until_eof(FILE *stream);

#endif
