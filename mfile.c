#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <zmq.h>
#include <pthread.h>
#include <signal.h>

typedef struct {
    char *zmq_url;
} config_t;

config_t *parse_arg(int argc, char **argv);
void usage(void);
int init_socket(const char *url);
void *recv_thread(void *param);
void sigHandler(int dummy);
void clean(void);

void *context;
void *socket;
config_t *cfg;
int running = 1;

int main(int argc, char **argv)
{
    signal(SIGINT, sigHandler);

    cfg = parse_arg(argc, argv);
    int rc;

    rc = init_socket(cfg->zmq_url);
    if (rc) {
        fprintf(stderr, "Cannot open socket to %s!\n", cfg->zmq_url);
        exit(1);
    }

    pthread_t thread;
    rc = pthread_create(&thread, NULL, recv_thread, NULL);
    if (rc) {
        fprintf(stderr, "Start thread error !\n");
        exit(1);
    }
    pthread_join(thread, NULL);

    clean();
    return 0;
}

config_t *parse_arg(int argc, char **argv)
{
    static struct option long_options[] = {
        {"zmq-url", required_argument, 0, 'z'},
        {"help", no_argument, 0, 'h'},
        {0,0,0,0}
    };
    char *zmq_url = NULL;
    int c;
    int option_index = 0;
    while ((c = getopt_long(argc, argv, "z:h", long_options, &option_index)) != -1) {
        switch (c) {
            case 'z':
               zmq_url = strdup(optarg);
                break;
            case 'h':
                usage();
                exit(0);
        }
    }
    if (zmq_url) {
        config_t *cfg = (config_t *)malloc(sizeof(config_t));
        cfg->zmq_url = zmq_url;
        return cfg;
    }
    else {
        usage();
        exit(0);
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
    while (running) {
        sleep(1);
    }
    return NULL;
}

void sigHandler(int dummy)
{
    running = 0;
}

void clean(void)
{
    free(cfg->zmq_url);
    free(cfg);
    zmq_close(socket);
    zmq_ctx_destroy(context);
}
