
.. _misc:

Miscellaneous utilities
=======================

This section contains miscellaneous functions that don't really belong in any
other section.


Data types
----------

.. c:type:: uv_buf_t

    Buffer data type.

    .. c:member:: char* uv_buf_t.base

        Pointer to the base of the buffer.

    .. c:member:: size_t uv_buf_t.len

        Total bytes in the buffer.

        .. note::
            On Windows this field is ULONG.

.. c:type:: void* (*uv_malloc_func)(size_t size)

        Replacement function for :man:`malloc(3)`.
        See :c:func:`uv_replace_allocator`.

.. c:type::  void* (*uv_realloc_func)(void* ptr, size_t size)

        Replacement function for :man:`realloc(3)`.
        See :c:func:`uv_replace_allocator`.

.. c:type::  void* (*uv_calloc_func)(size_t count, size_t size)

        Replacement function for :man:`calloc(3)`.
        See :c:func:`uv_replace_allocator`.

.. c:type:: void (*uv_free_func)(void* ptr)

        Replacement function for :man:`free(3)`.
        See :c:func:`uv_replace_allocator`.

.. c:type:: uv_file

    Cross platform representation of a file handle.

.. c:type:: uv_os_sock_t

    Cross platform representation of a socket handle.

.. c:type:: uv_os_fd_t

    Abstract representation of a file descriptor. On Unix systems this is a
    `typedef` of `int` and on Windows a `HANDLE`.

.. c:type:: uv_pid_t

    Cross platform representation of a `pid_t`.

    .. versionadded:: 1.16.0

.. c:type:: uv_rusage_t

    Data type for resource usage results.

    ::

        typedef struct {
            uv_timeval_t ru_utime; /* user CPU time used */
            uv_timeval_t ru_stime; /* system CPU time used */
            uint64_t ru_maxrss; /* maximum resident set size */
            uint64_t ru_ixrss; /* integral shared memory size (X) */
            uint64_t ru_idrss; /* integral unshared data size (X) */
            uint64_t ru_isrss; /* integral unshared stack size (X) */
            uint64_t ru_minflt; /* page reclaims (soft page faults) (X) */
            uint64_t ru_majflt; /* page faults (hard page faults) */
            uint64_t ru_nswap; /* swaps (X) */
            uint64_t ru_inblock; /* block input operations */
            uint64_t ru_oublock; /* block output operations */
            uint64_t ru_msgsnd; /* IPC messages sent (X) */
            uint64_t ru_msgrcv; /* IPC messages received (X) */
            uint64_t ru_nsignals; /* signals received (X) */
            uint64_t ru_nvcsw; /* voluntary context switches (X) */
            uint64_t ru_nivcsw; /* involuntary context switches (X) */
        } uv_rusage_t;

    Members marked with `(X)` are unsupported on Windows.
    See :man:`getrusage(2)` for supported fields on Unix

.. c:type:: uv_cpu_info_t

    Data type for CPU information.

    ::

        typedef struct uv_cpu_info_s {
            char* model;
            int speed;
            struct uv_cpu_times_s {
                uint64_t user;
                uint64_t nice;
                uint64_t sys;
                uint64_t idle;
                uint64_t irq;
            } cpu_times;
        } uv_cpu_info_t;

.. c:type:: uv_interface_address_t

    Data type for interface addresses.

    ::

        typedef struct uv_interface_address_s {
            char* name;
            char phys_addr[6];
            int is_internal;
            union {
                struct sockaddr_in address4;
                struct sockaddr_in6 address6;
            } address;
            union {
                struct sockaddr_in netmask4;
                struct sockaddr_in6 netmask6;
            } netmask;
        } uv_interface_address_t;

.. c:type:: uv_passwd_t

    Data type for password file information.

    ::

        typedef struct uv_passwd_s {
            char* username;
            long uid;
            long gid;
            char* shell;
            char* homedir;
        } uv_passwd_t;


API
---

.. c:function:: uv_handle_type uv_guess_handle(uv_file file)

    Used to detect what type of stream should be used with a given file
    descriptor. Usually this will be used during initialization to guess the
    type of the stdio streams.

    For :man:`isatty(3)` equivalent functionality use this function and test
    for ``UV_TTY``.

