#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>
#include <fcntl.h>
#include <msgpack.h>

#include "common.h"
#include "monitor.h"

int sock, sock_listen;
struct sockaddr_in addr_client;

msgpack_sbuffer *sbuf;

void monitor_init(void)
{
  struct sockaddr_in addr;
  socklen_t len;

  sbuf = msgpack_sbuffer_new();

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
  msgpack_sbuffer_free(sbuf);

  close(sock);
  close(sock_listen);
}

msgpack_packer *monitor_method_new(char *method)
{
  msgpack_packer *pk;

  msgpack_sbuffer_clear(sbuf);

  pk = msgpack_packer_new(sbuf, msgpack_sbuffer_write);
  msgpack_pack_array(pk, 2);
  msgpack_pack_raw(pk, strlen(method));
  msgpack_pack_raw_body(pk, method, strlen(method));

  return pk;
}

void monitor_method_send(void)
{
  write(sock, sbuf->data, sbuf->size);
}

void monitor_method_recv(void)
{
  fd_set rfds;
  struct timeval tv;
  int retval;

  ssize_t size;
  char strname[100];

  msgpack_unpacker *up;
  msgpack_unpacked result;
  msgpack_object *name, *data;

  unsigned char scancode;

  FD_ZERO(&rfds);
  FD_SET(sock, &rfds);
  tv.tv_sec = 0;
  tv.tv_usec = 0;

  retval = select(sock + 1, &rfds, NULL, NULL, &tv);

  if(retval == -1) {
    errx(EXIT_FAILURE, "select");
  }
  else if(!retval) {
    return;
  }

  up = msgpack_unpacker_new(MONITOR_BUF_SIZE);

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

    if(name->type != MSGPACK_OBJECT_RAW || (data && data->type != MSGPACK_OBJECT_ARRAY)) {
      errx(EXIT_FAILURE, "invalid method");      
    }

    /* convert method name */
    memcpy(strname, name->via.raw.ptr, name->via.raw.size);
    strname[name->via.raw.size] = '\0';

    /* call method */
    if(!strcmp(strname, "CONNECT")) {
    }
    else if(!strcmp(strname, "KEYBOARD_SCANCODE")) {
      if(data->via.array.ptr->type != MSGPACK_OBJECT_NIL) {
	/* push FIFO */
	scancode = data->via.array.ptr->via.i64 & 0xff;
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

	DEBUGMON("[Monitor] KEYBOARD_SCANCODE %x\n", scancode);
      }
    }
    else {
      errx(EXIT_FAILURE, "unknown method '%s'", strname);
    }
  }

  msgpack_unpacker_free(up);
}

void monitor_display_draw(unsigned int x, unsigned int y, unsigned int color)
{
  msgpack_packer *pk;

  pk = monitor_method_new("DISPLAY_DRAW");

  msgpack_pack_array(pk, 3);
  msgpack_pack_int(pk, x);
  msgpack_pack_int(pk, y);
  msgpack_pack_int(pk, color);

  monitor_method_send();

  msgpack_packer_free(pk);
}
