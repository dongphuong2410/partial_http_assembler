#ifndef __MFILE_H__
#define __MFILE_H__

#define STR_LEN 1024

typedef struct {
    char url[STR_LEN];
    char host[STR_LEN];
    char filepath[STR_LEN];
    char range[STR_LEN];
    char content_type[STR_LEN];
    size_t filesize;
} partial_data_t;

#endif
