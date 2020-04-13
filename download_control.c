#define initial_download_block_size 0x400 // download 1MB(for each thread) for test usage.
#define maximum_initial_test_time 5 // in second, max wait time for download
#define expect_download_time_per_thread 30 // in sec. new thread will be expected to download for this time.



void download_whole_file(char *download_address, char **proxy_list, char *filename, int proxy_count, file_bytes total_length){
    file_bytes * thread_block_size_list = malloc(proxy_count * sizeof(file_bytes));
    for(int i=0;i<proxy_count;i++){
        thread_block_size_list[i] = initial_download_block_size;
    }
    file_bytes current_download_block_position = 0;
    // first time arrange size. IF YOU CAN DOWNLOAD, DOWNLOAD MORE, NO BB.
    struct download_speed_list current_block_download_speed_list =
            thread_arrange(download_address, proxy_list, filename, proxy_count,thread_block_size_list,0);
    for(int i=0;i<proxy_count;i++){
        thread_block_size_list[i] = current_block_download_speed_list.speed_list[i] * expect_download_time_per_thread;
    }
    short download_finished = 0;
    // small init download is only for test use.
    while(!download_finished){
        file_bytes total_trunk_size_this_loop = 0;
        for(int j=0;j<proxy_count;j++){
            if(thread_block_size_list[j] > 0){
                total_trunk_size_this_loop += thread_block_size_list[j];
                if(total_trunk_size_this_loop > total_length){
                    // download will finish in this loop.
                    thread_block_size_list[j] = total_length - total_trunk_size_this_loop;
                    for (j++;j<proxy_count;j++) {
                        thread_block_size_list[j] = 0;
                    }
                    download_finished = 1;
                    break;
                }
            }
        }
        // start wait block to download, this is a block function
        download_speed_list speedList = thread_arrange(download_address, proxy_list, filename, proxy_count, thread_block_size_list,current_download_block_position);
        for(int i=0;i<speedList.list_length;i++){
            thread_block_size_list[i] = speedList.speed_list[i] * expect_download_time_per_thread;
        }
    }
}