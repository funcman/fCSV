#ifndef FCSV_H
#define FCSV_H

#include <string>
#include "fstr.h"

struct fCSV;
struct fRow;

struct fCSV*    fcsv_create();
struct fCSV*    fcsv_open(char const* filename);
struct fCSV*    fcsv_read(char const* data, unsigned int size);
void            fcsv_close(struct fCSV* csv);
void            fcsv_save(struct fCSV* csv, char const* filename);
struct fStr*    fcsv_to_str(struct fCSV* csv, struct fStr* str);

struct fRow*    fcsv_first_row(struct fCSV* csv);
struct fRow*    fcsv_next_row(struct fRow* row);
struct fRow*    fcsv_insert_row(struct fRow* row);
struct fRow*    fcsv_insert_row_in_front(struct fRow* row);
void            fcsv_delete_row(struct fRow* row);

std::string     fcsv_get_field(struct fRow* row, unsigned int index);
void            fcsv_set_field(struct fRow* row, unsigned int index, std::string value);
void            fcsv_delete_field(struct fRow* row, unsigned int index);

unsigned int    fcsv_rows_count(struct fCSV* csv);
unsigned int    fcsv_fields_count(struct fRow* row);
void            fcsv_set_fields_count(struct fRow* row, unsigned int count);

#endif//FCSV_H
