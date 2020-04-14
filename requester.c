#include <stdio.h>
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define _FILE_OFFSET_BITS 64

typedef unsigned long long file_bytes;

// standard string index range. for example cde in abcdefg is at 2,5
typedef struct {
    unsigned long long start;
    unsigned long long end;
} range;

typedef struct {
    char *download_address;
    range range;
    char *proxy;
    char *file_name;
} params;

// range is very annoying. HTTP range and FILE* range are different.
// For example. HTTP range 42-45 contains 4 bytes. The pointer must be at 41 when write data.
curl_off_t part_download(char *download_address, range range, char *proxy, char *file_name) {
    // rb+ explanation: open in this mode, the file won't get destroyed. But it requires a long enough file in advance.
    FILE *file = fopen(file_name, "rb+");
    // fseeko can take larger param as position up to 64 bytes integer.
    fseeko(file, range.start, SEEK_SET);
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, download_address);
    char range_string[60]; // maybe 60 is just enough...
    sprintf(range_string, "%llu-%llu", range.start, range.end);
    curl_easy_setopt(curl, CURLOPT_RANGE, range_string);
    curl_easy_setopt(curl, CURLOPT_PROXY, proxy);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    curl_off_t download_speed;
    // bytes per second
    int result = curl_easy_perform(curl);
    if (result != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror((CURLcode) result));
        download_speed = -1;
    } else {
        curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD_T, &download_speed);
        curl_easy_cleanup(curl);
        fclose(file);
    }
    // TODO: for test usage.
    fprintf(stderr, "%s\n", range_string);
    return download_speed;
}

void copy_and_low(const char *source, char *dest, size_t length) {
    for (int offset = 0; offset < length; offset++) {
        dest[offset] = (char) tolower(source[offset]);
    }
}

// size is always 1
// userdata is a pointer to a file_bytes object.
static size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata) {
    char *head_data = malloc(nitems + 1);
    copy_and_low(buffer, head_data, nitems);
    head_data[nitems - 2] = 0;
    char *head_occur_position = strstr(head_data, "content-length: ");
    if (head_occur_position) {
        file_bytes total_file_length = strtoull(head_occur_position + sizeof("content-length: ") - 1, NULL, 0);
        *(file_bytes *) userdata = total_file_length;
    }
    return nitems * size;
}

int get_file_size(char *download_address) {
    CURL *curl = curl_easy_init();
    file_bytes total_file_length;
    curl_easy_setopt(curl, CURLOPT_URL, download_address);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &total_file_length);
    curl_easy_perform(curl);
    fputs("body request\n", stderr);
    return total_file_length;
}