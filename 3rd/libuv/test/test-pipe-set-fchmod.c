/* Copyright libuv project contributors. All rights reserved.
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

TEST_IMPL(pipe_set_chmod) {
  uv_pipe_t pipe_handle;
  uv_loop_t* loop;
  int r;

  loop = uv_default_loop();

  r = uv_pipe_init(loop, &pipe_handle, 0);
  ASSERT(r == 0);

  r = uv_pipe_bind(&pipe_handle, TEST_PIPENAME);
  ASSERT(r == 0);

  /* No easy way to test if this works, we will only make sure that */
  /* the call is successful. */
  r = uv_pipe_chmod(&pipe_handle, UV_READABLE);
  if (r == UV_EPERM) {
    MAKE_VALGRIND_HAPPY();
    RETURN_SKIP("Insufficient privileges to alter pipe fmode");
  }
  ASSERT(r == 0);

  r = uv_pipe_chmod(&pipe_handle, UV_WRITABLE);
  ASSERT(r == 0);

  r = uv_pipe_chmod(&pipe_handle, UV_WRITABLE | UV_READABLE);
  ASSERT(r == 0);

  r = uv_pipe_chmod(NULL, UV_WRITABLE | UV_READABLE);
  ASSERT(r == UV_EBADF);

  r = uv_pipe_chmod(&pipe_handle, 12345678);
  ASSERT(r == UV_EINVAL);

  uv_close((uv_handle_t*)&pipe_handle, NULL);
  r = uv_pipe_chmod(&pipe_handle, UV_WRITABLE | UV_READABLE);
  ASSERT(r == UV_EBADF);

  MAKE_VALGRIND_HAPPY();
  return 0;
}
