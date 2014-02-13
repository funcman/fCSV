#include "fcsv.h"
#include <stdio.h>
#include <vector>

struct fCSV;

struct fRow {
    std::vector<std::string> fields;
    struct fRow* prev;
    struct fRow* next;
    struct fCSV* csv;
    fRow() : prev(0), next(0), csv(0) {}
};

struct fCSV {
    int count;
    struct fRow* first;
    fCSV() : count(0), first(0) {}
};

static bool new_row_ = false;

enum SEPARATING_CHARACTER {
    SC_COMMA,
    SC_WIN_NEWLINE,
    SC_UNIX_NEWLINE,
    SC_EOF,
};

static enum SEPARATING_CHARACTER old_sc = SC_EOF;
static enum SEPARATING_CHARACTER cur_sc = SC_EOF;

static enum SEPARATING_CHARACTER get_sepchar_(char* str, unsigned int len, int* pos, int* last_pos) {
    for ( unsigned int i=0; i<len; ++i ) {
        if ( str[i] == ',' ) {
            *pos = i;
            *last_pos = i+1;
            return SC_COMMA;
        }else if ( str[i] == '\r' && i+1<len && str[i+1] == '\n' ) {
            *pos = i;
            *last_pos = i+2;
            return SC_WIN_NEWLINE;
        }else if ( str[i] == '\n' ) {
            *pos = i;
            *last_pos = i+1;
            return SC_UNIX_NEWLINE;
        }
    }
    *pos = len;
    *last_pos = len;
    return SC_EOF;
}

static int get_comma_pos_(char* str, unsigned int len) {
    for ( unsigned int i=0; i<len; ++i ) {
        if ( str[i] == '\"' ) {
            if( i+1<len && str[i+1]=='\"' ) {
                i++;
            }else {
                return i;
            }
        }
    }
    return -1;
}

static int std_string_replace_(std::string& base, std::string src, std::string des) {
    std::string::size_type pos = base.find(src, 0);
    std::string::size_type src_len = src.size();
    std::string::size_type des_len = des.size();
    int resulte = 0;
    while ((pos != std::string::npos)) {
        base.replace(pos, src_len, des);
        ++resulte;
        pos = base.find(src, (pos+des_len));
    }
    return resulte;
}

static std::string get_a_escaped_field_(char* str, unsigned int len, char** last_str) {
    std::string s;
    int p = get_comma_pos_(str, len);
    if( p < 0 ) {
        *last_str = &str[len];
        return s;
    }
    int pos;
    int last_pos;
    s.resize(p);
    s.assign(str, p);
    std_string_replace_(s, "\"\"", "\"");
    old_sc = cur_sc;
    cur_sc = get_sepchar_(&str[p], len-p, &pos, &last_pos);
    *last_str = &str[p+last_pos];
    if ( cur_sc == SC_WIN_NEWLINE || cur_sc == SC_UNIX_NEWLINE ) {
        new_row_ = true;
    }
    return s;
}

static std::string get_a_non_escaped_field_(char* str, unsigned int len, char** last_str) {
    int pos;
    int last_pos;
    std::string s;
    old_sc = cur_sc;
    cur_sc = get_sepchar_(str, len, &pos, &last_pos);
    s.resize(pos);
    s.assign(str, pos);
    *last_str = &str[last_pos];
    if ( cur_sc == SC_WIN_NEWLINE || cur_sc == SC_UNIX_NEWLINE ) {
        new_row_ = true;
    }
    return s;
}

static std::string get_a_field_(char* str, unsigned int len, char** last_str) {
    if( len >= 1 && str[0] == '\"' ) {
        return get_a_escaped_field_(str+1, len-1, last_str);
    }else {
        return get_a_non_escaped_field_(str, len, last_str);
    }
}

static std::string writable_string_(std::string const& str) {
    std::string s = str;
    if ( std_string_replace_(s, "\"", "\"\"")   ||
         str.find(",", 0) != std::string::npos  ||
         str.find("\n", 0) != std::string::npos ) {
        return "\""+s+"\"";
    }
    return str;
}

