#include <stdio.h>
#include <pthread.h>
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define _FILE_OFFSET_BITS 64

typedef unsigned long long file_bytes;

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
int part_download(char *download_address, range range, char *proxy, char *file_name) {
    FILE *file = fopen(file_name, "wb");
    fseeko(file, 0xFFFFFFF, SEEK_SET);
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, download_address);
    char range_string[60]; // maybe 60 is just enough...
    sprintf(range_string, "%llu-%llu", range.start, range.end);
    curl_easy_setopt(curl, CURLOPT_RANGE, range_string);
    curl_easy_setopt(curl, CURLOPT_PROXY, proxy);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    int result = curl_easy_perform(curl);
    if (result != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror((CURLcode) result));
    // if the result is error, caller can come up with new way to perform the download of this block.
    return result;
}

void copy_and_low(const char *source, char *dest, size_t size) {
    for (int offset = 0; offset < size; offset++) {
        dest[offset] = (char) tolower(source[offset]);
    }
}

// size is always 1
// userdata is a pointer to a file_bytes object.
static size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata) {
    char *head_data = malloc(size + 2);
    copy_and_low(buffer, head_data, size);
    head_data[size] = '\n';
    head_data[size + 1] = 0;
    char *head_occur_position = strstr(head_data, "content-length: ");
    char content_length_value[50];
    memccpy(content_length_value, head_occur_position + strlen("content-length: "), '\n', 50);
    file_bytes total_file_length = strtoull(content_length_value, NULL, 0);
    *(file_bytes *) userdata = total_file_length;
    /* received header is nitems * size long in 'buffer' NOT ZERO TERMINATED */
    /* 'userdata' is set with CURLOPT_HEADERDATA */
    return nitems * size;
}

int get_file_size(char *download_address) {
    CURL *curl = curl_easy_init();
    file_bytes total_file_length;
    curl_easy_setopt(curl, CURLOPT_URL, download_address);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &total_file_length);
    return total_file_length;
}

// things will be done in each single thread.
int in_thread(void *param) {
    params *params1 = param;
    int code = part_download(params1->download_address, params1->range, params1->proxy, params1->file_name);
    return code;
}

void thread_init(char *download_address, char **proxy_list, char *file_name) {

}
