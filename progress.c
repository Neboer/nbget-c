typedef struct {
    file_bytes already_download;
    curl_off_t current_speed;
} small_info;

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
void show_progress(small_info *info_list, int count, file_bytes checkpoint, file_bytes total_size) {
    curl_off_t total_speed = 0;
    file_bytes total_progress = checkpoint;
    for (int i = 0; i < count; ++i) {
        total_speed += info_list[i].current_speed;
        total_progress += info_list[i].already_download;
    }
    fprintf(stderr, "\rspeed:%s, progress: %.*f%%", humanSize(total_speed), 1,
            (float) total_progress / (float) total_size * 100);
}

small_info *make_info_list(int size) {
    small_info *info_list = malloc(sizeof(small_info) * size);
    return info_list;
}