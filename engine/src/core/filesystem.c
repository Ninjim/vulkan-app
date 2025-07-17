#include "filesystem.h"

#include "logging.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

 b8 filesystem_exists(const char* path){
    struct stat buffer; // i think it throws an error if no buffer given
    return stat(path, &buffer) == 0;
 }

// Binary mode means we get raw bytes, so special characters (like \n or the sort) that are interpreted in normal read mode, can corrupt the file if opened in binary mode
// normal mode is for text files because we need those special characters to parse properly,
// binary mode is for things like images, audio files, etc.
 b8 filesystem_open(const char* path, file_modes mode, b8 binary, file_handle* out_handle){
    out_handle->is_valid = false;
    out_handle->handle = 0;
    const char* mode_str;

    if((mode & FILE_MODE_READ) && (mode & FILE_MODE_WRITE)){
        mode_str = binary ? "w+b" : "w+";
    } else if((mode & FILE_MODE_READ) && !(mode & FILE_MODE_WRITE)){
        mode_str = binary ? "rb" : "r";
    } else if(!(mode & FILE_MODE_READ) && (mode & FILE_MODE_WRITE)){
        mode_str = binary ? "wb" : "w";
    } else {
        ERROR("Invalid mode passed while trying to open file: %s", path);
        return false;
    }

    FILE* file = fopen(path, mode_str);
    if (!file){
        ERROR("Error opening file: %s", path);
        return false;
    }

    out_handle->handle = file;
    out_handle->is_valid = true;

    return true;
 }

 void filesystem_close(file_handle* handle){
    if(handle->handle){
        fclose((FILE*)handle->handle);
        handle->handle = 0;
        handle->is_valid = false;
    }
 }

 b8 filesystem_read_line(file_handle* handle, char** line_buf){
    if(handle->handle){
        char buffer[32000];
        if(fgets(buffer, 32000, (FILE*)handle->handle) != 0){
            u64 length = strlen(buffer);
            *line_buf = malloc((sizeof(char) * length) + 1);
            strcpy(*line_buf, buffer);
            return true;
        }
    }
    return false;
 }

 b8 filesystem_write_line(file_handle* handle, const char* text){

    if(handle->handle) {
        i32 result = fputs(text, (FILE*)handle->handle);
        if(result != EOF){ // doesn't actually check anything for eof i dont think, since this is a write, eof is just the error code it returns?
            result = fputc('\n', (FILE*)handle->handle);
        }

        fflush((FILE*)handle->handle);
        return result != EOF;
    }

    return false;
 }

 b8 filesystem_read(file_handle* handle, u64 data_size, void* out_data, u64* out_bytes_read){

    // out_data is already allocated with the correct size and we already know how long the file is somehow/
    if(handle->handle && out_data){
        *out_bytes_read = fread(out_data, 1, data_size, (FILE*)handle->handle); // second param is into how many objects to read???
        if(*out_bytes_read != data_size){
            return false;
        }
        return true;
    }
    return false;
 }

 b8 filesystem_read_all_bytes(file_handle* handle,KAPI u8** out_bytes, u64* out_bytes_read){

    if(handle->handle){
        fseek((FILE*)handle->handle, 0, SEEK_END); // second parameter is offset
        u64 size = ftell((FILE*)handle->handle); // how many bytes (chars) read??
        rewind((FILE*)handle->handle);

        *out_bytes = malloc(sizeof(u8) * size);
        *out_bytes_read = fread(*out_bytes, 1, size, (FILE*)handle->handle); // i swear we couldve just taken a u8* instead of a double pointer / actually no we couldnt because we want the pointer to we call the function with to point to the space where the read variables are, if we didnt do a double pointer the pointer in the function would point to the new space but the pointer we call the function with would still be pointing to 0
        if(*out_bytes_read != size){
            return false;
        }
        return true;
    }
    return false;
 }

 b8 filesystem_write(file_handle* handle, u64 data_size, const void* data, u64* out_bytes_written){

    if(handle->handle){
        *out_bytes_written = fwrite(data, 1, data_size, (FILE*)handle->handle); // second param is into how many objects to read???
        if(*out_bytes_written != data_size){
            return false;
        }
        fflush((FILE*)handle->handle);
        return true;
    }
    return false;
 }