#pragma once

#include "defines.h"

typedef struct file_handle{
    void* handle;
    b8 is_valid;
} file_handle;

typedef enum file_modes {
    FILE_MODE_READ = 0x1,
    FILE_MODE_WRITE = 0x2
} file_modes;

 b8 filesystem_exists(const char* path);

// Binary is for binary mode? Meaning? Like whether we should be reading byte code or not or?
 b8 filesystem_open(const char* path, file_modes mofe, b8 binary, file_handle* out_handle);

 void filesystem_close(file_handle* handle);

 b8 filesystem_read_line(file_handle* handle, char** line_buf);

 b8 filesystem_write_line(file_handle* handle, const char* text);

 b8 filesystem_read(file_handle* handle, u64 data_size, void* out_data, u64* out_bytes_read);

 b8 filesystem_read_all_bytes(file_handle* handle, u8** out_bytes, u64* out_bytes_read);

 b8 filesystem_write(file_handle* handle, u64 data_size, const void* data, u64* out_bytes_written);