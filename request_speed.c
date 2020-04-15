static size_t WriteCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
    /* we are not interested in the downloaded bytes itself,
       so we only return the size we would have saved ... */
    (void)ptr;  /* unused */
    (void)data; /* unused */
    return (size_t)(size * nmemb);
}

curl_off_t test_proxy_speed(char* download_address, )