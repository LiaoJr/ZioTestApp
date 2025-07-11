/* Copyright The libuv project and contributors. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "uv.h"
#include "task.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK_OBJECT(handle, type, parent) \
  ASSERT((type*)(handle) == &(parent))

static uv_udp_t client;
static uv_idle_t idle_handle;
static uv_udp_send_t send_req;
static uv_buf_t buf;
static struct sockaddr_in addr;
static char send_data[1024];

static int loop_hang_called;

static void send_cb(uv_udp_send_t* req, int status);


static void idle_cb(uv_idle_t* handle) {
  int r;

  ASSERT(send_req.handle == NULL);
  CHECK_OBJECT(handle, uv_idle_t, idle_handle);
  ASSERT(0 == uv_idle_stop(handle));

  /* It probably would have stalled by now if it's going to stall at all. */
  if (++loop_hang_called > 1000) {
    uv_close((uv_handle_t*) &client, NULL);
    uv_close((uv_handle_t*) &idle_handle, NULL);
    return;
  }

  r = uv_udp_send(&send_req,
                  &client,
                  &buf,
                  1,
                  (const struct sockaddr*) &addr,
                  send_cb);
  ASSERT(r == 0);
}


static void send_cb(uv_udp_send_t* req, int status) {
  ASSERT(req != NULL);
  ASSERT(status == 0);
  CHECK_OBJECT(req->handle, uv_udp_t, client);
  CHECK_OBJECT(req, uv_udp_send_t, send_req);
  req->handle = NULL;

  ASSERT(0 == uv_idle_start(&idle_handle, idle_cb));
}


TEST_IMPL(udp_send_hang_loop) {
  ASSERT(0 == uv_idle_init(uv_default_loop(), &idle_handle));

  /* 192.0.2.0/8 is "TEST-NET" and reserved for documentation.
   * Good for us, though. Since we want to have something unreachable.
   */
  ASSERT(0 == uv_ip4_addr("192.0.2.3", TEST_PORT, &addr));

  ASSERT(0 == uv_udp_init(uv_default_loop(), &client));

  buf = uv_buf_init(send_data, sizeof(send_data));

  ASSERT(0 == uv_idle_start(&idle_handle, idle_cb));

  uv_run(uv_default_loop(), UV_RUN_DEFAULT);

  ASSERT(loop_hang_called > 1000);

  MAKE_VALGRIND_HAPPY();
  return 0;
}
