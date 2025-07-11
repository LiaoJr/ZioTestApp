/* Copyright Joyent, Inc. and other Node contributors. All rights reserved.
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

#ifdef _WIN32
# include <io.h>
# include <windows.h>
#else /*  Unix */
# include <fcntl.h>
# include <unistd.h>
# if (defined(__linux__) || defined(__GLIBC__)) && !defined(__ANDROID__)
#  include <pty.h>
# elif defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__)
#  include <util.h>
# elif defined(__FreeBSD__) || defined(__DragonFly__)
#  include <libutil.h>
# endif
#endif

#include <string.h>
#include <errno.h>


TEST_IMPL(tty) {
  int r, width, height;
  int ttyin_fd, ttyout_fd;
  uv_tty_t tty_in, tty_out;
  uv_loop_t* loop = uv_default_loop();

  /* Make sure we have an FD that refers to a tty */
#ifdef _WIN32
  HANDLE handle;
  handle = CreateFileA("conin$",
                       GENERIC_READ | GENERIC_WRITE,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
  ASSERT(handle != INVALID_HANDLE_VALUE);
  ttyin_fd = _open_osfhandle((intptr_t) handle, 0);

  handle = CreateFileA("conout$",
                       GENERIC_READ | GENERIC_WRITE,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
  ASSERT(handle != INVALID_HANDLE_VALUE);
  ttyout_fd = _open_osfhandle((intptr_t) handle, 0);

#else /* unix */
  ttyin_fd = open("/dev/tty", O_RDONLY, 0);
  if (ttyin_fd < 0) {
    fprintf(stderr, "Cannot open /dev/tty as read-only: %s\n", strerror(errno));
    fflush(stderr);
    return TEST_SKIP;
  }

  ttyout_fd = open("/dev/tty", O_WRONLY, 0);
  if (ttyout_fd < 0) {
    fprintf(stderr, "Cannot open /dev/tty as write-only: %s\n", strerror(errno));
    fflush(stderr);
    return TEST_SKIP;
  }
#endif

  ASSERT(ttyin_fd >= 0);
  ASSERT(ttyout_fd >= 0);

  ASSERT(UV_UNKNOWN_HANDLE == uv_guess_handle(-1));

  ASSERT(UV_TTY == uv_guess_handle(ttyin_fd));
  ASSERT(UV_TTY == uv_guess_handle(ttyout_fd));

  r = uv_tty_init(uv_default_loop(), &tty_in, ttyin_fd, 1);  /* Readable. */
  ASSERT(r == 0);

  r = uv_tty_init(uv_default_loop(), &tty_out, ttyout_fd, 0);  /* Writable. */
  ASSERT(r == 0);

  r = uv_tty_get_winsize(&tty_out, &width, &height);
  ASSERT(r == 0);

  printf("width=%d height=%d\n", width, height);

  if (width == 0 && height == 0) {
   /* Some environments such as containers or Jenkins behave like this
    * sometimes */
    MAKE_VALGRIND_HAPPY();
    return TEST_SKIP;
  }

  /*
   * Is it a safe assumption that most people have terminals larger than
   * 10x10?
   */
  ASSERT(width > 10);
  ASSERT(height > 10);

  /* Turn on raw mode. */
  r = uv_tty_set_mode(&tty_in, UV_TTY_MODE_RAW);
  ASSERT(r == 0);

  /* Turn off raw mode. */
  r = uv_tty_set_mode(&tty_in, UV_TTY_MODE_NORMAL);
  ASSERT(r == 0);

  /* Calling uv_tty_reset_mode() repeatedly should not clobber errno. */
  errno = 0;
  ASSERT(0 == uv_tty_reset_mode());
  ASSERT(0 == uv_tty_reset_mode());
  ASSERT(0 == uv_tty_reset_mode());
  ASSERT(0 == errno);

  /* TODO check the actual mode! */

  uv_close((uv_handle_t*) &tty_in, NULL);
  uv_close((uv_handle_t*) &tty_out, NULL);

  uv_run(loop, UV_RUN_DEFAULT);

  MAKE_VALGRIND_HAPPY();
  return 0;
}


#ifdef _WIN32
static void tty_raw_alloc(uv_handle_t* handle, size_t size, uv_buf_t* buf) {
  buf->base = malloc(size);
  buf->len = size;
}

static void tty_raw_read(uv_stream_t* tty_in, ssize_t nread, const uv_buf_t* buf) {
  if (nread > 0) {
    ASSERT(nread  == 1);
    ASSERT(buf->base[0] == ' ');
    uv_close((uv_handle_t*) tty_in, NULL);
  } else {
    ASSERT(nread == 0);
  }
}

TEST_IMPL(tty_raw) {
  int r;
  int ttyin_fd;
  uv_tty_t tty_in;
  uv_loop_t* loop = uv_default_loop();
  HANDLE handle;
  INPUT_RECORD record;
  DWORD written;

  /* Make sure we have an FD that refers to a tty */
  handle = CreateFileA("conin$",
                       GENERIC_READ | GENERIC_WRITE,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
  ASSERT(handle != INVALID_HANDLE_VALUE);
  ttyin_fd = _open_osfhandle((intptr_t) handle, 0);
  ASSERT(ttyin_fd >= 0);
  ASSERT(UV_TTY == uv_guess_handle(ttyin_fd));

  r = uv_tty_init(uv_default_loop(), &tty_in, ttyin_fd, 1);  /* Readable. */
  ASSERT(r == 0);

  r = uv_read_start((uv_stream_t*)&tty_in, tty_raw_alloc, tty_raw_read);
  ASSERT(r == 0);

  /* Give uv_tty_line_read_thread time to block on ReadConsoleW */
  Sleep(100);

  /* Turn on raw mode. */
  r = uv_tty_set_mode(&tty_in, UV_TTY_MODE_RAW);
  ASSERT(r == 0);

  /* Write ' ' that should be read in raw mode */
  record.EventType = KEY_EVENT;
  record.Event.KeyEvent.bKeyDown = TRUE;
  record.Event.KeyEvent.wRepeatCount = 1;
  record.Event.KeyEvent.wVirtualKeyCode = VK_SPACE;
  record.Event.KeyEvent.wVirtualScanCode = MapVirtualKeyW(VK_SPACE, MAPVK_VK_TO_VSC);
  record.Event.KeyEvent.uChar.UnicodeChar = L' ';
  record.Event.KeyEvent.dwControlKeyState = 0;
  WriteConsoleInputW(handle, &record, 1, &written);

  uv_run(loop, UV_RUN_DEFAULT);

  MAKE_VALGRIND_HAPPY();
  return 0;
}

TEST_IMPL(tty_empty_write) {
  int r;
  int ttyout_fd;
  uv_tty_t tty_out;
  char dummy[1];
  uv_buf_t bufs[1];
  uv_loop_t* loop;

  /* Make sure we have an FD that refers to a tty */
  HANDLE handle;

  loop = uv_default_loop();

  handle = CreateFileA("conout$",
                       GENERIC_READ | GENERIC_WRITE,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
  ASSERT(handle != INVALID_HANDLE_VALUE);
  ttyout_fd = _open_osfhandle((intptr_t) handle, 0);

  ASSERT(ttyout_fd >= 0);

  ASSERT(UV_TTY == uv_guess_handle(ttyout_fd));

  r = uv_tty_init(uv_default_loop(), &tty_out, ttyout_fd, 0);  /* Writable. */
  ASSERT(r == 0);

  bufs[0].len = 0;
  bufs[0].base = &dummy[0];

  r = uv_try_write((uv_stream_t*) &tty_out, bufs, 1);
  ASSERT(r == 0);

  uv_close((uv_handle_t*) &tty_out, NULL);

  uv_run(loop, UV_RUN_DEFAULT);

  MAKE_VALGRIND_HAPPY();
  return 0;
}

TEST_IMPL(tty_large_write) {
  int r;
  int ttyout_fd;
  uv_tty_t tty_out;
  char dummy[10000];
  uv_buf_t bufs[1];
  uv_loop_t* loop;

  /* Make sure we have an FD that refers to a tty */
  HANDLE handle;

  loop = uv_default_loop();

  handle = CreateFileA("conout$",
                       GENERIC_READ | GENERIC_WRITE,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
  ASSERT(handle != INVALID_HANDLE_VALUE);
  ttyout_fd = _open_osfhandle((intptr_t) handle, 0);

  ASSERT(ttyout_fd >= 0);

  ASSERT(UV_TTY == uv_guess_handle(ttyout_fd));

  r = uv_tty_init(uv_default_loop(), &tty_out, ttyout_fd, 0);  /* Writable. */
  ASSERT(r == 0);

  memset(dummy, '.', sizeof(dummy) - 1);
  dummy[sizeof(dummy) - 1] = '\n';

  bufs[0] = uv_buf_init(dummy, sizeof(dummy));

  r = uv_try_write((uv_stream_t*) &tty_out, bufs, 1);
  ASSERT(r == 10000);

  uv_close((uv_handle_t*) &tty_out, NULL);

  uv_run(loop, UV_RUN_DEFAULT);

  MAKE_VALGRIND_HAPPY();
  return 0;
}
#endif


TEST_IMPL(tty_file) {
#ifndef _WIN32
  uv_loop_t loop;
  uv_tty_t tty;
  int fd;

  ASSERT(0 == uv_loop_init(&loop));

  fd = open("test/fixtures/empty_file", O_RDONLY);
  if (fd != -1) {
    ASSERT(UV_EINVAL == uv_tty_init(&loop, &tty, fd, 1));
    ASSERT(0 == close(fd));
  }

/* Bug on AIX where '/dev/random' returns 1 from isatty() */
#ifndef _AIX
  fd = open("/dev/random", O_RDONLY);
  if (fd != -1) {
    ASSERT(UV_EINVAL == uv_tty_init(&loop, &tty, fd, 1));
    ASSERT(0 == close(fd));
  }
#endif /* _AIX */

  fd = open("/dev/zero", O_RDONLY);
  if (fd != -1) {
    ASSERT(UV_EINVAL == uv_tty_init(&loop, &tty, fd, 1));
    ASSERT(0 == close(fd));
  }

  fd = open("/dev/tty", O_RDONLY);
  if (fd != -1) {
    ASSERT(0 == uv_tty_init(&loop, &tty, fd, 1));
    ASSERT(0 == close(fd));
    uv_close((uv_handle_t*) &tty, NULL);
  }

  ASSERT(0 == uv_run(&loop, UV_RUN_DEFAULT));
  ASSERT(0 == uv_loop_close(&loop));

  MAKE_VALGRIND_HAPPY();
#endif
  return 0;
}

TEST_IMPL(tty_pty) {
#if defined(__APPLE__)                            || \
    defined(__DragonFly__)                        || \
    defined(__FreeBSD__)                          || \
    defined(__FreeBSD_kernel__)                   || \
    (defined(__linux__) && !defined(__ANDROID__)) || \
    defined(__NetBSD__)                           || \
    defined(__OpenBSD__)
  int master_fd, slave_fd, r;
  struct winsize w;
  uv_loop_t loop;
  uv_tty_t master_tty, slave_tty;

  ASSERT(0 == uv_loop_init(&loop));

  r = openpty(&master_fd, &slave_fd, NULL, NULL, &w);
  if (r != 0)
    RETURN_SKIP("No pty available, skipping.");

  ASSERT(0 == uv_tty_init(&loop, &slave_tty, slave_fd, 0));
  ASSERT(0 == uv_tty_init(&loop, &master_tty, master_fd, 0));
  /* Check if the file descriptor was reopened. If it is,
   * UV_STREAM_BLOCKING (value 0x80) isn't set on flags.
   */
  ASSERT(0 == (slave_tty.flags & 0x80));
  /* The master_fd of a pty should never be reopened.
   */
  ASSERT(master_tty.flags & 0x80);
  ASSERT(0 == close(slave_fd));
  uv_close((uv_handle_t*) &slave_tty, NULL);
  ASSERT(0 == close(master_fd));
  uv_close((uv_handle_t*) &master_tty, NULL);

  ASSERT(0 == uv_run(&loop, UV_RUN_DEFAULT));

  MAKE_VALGRIND_HAPPY();
#endif
  return 0;
}
