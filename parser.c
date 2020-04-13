typedef struct arguments {
    char *download_address;
    int proxy_count;
    char **proxy_list;
    char *file_name;
} commandline_args;

commandline_args parse_args(int argc, char **argv) {
    if (argc < 4) {
        printf("usage: nbget http://example.com/file http://proxy1:1234 socks5://proxy2:1122 output.file");
        exit(0);
    }
    commandline_args args;
    args.download_address = argv[1];
    args.proxy_list = argv + 2;
    args.proxy_count = argc - 2;
    args.file_name = argv[argc-1];
    return args;
}