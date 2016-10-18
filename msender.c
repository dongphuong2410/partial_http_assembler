#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <zmq.h>
#include "mfile.h"

typedef struct {
    char *zmq_url;
    char *url;
    char *host;
    char *filepath;
    size_t filesize;
    char *range;
    char *content_type;
} config_t;

void usage(void);
void clean_config(config_t *cfg);
void clean_zmq(void);
int init_socket(const char *url);
void parse_arg(int argc, char **argv, config_t *cfg);

void *context = NULL;
void *socket = NULL;

int main(int argc, char **argv)
{
    config_t *cfg = (config_t *)calloc(1, sizeof(config_t));
    parse_arg(argc, argv, cfg);

    if (!cfg->zmq_url) {
        usage();
        goto exit;
    }

    int rc;
    rc = init_socket(cfg->zmq_url);
    if (rc) {
        fprintf(stderr, "Cannot connect to %s!\n", cfg->zmq_url);
        goto exit;
    }

    int ret;
    zmq_msg_t msg;
    partial_data_t partial_data;
    if (cfg->url)
        strcpy(partial_data.url, cfg->url);
    if (cfg->host)
        strcpy(partial_data.host, cfg->host);
    if (cfg->filepath)
        strcpy(partial_data.filepath, cfg->filepath);
    partial_data.filesize = cfg->filesize;
    if (cfg->range)
        strcpy(partial_data.range, cfg->range);
    if (cfg->content_type)
        strcpy(partial_data.content_type, cfg->content_type);
    zmq_msg_init_data(&msg, &partial_data, sizeof(partial_data), NULL, NULL);
    zmq_msg_send(&msg, socket, 0);
    zmq_recv(socket, &ret, sizeof(int), 0);

exit:
    clean_config(cfg);
    clean_zmq();
    exit(0);
}

void parse_arg(int argc, char **argv, config_t *cfg)
{
    static struct option long_options[] = {
        {"zmq-url", required_argument, 0, 'z'},
        {"input-file", required_argument, 0, 'f'},
        {"url", required_argument, 0, 'u'},
        {"host", required_argument, 0, 'o'},
        {"filepath", required_argument, 0, 'p'},
        {"filesize", required_argument, 0, 'e'},
        {"range", required_argument, 0, 'r'},
        {"content-type", required_argument, 0, 'c'},
        {"help", no_argument, 0, 'h'},
        {0,0,0,0}
    };
    int c;
    int option_index = 0;
    while ((c = getopt_long(argc, argv, "z:u:h:p:o:e:r:c:h", long_options, &option_index)) != -1) {
        switch (c) {
            case 'z':
               cfg->zmq_url = strdup(optarg);
                break;
            case 'u':
               cfg->url = strdup(optarg);
                break;
            case 'o':
               cfg->host = strdup(optarg);
                break;
            case 'p':
               cfg->filepath = strdup(optarg);
                break;
            case 'e':
               cfg->filesize = atoi(optarg);
                break;
            case 'r':
               cfg->range = strdup(optarg);
                break;
            case 'c':
               cfg->content_type = strdup(optarg);
                break;
            case 'h':
                usage();
                exit(0);
        }
    }
}

void usage(void)
{
    printf("Usage: \n");
    printf("\tmsender -zfuoperc> [-h]\n");
    printf("\t\t-z\t\t--zmq-url\t\tZMQ socket url\n");
    printf("\t\t-f\t\t--input-file\t\tfile that contains data to send\n");
    printf("\t\t-u\t\t--url\t\tpartial file url\n");
    printf("\t\t-o\t\t--host\t\t\t partial file host\n");
    printf("\t\t-p\t\t--filepath\t\tpartial file path\n");
    printf("\t\t-e\t\t--filesize\t\tpartial file size\n");
    printf("\t\t-r\t\t--range\t\t\tpartial file range\n");
    printf("\t\t-c\t\t--content-type\t\tpartial file content type\n");
    printf("\t\t-h\t\t--help\t\t\tHelp\n");
}

/**
 * return 0 if success
 * return -1 if failed
 */
int init_socket(const char *url)
{
    context = zmq_ctx_new();
    socket = zmq_socket(context, ZMQ_REQ);
    int rc = zmq_connect(socket, url);
    if (rc != 0) {
        return -1;
    }
    return 0;
}

void clean_config(config_t *cfg)
{
    if (cfg->zmq_url) free(cfg->zmq_url);
    if (cfg->url) free(cfg->url);
    if (cfg->host) free(cfg->host);
    if (cfg->filepath) free(cfg->filepath);
    if (cfg->range) free(cfg->range);
    if (cfg->content_type) free(cfg->content_type);
    free(cfg);
}

void clean_zmq(void)
{
    if (socket) zmq_close(socket);
    if (context) zmq_ctx_destroy(context);
}
