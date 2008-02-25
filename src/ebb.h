/* Ebb Web Server
 * Copyright (c) 2007 Ry Dahl <ry.d4hl@gmail.com>
 * This software is released under the "MIT License". See README file for details.
 */
#ifndef ebb_h
#define ebb_h

#include <sys/socket.h>
#include <netinet/in.h>
#include <glib.h>

#define EV_STANDALONE 1
#include "ev.h"

#include "parser.h"


typedef struct ebb_server ebb_server;
typedef struct ebb_client ebb_client;

#define EBB_LOG_DOMAIN "Ebb"
#define ebb_error(str, ...)  \
  g_log(EBB_LOG_DOMAIN, G_LOG_LEVEL_ERROR, str, ## __VA_ARGS__);
#define ebb_warning(str, ...)  \
  g_log(EBB_LOG_DOMAIN, G_LOG_LEVEL_WARNING, str, ## __VA_ARGS__);
#define ebb_info(str, ...)  \
  g_log(EBB_LOG_DOMAIN, G_LOG_LEVEL_INFO, str, ## __VA_ARGS__);
#define ebb_debug(str, ...)  \
  g_log(EBB_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, str, ## __VA_ARGS__);

#define EBB_BUFFERSIZE (2*1024)
#define EBB_MAX_CLIENTS 200
#define EBB_TIMEOUT 30.0
#define EBB_MAX_ENV 100
#define EBB_TCP_COMMON          \
  unsigned open : 1;            \
  int fd;                       \
  struct sockaddr_in sockaddr;

/*** Ebb Client ***/
void ebb_client_close(ebb_client*);
int ebb_client_read(ebb_client *client, char *buffer, int length);
void ebb_client_write(ebb_client*, const char *data, int length);
void ebb_client_finished( ebb_client *client);

enum { EBB_REQUEST_METHOD
     , EBB_REQUEST_URI
     , EBB_FRAGMENT
     , EBB_REQUEST_PATH
     , EBB_QUERY_STRING
     , EBB_HTTP_VERSION
     , EBB_SERVER_NAME
     , EBB_SERVER_PORT
     , EBB_CONTENT_LENGTH
     };

struct ebb_client {
  EBB_TCP_COMMON
  
  ebb_server *server;
  http_parser parser;
  
  char request_buffer[EBB_BUFFERSIZE];
  ev_io read_watcher;
  size_t nread_head, nread_body;
  
  int content_length;
  
  ev_io write_watcher;
  GString *response_buffer;
  size_t written;
  
  void *data;
  
  
  ev_timer timeout_watcher;
  
  /* the ENV structure */
  int env_size;
  const char *env_fields[EBB_MAX_ENV];
  int  env_field_lengths[EBB_MAX_ENV];
  const char *env_values[EBB_MAX_ENV];
  int  env_value_lengths[EBB_MAX_ENV];
};

/*** Ebb Server ***/

typedef void (*ebb_request_cb)(ebb_client*, void*);

ebb_server* ebb_server_alloc();
void ebb_server_free(ebb_server*);
void ebb_server_init( ebb_server *server
                    , struct ev_loop *loop
                    , ebb_request_cb request_cb
                    , void *request_cb_data
                    );
int ebb_server_listen_on_port(ebb_server*, const int port);
int ebb_server_listen_on_socket(ebb_server*, const char *socketpath);
void ebb_server_unlisten(ebb_server*);

struct ebb_server {
  EBB_TCP_COMMON
  char *port;
  char *socketpath;
  ev_io request_watcher;
  ebb_client clients[EBB_MAX_CLIENTS];
  struct ev_loop *loop;
  void *request_cb_data;
  ebb_request_cb request_cb;
};

#endif ebb_h