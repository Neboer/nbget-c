#include <pthread.h>

// things will be done in each single thread.
void *block_download(void *param) {
    params *params1 = param;
    curl_off_t download_speed = part_download(params1->download_address, params1->range, params1->proxy,
                                              params1->file_name);
    curl_off_t *download_speed_address = malloc(sizeof(curl_off_t));
    *download_speed_address = download_speed;
    free(param);
    return (void *) download_speed_address;
}
// download control. start_index is a normal selector. start_index will be used in program as a bytes counter.
// this function is blocked. until tile limit exceed or all download are finished.
curl_off_t* blocked_multi_download(char *download_address, char **proxy_list, char *filename, int proxy_count,
                                   file_bytes *thread_block_size, file_bytes start_index) {
    file_bytes current_index = start_index;
    pthread_t *thread_list = malloc(sizeof(pthread_t) * proxy_count);
    // download_speed_list store the real speed value instead of a pointer like block_download returns.
    curl_off_t *download_speed_value_list = malloc(sizeof(curl_off_t) * proxy_count);
    for (int i = 0; i < proxy_count; i++) {
        file_bytes current_thread_download_size = thread_block_size[i];
        if (current_thread_download_size > 0) {
            char *proxy_string = proxy_list[i];
            void *param_addr = malloc(sizeof(params));
            (*(params *) param_addr).download_address = download_address;
            (*(params *) param_addr).range.start = current_index;
            (*(params *) param_addr).range.end = current_index + current_thread_download_size;
            (*(params *) param_addr).proxy = proxy_string;
            (*(params *) param_addr).file_name = filename;
            // now start the process
            pthread_create(&thread_list[i], NULL, (void *(*)(void *)) block_download, param_addr);
            current_index += current_thread_download_size;
        }
    }
    for (int j = 0; j < proxy_count; j++) {
        if (thread_block_size[j] > 0) {
            void *speed_value_addr;
            pthread_join(thread_list[j], &speed_value_addr);
            // now, speed_value_addr points to the real speed value.
            curl_off_t speed_value = *(curl_off_t *) speed_value_addr;
            download_speed_value_list[j] = speed_value;
            free(speed_value_addr);// a space of this address is opened by malloc(), now it's useless.
        } else {
            // if thread failed to download his block, the code will be kept
            download_speed_value_list[j] = thread_block_size[j];
        }
    }
    return download_speed_value_list;
}