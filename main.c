#include "requester.c"
#include "thread_control.c"
#include "parser.c"
#include "download_control.c"
#include "file_control.c"

/* command line usage
 * nbget https://www.example.com http://localhost:1090 socks5://localhost:1080
 * the download will be separated into 2 thread, with corresponding proxy server address with protocol.
 * */
int main(int argc, char** argv) {
    commandline_args args = parse_args(argc, argv);
    
}