.. c:function:: int uv_replace_allocator(uv_malloc_func malloc_func, uv_realloc_func realloc_func, uv_calloc_func calloc_func, uv_free_func free_func)

    .. versionadded:: 1.6.0

    Override the use of the standard library's :man:`malloc(3)`,
    :man:`calloc(3)`, :man:`realloc(3)`, :man:`free(3)`, memory allocation
    functions.

    This function must be called before any other libuv function is called or
    after all resources have been freed and thus libuv doesn't reference
    any allocated memory chunk.

    On success, it returns 0, if any of the function pointers is NULL it
    returns UV_EINVAL.

    .. warning:: There is no protection against changing the allocator multiple
                 times. If the user changes it they are responsible for making
                 sure the allocator is changed while no memory was allocated with
                 the previous allocator, or that they are compatible.

.. c:function:: uv_buf_t uv_buf_init(char* base, unsigned int len)

    Constructor for :c:type:`uv_buf_t`.

    Due to platform differences the user cannot rely on the ordering of the
    `base` and `len` members of the uv_buf_t struct. The user is responsible for
    freeing `base` after the uv_buf_t is done. Return struct passed by value.

.. c:function:: char** uv_setup_args(int argc, char** argv)

    Store the program arguments. Required for getting / setting the process title.

.. c:function:: int uv_get_process_title(char* buffer, size_t size)

    Gets the title of the current process. You *must* call `uv_setup_args`
    before calling this function. If `buffer` is `NULL` or `size` is zero,
    `UV_EINVAL` is returned. If `size` cannot accommodate the process title and
    terminating `NULL` character, the function returns `UV_ENOBUFS`.

    .. warning::
        `uv_get_process_title` is not thread safe on any platform except Windows.

.. c:function:: int uv_set_process_title(const char* title)

    Sets the current process title. You *must* call `uv_setup_args` before
    calling this function. On platforms with a fixed size buffer for the process
    title the contents of `title` will be copied to the buffer and truncated if
    larger than the available space. Other platforms will return `UV_ENOMEM` if
    they cannot allocate enough space to duplicate the contents of `title`.

    .. warning::
        `uv_set_process_title` is not thread safe on any platform except Windows.

.. c:function:: int uv_resident_set_memory(size_t* rss)

    Gets the resident set size (RSS) for the current process.

.. c:function:: int uv_uptime(double* uptime)

    Gets the current system uptime.

.. c:function:: int uv_getrusage(uv_rusage_t* rusage)

    Gets the resource usage measures for the current process.

    .. note::
        On Windows not all fields are set, the unsupported fields are filled with zeroes.
        See :c:type:`uv_rusage_t` for more details.

.. c:function:: uv_pid_t uv_os_getpid(void)

    Returns the current process ID.

    .. versionadded:: 1.18.0

.. c:function:: uv_pid_t uv_os_getppid(void)

    Returns the parent process ID.

    .. versionadded:: 1.16.0

.. c:function:: int uv_cpu_info(uv_cpu_info_t** cpu_infos, int* count)

    Gets information about the CPUs on the system. The `cpu_infos` array will
    have `count` elements and needs to be freed with :c:func:`uv_free_cpu_info`.

.. c:function:: void uv_free_cpu_info(uv_cpu_info_t* cpu_infos, int count)

    Frees the `cpu_infos` array previously allocated with :c:func:`uv_cpu_info`.

.. c:function:: int uv_interface_addresses(uv_interface_address_t** addresses, int* count)

    Gets address information about the network interfaces on the system. An
    array of `count` elements is allocated and returned in `addresses`. It must
    be freed by the user, calling :c:func:`uv_free_interface_addresses`.

.. c:function:: void uv_free_interface_addresses(uv_interface_address_t* addresses, int count)

    Free an array of :c:type:`uv_interface_address_t` which was returned by
    :c:func:`uv_interface_addresses`.