struct fCSV* fcsv_create() {
    struct fCSV* csv = new struct fCSV;
    fcsv_first_row(csv);
    return csv;
}

struct fCSV* fcsv_open(char const* filename) {
    struct fCSV* csv = new struct fCSV;
    struct fRow* row = fcsv_first_row(csv);
    FILE* fp;
    fp = fopen(filename, "rb");
    if( fp ) {
        char* csv_str;
        int size;
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        csv_str = new char[size];
        fseek(fp, 0, SEEK_SET);
        fread(csv_str, size, 1, fp);
        char* s1 = csv_str;
        char* s2 = s1;
        unsigned int len = size;
        old_sc = SC_EOF;
        cur_sc = SC_EOF;
        do {
            row->fields.push_back(get_a_field_(s1, len, &s2));
            if( new_row_ ) {
                row = fcsv_insert_row(row);
                new_row_ = false;
            }
            len -= (s2-s1);
            s1=s2;
        }while ( cur_sc != SC_EOF );
        delete[] csv_str;
        fclose(fp);
    }
    return csv;
}

void fcsv_close(struct fCSV* csv) {
    while ( csv->first ) {
        fcsv_delete_row(csv->first);
    }
}

void fcsv_save(struct fCSV* csv, char const* filename) {
    FILE* fp;
    fp = fopen(filename, "wb");
    if ( fp ) {
        struct fRow* row = csv->first;
        while ( row ) {
            for ( unsigned int i=0; i<row->fields.size(); ++i ) {
                std::string field = writable_string_(row->fields[i]);
                fwrite(field.c_str(), field.length(), 1, fp);
                if ( i+1<row->fields.size() ) fwrite(",", 1, 1, fp);
            }
            row = fcsv_next_row(row);
            if ( row ) fwrite("\r\n", 2, 1, fp);
        }
        fclose(fp);
    }
}

struct fRow* fcsv_first_row(struct fCSV* csv) {
    if ( !csv->first ) {
        csv->first = new struct fRow;
        csv->first->csv = csv;
        csv->count++;
    }
    return csv->first;
}

struct fRow* fcsv_next_row(struct fRow* row) {
    return row->next;
}

struct fRow* fcsv_insert_row(struct fRow* row) {
    struct fRow* new_row = new struct fRow;
    if ( row->next ) {
        row->next->prev = new_row;
        new_row->next = row->next;
    }
    row->next = new_row;
    new_row->prev = row;
    new_row->csv = row->csv;
    row->csv->count++;
    return new_row;
}

struct fRow* fcsv_insert_row_in_front(struct fRow* row) {
    if ( row->prev ) return fcsv_insert_row(row->prev);
    row->prev = new struct fRow;
    row->prev->next = row;
    row->prev->csv = row->csv;
    row->csv->count++;
    row->csv->first = row->prev;
    return row->prev;
}

void fcsv_delete_row(struct fRow* row) {
    if ( row->prev && row->next ) {
        row->prev->next = row->next;
        row->next->prev = row->prev;
    }else if ( row->prev ) {
        row->prev->next = 0;
    }else if ( row->next ) {
        row->next->prev = 0;
    }
    if ( row->csv->first == row ) row->csv->first = row->next;
    delete row;
}

std::string fcsv_get_field(struct fRow* row, unsigned int index) {
    if ( index >= row->fields.size() ) return "";
    return row->fields[index];
}

void fcsv_set_field(struct fRow* row, unsigned int index, std::string value) {
    if ( index >= row->fields.size() ) row->fields.resize(index+1);
    row->fields[index] = value;
}

void fcsv_delete_field(struct fRow* row, unsigned int index) {
    if ( index >= row->fields.size() ) return;
    row->fields.erase(row->fields.begin()+index);
}

unsigned int fcsv_rows_count(struct fCSV* csv) {
    return csv->count;
}

unsigned int fcsv_fields_count(struct fRow* row) {
    return row->fields.size();
}

void fcsv_set_fields_count(struct fRow* row, unsigned int count) {
    row->fields.resize(count);
}
