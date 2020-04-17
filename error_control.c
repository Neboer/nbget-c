typedef struct {
    range* error_storage_list_addr[];
    int
};

range** make_error_storage_place(int init_proxy_count){

    range* error_block_list = malloc(sizeof(range) * init_proxy_count);
    return
}

range*