.. c:function:: void uv_loadavg(double avg[3])

    Gets the load average. See: `<http://en.wikipedia.org/wiki/Load_(computing)>`_

    .. note::
        Returns [0,0,0] on Windows (i.e., it's not implemented).

.. c:function:: int uv_ip4_addr(const char* ip, int port, struct sockaddr_in* addr)

    Convert a string containing an IPv4 addresses to a binary structure.

.. c:function:: int uv_ip6_addr(const char* ip, int port, struct sockaddr_in6* addr)

    Convert a string containing an IPv6 addresses to a binary structure.

.. c:function:: int uv_ip4_name(const struct sockaddr_in* src, char* dst, size_t size)

    Convert a binary structure containing an IPv4 address to a string.

.. c:function:: int uv_ip6_name(const struct sockaddr_in6* src, char* dst, size_t size)

    Convert a binary structure containing an IPv6 address to a string.

.. c:function:: int uv_inet_ntop(int af, const void* src, char* dst, size_t size)
.. c:function:: int uv_inet_pton(int af, const char* src, void* dst)

    Cross-platform IPv6-capable implementation of :man:`inet_ntop(3)`
    and :man:`inet_pton(3)`. On success they return 0. In case of error
    the target `dst` pointer is unmodified.

.. c:macro:: UV_IF_NAMESIZE

    Maximum IPv6 interface identifier name length.  Defined as
    `IFNAMSIZ` on Unix and `IF_NAMESIZE` on Linux and Windows.

    .. versionadded:: 1.16.0

.. c:function:: int uv_if_indextoname(unsigned int ifindex, char* buffer, size_t* size)

    IPv6-capable implementation of :man:`if_indextoname(3)`. When called,
    `*size` indicates the length of the `buffer`, which is used to store the
    result.
    On success, zero is returned, `buffer` contains the interface name, and
    `*size` represents the string length of the `buffer`, excluding the NUL
    terminator byte from `*size`. On error, a negative result is
    returned. If `buffer` is not large enough to hold the result,
    `UV_ENOBUFS` is returned, and `*size` represents the necessary size in
    bytes, including the NUL terminator byte into the `*size`.

    On Unix, the returned interface name can be used directly as an
    interface identifier in scoped IPv6 addresses, e.g.
    `fe80::abc:def1:2345%en0`.

    On Windows, the returned interface cannot be used as an interface
    identifier, as Windows uses numerical interface identifiers, e.g.
    `fe80::abc:def1:2345%5`.

    To get an interface identifier in a cross-platform compatible way,
    use `uv_if_indextoiid()`.

    Example:

    ::

        char ifname[UV_IF_NAMESIZE];
        size_t size = sizeof(ifname);
        uv_if_indextoname(sin6->sin6_scope_id, ifname, &size);

    .. versionadded:: 1.16.0

.. c:function:: int uv_if_indextoiid(unsigned int ifindex, char* buffer, size_t* size)

    Retrieves a network interface identifier suitable for use in an IPv6 scoped
    address. On Windows, returns the numeric `ifindex` as a string. On all other
    platforms, `uv_if_indextoname()` is called. The result is written to
    `buffer`, with `*size` indicating the length of `buffer`. If `buffer` is not
    large enough to hold the result, then `UV_ENOBUFS` is returned, and `*size`
    represents the size, including the NUL byte, required to hold the
    result.

    See `uv_if_indextoname` for further details.

    .. versionadded:: 1.16.0

.. c:function:: int uv_exepath(char* buffer, size_t* size)

    Gets the executable path.

.. c:function:: int uv_cwd(char* buffer, size_t* size)

    Gets the current working directory, and stores it in `buffer`. If the
    current working directory is too large to fit in `buffer`, this function
    returns `UV_ENOBUFS`, and sets `size` to the required length, including the
    null terminator.

    .. versionchanged:: 1.1.0

        On Unix the path no longer ends in a slash.

    .. versionchanged:: 1.9.0 the returned length includes the terminating null
                        byte on `UV_ENOBUFS`, and the buffer is null terminated
                        on success.


.. c:function:: int uv_chdir(const char* dir)

    Changes the current working directory.

.. c:function:: int uv_os_homedir(char* buffer, size_t* size)

    Gets the current user's home directory. On Windows, `uv_os_homedir()` first
    checks the `USERPROFILE` environment variable using
    `GetEnvironmentVariableW()`. If `USERPROFILE` is not set,
    `GetUserProfileDirectoryW()` is called. On all other operating systems,
    `uv_os_homedir()` first checks the `HOME` environment variable using
    :man:`getenv(3)`. If `HOME` is not set, :man:`getpwuid_r(3)` is called. The
    user's home directory is stored in `buffer`. When `uv_os_homedir()` is
    called, `size` indicates the maximum size of `buffer`. On success `size` is set
    to the string length of `buffer`. On `UV_ENOBUFS` failure `size` is set to the
    required length for `buffer`, including the null byte.

    .. warning::
        `uv_os_homedir()` is not thread safe.

    .. versionadded:: 1.6.0

.. c:function:: int uv_os_tmpdir(char* buffer, size_t* size)

    Gets the temp directory. On Windows, `uv_os_tmpdir()` uses `GetTempPathW()`.
    On all other operating systems, `uv_os_tmpdir()` uses the first environment
    variable found in the ordered list `TMPDIR`, `TMP`, `TEMP`, and `TEMPDIR`.
    If none of these are found, the path `"/tmp"` is used, or, on Android,
    `"/data/local/tmp"` is used. The temp directory is stored in `buffer`. When
    `uv_os_tmpdir()` is called, `size` indicates the maximum size of `buffer`.
    On success `size` is set to the string length of `buffer` (which does not
    include the terminating null). On `UV_ENOBUFS` failure `size` is set to the
    required length for `buffer`, including the null byte.

    .. warning::
        `uv_os_tmpdir()` is not thread safe.

    .. versionadded:: 1.9.0

.. c:function:: int uv_os_get_passwd(uv_passwd_t* pwd)

    Gets a subset of the password file entry for the current effective uid (not
    the real uid). The populated data includes the username, euid, gid, shell,
    and home directory. On non-Windows systems, all data comes from
    :man:`getpwuid_r(3)`. On Windows, uid and gid are set to -1 and have no
    meaning, and shell is `NULL`. After successfully calling this function, the
    memory allocated to `pwd` needs to be freed with
    :c:func:`uv_os_free_passwd`.

    .. versionadded:: 1.9.0

.. c:function:: void uv_os_free_passwd(uv_passwd_t* pwd)

    Frees the `pwd` memory previously allocated with :c:func:`uv_os_get_passwd`.

    .. versionadded:: 1.9.0

.. uint64_t uv_get_free_memory(void)
.. c:function:: uint64_t uv_get_total_memory(void)

    Gets memory information (in bytes).

.. c:function:: uint64_t uv_hrtime(void)

    Returns the current high-resolution real time. This is expressed in
    nanoseconds. It is relative to an arbitrary time in the past. It is not
    related to the time of day and therefore not subject to clock drift. The
    primary use is for measuring performance between intervals.

    .. note::
        Not every platform can support nanosecond resolution; however, this value will always
        be in nanoseconds.

.. c:function:: void uv_print_all_handles(uv_loop_t* loop, FILE* stream)

    Prints all handles associated with the given `loop` to the given `stream`.

    Example:

    ::

        uv_print_all_handles(uv_default_loop(), stderr);
        /*
        [--I] signal   0x1a25ea8
        [-AI] async    0x1a25cf0
        [R--] idle     0x1a7a8c8
        */

    The format is `[flags] handle-type handle-address`. For `flags`:

    - `R` is printed for a handle that is referenced
    - `A` is printed for a handle that is active
    - `I` is printed for a handle that is internal

    .. warning::
        This function is meant for ad hoc debugging, there is no API/ABI
        stability guarantees.

    .. versionadded:: 1.8.0

.. c:function:: void uv_print_active_handles(uv_loop_t* loop, FILE* stream)

    This is the same as :c:func:`uv_print_all_handles` except only active handles
    are printed.

    .. warning::
        This function is meant for ad hoc debugging, there is no API/ABI
        stability guarantees.

    .. versionadded:: 1.8.0

.. c:function:: int uv_os_getenv(const char* name, char* buffer, size_t* size)

    Retrieves the environment variable specified by `name`, copies its value
    into `buffer`, and sets `size` to the string length of the value. When
    calling this function, `size` must be set to the amount of storage available
    in `buffer`, including the null terminator. If the environment variable
    exceeds the storage available in `buffer`, `UV_ENOBUFS` is returned, and
    `size` is set to the amount of storage required to hold the value. If no
    matching environment variable exists, `UV_ENOENT` is returned.

    .. warning::
        This function is not thread safe.

    .. versionadded:: 1.12.0

.. c:function:: int uv_os_setenv(const char* name, const char* value)

    Creates or updates the environment variable specified by `name` with
    `value`.

    .. warning::
        This function is not thread safe.

    .. versionadded:: 1.12.0

.. c:function:: int uv_os_unsetenv(const char* name)

    Deletes the environment variable specified by `name`. If no such environment
    variable exists, this function returns successfully.

    .. warning::
        This function is not thread safe.

    .. versionadded:: 1.12.0

.. c:function:: int uv_os_gethostname(char* buffer, size_t* size)

    Returns the hostname as a null-terminated string in `buffer`, and sets
    `size` to the string length of the hostname. When calling this function,
    `size` must be set to the amount of storage available in `buffer`, including
    the null terminator. If the hostname exceeds the storage available in
    `buffer`, `UV_ENOBUFS` is returned, and `size` is set to the amount of
    storage required to hold the value.

    .. versionadded:: 1.12.0
