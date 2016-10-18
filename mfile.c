#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <zmq.h>
#include <pthread.h>
#include <signal.h>
#include "mfile.h"

typedef struct {
    char *zmq_url;
} config_t;

void parse_arg(int argc, char **argv, config_t *cfg);
void usage(void);
int init_socket(const char *url);
void *recv_thread(void *param);
void sigHandler(int dummy);
void clean_config(config_t *cfg);
void clean_zmq(void);
void process_data(partial_data_t *data);

void *context;
void *socket;
int running = 1;

int main(int argc, char **argv)
{
    signal(SIGINT, sigHandler);

    config_t *cfg = (config_t *)malloc(sizeof(config_t));
    parse_arg(argc, argv, cfg);
    if (!cfg->zmq_url) {
        usage();
        goto exit;
    }

    int rc;
    rc = init_socket(cfg->zmq_url);
    if (rc) {
        fprintf(stderr, "Cannot open socket to %s!\n", cfg->zmq_url);
        fprintf(stderr, "%d", rc);
        goto exit;;
    }

    pthread_t thread;
    rc = pthread_create(&thread, NULL, recv_thread, NULL);
    if (rc) {
        fprintf(stderr, "Start thread error !\n");
        goto exit;;
    }
    pthread_join(thread, NULL);

exit:
    clean_config(cfg);
    clean_zmq();
    return 0;
}

void parse_arg(int argc, char **argv, config_t *cfg)
{
    static struct option long_options[] = {
        {"zmq-url", required_argument, 0, 'z'},
        {"help", no_argument, 0, 'h'},
        {0,0,0,0}
    };
    int c;
    int option_index = 0;
    while ((c = getopt_long(argc, argv, "z:h", long_options, &option_index)) != -1) {
        switch (c) {
            case 'z':
                cfg->zmq_url = strdup(optarg);
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
    printf("\tmfile -z <zmqurl> [-h]\n");
}

/**
 * return 0 if success
 * return -1 if failed
 */
int init_socket(const char *url)
{
    context = zmq_ctx_new();
    socket = zmq_socket(context, ZMQ_REP);
    int rc = zmq_bind(socket, url);
    if (rc != 0) {
        return -1;
    }
    return 0;
}

void *recv_thread(void *param)
{
    int rc;
    zmq_msg_t msg;
    char *buffer;
    partial_data_t partial_data;
    zmq_msg_init_data(&msg, &partial_data, sizeof(partial_data), NULL, NULL);
    int num = 0;
    while (running) {
        int size = zmq_msg_recv(&msg, socket, ZMQ_DONTWAIT);
        if (size != -1) {
            buffer = zmq_msg_data(&msg);
            memcpy(&partial_data, buffer, sizeof(partial_data));
            zmq_send(socket, &num, sizeof(int), 0);
#ifdef TESTMODE
            printf("zmq_msg_recv: %d\n", size);
            printf("URL : %s\n", partial_data.url);
            printf("Host: %s\n", partial_data.host);
            printf("filepath: %s\n", partial_data.filepath);
            printf("filesize: %lu\n", partial_data.filesize);
            printf("range: %s\n", partial_data.range);
            printf("content_type: %s\n", partial_data.content_type);
            fflush(stdout);
#endif
            process_data(&partial_data);
        }
        else {
            sleep(1);
        }
    }
    return NULL;
}

void sigHandler(int dummy)
{
    running = 0;
}

void clean_config(config_t *cfg)
{
    if (cfg->zmq_url) free(cfg->zmq_url);
    free(cfg);
}

void clean_zmq(void)
{
    if (socket) zmq_close(socket);
    if (context) zmq_ctx_destroy(context);
}

void process_data(partial_data_t *data)
{
}
