#include <stdio.h>
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>
#include <unistd.h>

typedef unsigned long long file_bytes;
#define LOWEST_SPEED_BPS 50000

#include "progress.c"
#include "requester.c"
#include "request_speed.c"
#include "thread_control.c"
#include "parser.c"
#include "download_control.c"
#include "file_control.c"

//#include "error_control.c"

/* command line usage
 * nbget https://www.example.com http://localhost:1090 socks5://localhost:1080 good.txt
 * the download will be separated into 2 thread, with corresponding proxy server address with protocol.
 * */
int main(int argc, char **argv) {
    curl_global_init(CURL_GLOBAL_ALL);
    commandline_args args = parse_args(argc, argv);
    file_bytes size = get_file_size(args.download_address);
    struct test_result testResult = test_proxy_list(args.download_address, args.proxy_list, args.proxy_count);
    create_file(args.file_name, size);
    // these two vars will pass to another thread to monitor global download status.
    file_bytes last_big_block_checkpoint = 0;
    small_info *thread_report_info_list = make_info_list(testResult.proxyList.proxy_count);
    create_progress_thread(thread_report_info_list, testResult.proxyList.proxy_count, &last_big_block_checkpoint, size);
    download_whole_file(args.download_address, testResult, args.file_name, size, thread_report_info_list,&last_big_block_checkpoint);
    curl_global_cleanup();
}