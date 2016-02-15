#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <errno.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <msgpack.h>

#include "common.h"
#include "debug.h"
#include "gci.h"
#include "monitor.h"

static int sock, sock_listen;
static struct sockaddr_in addr_client;

static msgpack_unpacker *up;
static msgpack_sbuffer *sbuf, *draw_sbuf;
static msgpack_packer *draw_pk;

struct pixel {
  unsigned int x;
  unsigned int y;
  unsigned int color;
};
static struct pixel draw_queue[DISPLAY_WIDTH];
static unsigned int draw_queue_n = 0;

void monitor_init(void)
{
  struct sockaddr_in addr;
  socklen_t len;

  sbuf = msgpack_sbuffer_new();
  draw_sbuf = msgpack_sbuffer_new();
  up = msgpack_unpacker_new(MONITOR_BUF_SIZE);

  /* TCP socket */
  sock_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

  addr.sin_family = AF_INET;
  addr.sin_port = htons(MONITOR_PORT);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(sock_listen, (struct sockaddr *)&addr, sizeof(addr));

  printf("[System] Waiting monitor connection... \n");
  listen(sock_listen, 1);

  len = sizeof(addr_client);
  sock = accept(sock_listen, (struct sockaddr *)&addr_client, &len);

  printf("[System] Monitor connected %s\n", inet_ntoa(addr_client.sin_addr));
}

void monitor_close(void)
{
  shutdown(sock, SHUT_RDWR);
  shutdown(sock_listen, SHUT_RDWR);

  close(sock);
  close(sock_listen);

  msgpack_sbuffer_free(sbuf);
  msgpack_sbuffer_free(draw_sbuf);
  msgpack_unpacker_free(up);
}

static inline void monitor_method_add(char *method, msgpack_packer *pk)
{
  msgpack_pack_array(pk, 2);
  msgpack_pack_raw(pk, strlen(method));
  msgpack_pack_raw_body(pk, method, strlen(method));
}

static inline msgpack_packer *monitor_method_new(char *method, msgpack_sbuffer *sbuffer)
{
  msgpack_packer *pk;

  msgpack_sbuffer_clear(sbuffer);
  pk = msgpack_packer_new(sbuffer, msgpack_sbuffer_write);
  monitor_method_add(method, pk);

  return pk;
}

static inline void monitor_method_send(msgpack_sbuffer *sbuffer)
{
  write(sock, sbuffer->data, sbuffer->size);
}

void monitor_method_recv(void)
{
  fd_set rfds;
  int retval;
  static const struct timespec tv = {0, 0};

  ssize_t size;
  char strname[100];

  msgpack_unpacked result;
  msgpack_object *name, *data;

  FD_ZERO(&rfds);
  FD_SET(sock, &rfds);
  retval = pselect(sock + 1, &rfds, NULL, NULL, &tv, NULL);

  if(retval == 0) {
    /* no data */
    return;
  }
  else if(retval == -1) {
    if(errno != EINTR) {
      err(EXIT_FAILURE, "method_receive pselect");
    }
    return;
  }

  /* receive */
  size = read(sock, msgpack_unpacker_buffer(up), msgpack_unpacker_buffer_capacity(up));

  msgpack_unpacker_buffer_consumed(up, size);
  msgpack_unpacked_init(&result);

  /* stream unpacker */
  while(msgpack_unpacker_next(up, &result)) {
    if(DEBUG_MON) {
      printf("[Monitor] ");
      msgpack_object_print(stdout, result.data);
      puts("");
    }

    /* method: ["METHODNAME", [some, method, args]] */
    if(result.data.type != MSGPACK_OBJECT_ARRAY || result.data.via.array.size <= 0) {
      errx(EXIT_FAILURE, "invalid method");
    }

    /* method name */
    name = result.data.via.array.ptr;

    /* method data */
    if(result.data.via.array.size == 2) {
      data = result.data.via.array.ptr + 1;
    }
    else {
      data = NULL;
    }

    if(name->type != MSGPACK_OBJECT_RAW) {
      errx(EXIT_FAILURE, "invalid method");      
    }

    /* convert method name */
    memcpy(strname, name->via.raw.ptr, name->via.raw.size);
    strname[name->via.raw.size] = '\0';

    /* call method */
    if(!strcmp(strname, "CONNECT")) {
    }
    else if(!strcmp(strname, "DISCONNECT")) {
      exec_finish = true;
    }
    else if(data == NULL) {
      errx(EXIT_FAILURE, "invalid method (no data?)");
    }
    else if(!strcmp(strname, "KEYBOARD_SCANCODE")) {
      msgpack_object *obj;
      unsigned int i, n;

      unsigned char scancode;

      if(data->type == MSGPACK_OBJECT_ARRAY) {
	obj = data->via.array.ptr;
	n = data->via.array.size;
      }
      else if(data->type == MSGPACK_OBJECT_POSITIVE_INTEGER) {
	obj = data;
	n = 1;
      }
      else {
	n = 0;
      }

      for(i = 0; i < n; i++, obj++) {
	/* push FIFO */
	scancode = obj->via.i64 & 0xff;
	fifo_scancode[fifo_scancode_end++] = scancode;

	if(fifo_scancode_end >= KMC_FIFO_SCANCODE_SIZE) {
	  fifo_scancode_end = 0;
	}
	if(fifo_scancode_start == fifo_scancode_end) {
	  fifo_scancode_start++;
	  if(fifo_scancode_start >= KMC_FIFO_SCANCODE_SIZE) {
	    fifo_scancode_start = 0;
	  }
	}

	gci_nodes[GCI_KMC_NUM].int_dispatch = true;

	DEBUGMON("[Monitor] KEYBOARD_SCANCODE %x\n", scancode);
      }
    }
    else {
      errx(EXIT_FAILURE, "unknown method '%s'", strname);
    }
  }
}

void monitor_disconnect(void)
{
  msgpack_packer *pk;

  pk = monitor_method_new("DISCONNECT", sbuf);
  msgpack_pack_nil(pk);
  monitor_method_send(sbuf);
  msgpack_packer_free(pk);
}

void monitor_pack_draw_queue(void)
{
  unsigned int i;

  if(!draw_pk) {
    draw_pk = monitor_method_new("DISPLAY_DRAW", draw_sbuf);
  }
  else {
    monitor_method_add("DISPLAY_DRAW", draw_pk);
  }

  msgpack_pack_array(draw_pk, draw_queue_n);
  for(i = 0; i < draw_queue_n; i++) {
    msgpack_pack_array(draw_pk, 3);
    msgpack_pack_int(draw_pk, draw_queue[i].x);
    msgpack_pack_int(draw_pk, draw_queue[i].y);
    msgpack_pack_int(draw_pk, draw_queue[i].color);
  }

  draw_queue_n = 0;
}

void monitor_send_queue(void)
{
  if(draw_pk) {
    monitor_pack_draw_queue();
    monitor_method_send(draw_sbuf);
    msgpack_packer_free(draw_pk);
    draw_pk = NULL;
  }
}

void monitor_display_queue_draw(unsigned int x, unsigned int y, unsigned int color)
{
  draw_queue[draw_queue_n].x = x;
  draw_queue[draw_queue_n].y = y;
  draw_queue[draw_queue_n].color = color;

  if(++draw_queue_n >= DISPLAY_WIDTH) {
    monitor_pack_draw_queue();
  }
}
