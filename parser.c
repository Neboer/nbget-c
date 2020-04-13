typedef struct arguments {
    char *download_address;
    int proxy_count;
    char **proxy_list;
} commandline_args;

commandline_args parse_args(int argc, char **argv) {
    commandline_args args;
    args.download_address = argv[0];
    args.proxy_list = argv + 1;
    args.proxy_count = argc - 1;
    return args;
}