#define expect_download_time_per_thread 30 // in sec. new thread will be expected to download for this time.

// calculate the block size of each thread to download according to their last download speed. This function will not check bytes overflow.
file_bytes *calculate_block_size_to_download_one_thread(curl_off_t *last_download_speed_list, int thread_count) {
    file_bytes *size_list = malloc(thread_count * sizeof(file_bytes));
    for (int i = 0; i < thread_count; i++) {
        size_list[i] = (file_bytes) (last_download_speed_list[i] * expect_download_time_per_thread);
    }
    return size_list;
}


// if total block size reach the end of file, trunk the block and trunk block count into file range. return new count of trunk, else return 0. If the value is not 0,
// that means the last count of block is downloading.
file_bytes trunk_to_file_size(file_bytes *block_size_list, int *proxy_count, file_bytes current_position,
                              file_bytes total_length) {
    int old_proxy_count = *proxy_count;
    for (int i = 0; i < old_proxy_count; i++) {
        current_position += block_size_list[i];
        if (current_position > total_length) {
            block_size_list[i] = total_length - (current_position - block_size_list[i]);
            *proxy_count = i + 1;// that's why proxy_count is a pointer...
            return total_length;
        }
    }
    return current_position;
}

// checkpoint is where last block download start.
void download_whole_file(char *download_address, test_result proxy_with_info, int *work_proxy_count, char *filename,
                         file_bytes total_length, small_info *thread_report_info_list, file_bytes *checkpoint) {
    while (*checkpoint != total_length) {
        // going to download block of file size list.
        file_bytes *block_size_list_thread = calculate_block_size_to_download_one_thread(proxy_with_info.speed_list,
                                                                                         *work_proxy_count);
        file_bytes trunkResult = trunk_to_file_size(block_size_list_thread, work_proxy_count, *checkpoint,
                                                    total_length);
        proxy_with_info.speed_list = blocked_multi_download(download_address,
                                                            proxy_with_info.proxy_list, filename, *work_proxy_count,
                                                            block_size_list_thread, *checkpoint,
                                                            thread_report_info_list);
        *checkpoint = trunkResult;
    }
}

