#include "requester.c"
#include "thread_control.c"
#include "parser.c"
#include "download_control.c"
#include "file_control.c"
//#include "progress.c"
//#include "error_control.c"

/* command line usage
 * nbget https://www.example.com http://localhost:1090 socks5://localhost:1080 good.txt
 * the download will be separated into 2 thread, with corresponding proxy server address with protocol.
 * */
int main(int argc, char **argv) {
    curl_global_init(CURL_GLOBAL_ALL);
    commandline_args args = parse_args(argc, argv);
    file_bytes size = get_file_size(args.download_address);
    proxy_list proxyList = {args.proxy_list, args.proxy_count};
    create_file(args.file_name, size);
    struct test_result testResult = test_proxy_server(proxyList, args.download_address, args.file_name);
    download_whole_file(args.download_address, testResult, args.file_name, size);
    curl_global_cleanup();
}