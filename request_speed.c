static size_t WriteCallback(void *ptr, size_t size, size_t nmemb, void *data) {
    /* we are not interested in the downloaded bytes itself,
       so we only return the size we would have saved ... */
    (void) ptr;  /* unused */
    (void) data; /* unused */
    return (size_t) (size * nmemb);
}

struct test_params {
    char *download_address;
    char *proxy;
};

typedef struct {
    char **proxy_string_list;
    int proxy_count;
} proxy_list;


struct test_result {
    proxy_list proxyList;
    curl_off_t *download_speed_list;
};

void *test_one_proxy_speed(void *params_raw) {
    struct test_params *params = params_raw;
    CURL *curl = curl_easy_init();
    curl_off_t download_speed = 0;
    curl_easy_setopt(curl, CURLOPT_URL, params->download_address);
//    fprintf(stderr, "addr: %s proxy: %s\n", params->download_address, params->proxy);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_RANGE, "0-5000");
    curl_easy_setopt(curl, CURLOPT_PROXY, params->proxy);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 10);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 5);
    CURLcode result = curl_easy_perform(curl);
    if (result != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed while test: %s\n", curl_easy_strerror((CURLcode) result));
        download_speed = -1;
    } else {
        curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD_T, &download_speed);
        curl_easy_cleanup(curl);
    }
    curl_off_t *download_speed_raw = malloc(sizeof(curl_off_t));
    *download_speed_raw = download_speed;
    return (void *) download_speed_raw;
}

struct test_result test_proxy_list(char *download_address, char **proxy_list, int proxy_count) {
    pthread_t thread_list[proxy_count];
    char **useful_proxy_list = malloc(proxy_count * sizeof(char *));
    curl_off_t *useful_speed_list = malloc(proxy_count * sizeof(curl_off_t));
    for (int i = 0; i < proxy_count; ++i) {
        struct test_params params = {download_address, proxy_list[i]};
        pthread_create(&thread_list[i], NULL, test_one_proxy_speed, (void *) &params);
    }
    int proxy_index = 0;
    for (int i = 0; i < proxy_count; ++i) {
        void *download_speed_raw;
        pthread_join(thread_list[i], &download_speed_raw);
        curl_off_t download_speed = *((curl_off_t *) download_speed_raw);
        if (download_speed > 0) { // add more conditions to get fewer proxies.
            useful_proxy_list[proxy_index] = proxy_list[i];
            useful_speed_list[proxy_index] = download_speed;
            proxy_index++;
        }
    }
    struct test_result result = {{useful_proxy_list, proxy_index}, useful_speed_list};
    return result;
}