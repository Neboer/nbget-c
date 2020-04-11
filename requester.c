#include <stdio.h>
#include <limits.h>
#include <curl/curl.h>

#define proxy_http 1
#define proxy_https 2
#define proxy_socks4 3
#define proxy_socks5 4


typedef struct {
    unsigned long long start;
    unsigned long long end;
} range;


int part_request(char *download_address, range range, char *proxy, FILE *file) {
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, download_address);
    char range_string[60]; // maybe 60 is just enough...
    sprintf(range_string, "%llu-%llu", range.start, range.end);
    curl_easy_setopt(curl, CURLOPT_RANGE, range_string);


}