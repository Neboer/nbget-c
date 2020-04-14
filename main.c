#include "requester.c"
#include "thread_control.c"
#include "parser.c"
#include "download_control.c"
#include "file_control.c"

/* command line usage
 * nbget https://www.example.com http://localhost:1090 socks5://localhost:1080 good.txt
 * the download will be separated into 2 thread, with corresponding proxy server address with protocol.
 * */
int main(int argc, char** argv) {
    curl_global_init(CURL_GLOBAL_ALL);
    commandline_args args = parse_args(argc, argv);
    file_bytes size = get_file_size(args.download_address);
    create_file(args.file_name, size);
    download_whole_file(args.download_address, args.proxy_list, args.file_name,args.proxy_count, size);
    curl_global_cleanup();
}