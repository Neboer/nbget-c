#define initial_download_block_size 0x400 // download 1MB(for each thread) for test usage.
#define maximum_initial_test_time 5 // in second, max wait time for download
#define expect_download_time_per_thread 30 // in sec. new thread will be expected to download for this time.

typedef struct {
    char **proxy_string_list;
    int proxy_count;
} proxy_list;

struct test_result {
    proxy_list proxyList;
    curl_off_t *download_speed_list;
};

// called first to get proxy is useable and get proxy speed.
struct test_result test_proxy_server(proxy_list proxyList, char *download_address, char *filename) {
    file_bytes *thread_block_size_list = malloc(proxyList.proxy_count * sizeof(file_bytes));
    char **useful_proxy_list = malloc(proxyList.proxy_count * sizeof(char *));
    curl_off_t *useful_speed_list = malloc(proxyList.proxy_count * sizeof(curl_off_t));
    for (int i = 0; i < proxyList.proxy_count; i++) {
        thread_block_size_list[i] = initial_download_block_size;
    }
    curl_off_t *test_speed_result = thread_arrange(download_address, proxyList.proxy_string_list,
                                                   filename, proxyList.proxy_count,
                                                   thread_block_size_list, 0);
    int proxy_index = 0;
    for (int i = 0; i < proxyList.proxy_count; i++) {
        if (test_speed_result[i] >
            0) { // and there maybe some other restrict condition such as the lowest download speed.
            useful_proxy_list[proxy_index] = proxyList.proxy_string_list[i];
            useful_speed_list[proxy_index] = test_speed_result[i];
            proxy_index++;
        }
    }
    struct test_result result = {{useful_proxy_list, proxy_index}, useful_speed_list};
    return result;
}

// calculate the block size of each thread to download according to their last download speed. This function will not check bytes overflow.
file_bytes *calculate_block_size_to_download_one_thread(curl_off_t *last_download_speed_list, int thread_count) {
    file_bytes *size_list = malloc(thread_count * sizeof(file_bytes));
    for (int i = 0; i < thread_count; i++) {
        size_list[i] = last_download_speed_list[i] * expect_download_time_per_thread;
    }
    return size_list;
}

// if total block size reach the end of file, trunk the block and trunk block count into file range. return new count of trunk, else return 0. If the value is not 0,
// that means the last count of block is downloading.
int trunk_block_to_file_size(file_bytes *block_size_list, int block_count, file_bytes current_position,
                              file_bytes total_length) {
    for (int i = 0; i < block_count; i++) {
        current_position += block_size_list[i];
        if (current_position > total_length) {
            block_size_list[i] = total_length - (current_position - block_size_list[i]);
            return i + 1;// return new length of the list.
        }
    }
    return 0; // no need to trunk.
}

void download_whole_file(char *download_address, struct test_result proxy_with_info, char *filename,
                         file_bytes total_length) {
    short download_finished = 0;
    file_bytes current_download_block_position = 0;
    // going to download block of file size list.
    file_bytes *block_size_list_thread = calculate_block_size_to_download_one_thread(
            proxy_with_info.download_speed_list, proxy_with_info.proxyList.proxy_count);


    while (!download_finished) {
        file_bytes total_trunk_size_this_loop = 0;
        for (int j = 0; j < proxyList.proxy_count; j++) {
            if (thread_block_size_list[j] > 0) {
                total_trunk_size_this_loop += thread_block_size_list[j];
                if (total_trunk_size_this_loop > total_length) {
                    // download will finish in this loop.
                    thread_block_size_list[j] = total_length - total_trunk_size_this_loop;
                    for (j++; j < proxy_count; j++) {
                        thread_block_size_list[j] = 0;
                    }
                    download_finished = 1;
                    fputs("download finished\n", stderr);
                    break;
                }
            }
        }
        // start wait block to download, this is a block function
        download_speed_list speedList = thread_arrange(download_address, proxy_list, filename, proxy_count,
                                                       thread_block_size_list, current_download_block_position);
        for (int i = 0; i < speedList.list_length; i++) {
            if (speedList.speed_list[i] > 0) {
                thread_block_size_list[i] = speedList.speed_list[i] * expect_download_time_per_thread;
            } else {
                thread_block_size_list[i] = 0;
            }
        }
        // current download finished, now move to next big block.
        current_download_block_position += total_trunk_size_this_loop;
    }
}

