#define METER_INTERVAL_SEC 1

typedef struct {
    file_bytes already_download;
    curl_off_t current_speed;
} small_info;

struct args_progress {
    small_info *info_list;
    int count;
    file_bytes *checkpoint;
    file_bytes total_file_size;
};

static const char *humanSize(curl_off_t bytes) {
    char *suffix[] = {"B", "KB", "MB", "GB", "TB"};
    char length = sizeof(suffix) / sizeof(suffix[0]);

    int i = 0;
    double dblBytes = bytes;

    if (bytes > 1024) {
        for (i = 0; (bytes / 1024) > 0 && i < length - 1; i++, bytes /= 1024)
            dblBytes = bytes / 1024.0;
    }

    static char output[200];
    sprintf(output, "%.02lf %s", dblBytes, suffix[i]);
    return output;
}

// total progress (global already download bytes) equals to
void *show_progress(void *args_raw) {
    struct args_progress *args = (struct args_progress *) args_raw;
    file_bytes last_progress_total_download_value = 0;
    while (*(args->checkpoint) != args->total_file_size) {
        curl_off_t total_speed = 0;
        file_bytes total_progress = *(args->checkpoint);
        for (int i = 0; i < args->count; ++i) {
//            total_speed += args->info_list[i].current_speed;
            total_progress += args->info_list[i].already_download;
        }
        total_speed = (curl_off_t) ((total_progress - last_progress_total_download_value) / METER_INTERVAL_SEC);
        fprintf(stderr, "\rspeed:%s/s, progress: %.*f%%", humanSize(total_speed), 1,
                (float) total_progress / (float) args->total_file_size * 100);
        sleep(METER_INTERVAL_SEC);
        last_progress_total_download_value = (curl_off_t) total_progress;
    }
    return NULL;
}

small_info *make_info_list(int size) {
    small_info *info_list = malloc(sizeof(small_info) * size);
    for (int i = 0; i < size; ++i) {
        info_list[i].already_download = 0;
        info_list[i].current_speed = 0;
    }
    return info_list;
}

pthread_t *
create_progress_thread(small_info *info_list, int count, file_bytes *checkpoint, file_bytes total_file_size) {
    struct args_progress *argsProgress = malloc(sizeof(argsProgress));
    pthread_t *progress_thread = malloc(
            sizeof(pthread_t));// this is a memory block which is created and never recycle ><.
    struct args_progress argsProgress_temp = {info_list, count, checkpoint, total_file_size};
    *argsProgress = argsProgress_temp;
    pthread_create(progress_thread, NULL, show_progress, (void *) argsProgress);
    return progress_thread;
}