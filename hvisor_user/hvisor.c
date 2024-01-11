#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <signal.h>

#include "hvisor.h"
#include "virtio.h"
#include "log.h"
int hvisor_init();
// void hvisor_sig_handler(int n, siginfo_t *info, void *unused);
void handle_virtio_requests();

struct hvisor_device_region *device_region;
int fd;

int main(int argc, char *argv[])
{
    hvisor_init();
}

int hvisor_init()
{
    int err;
    log_set_level(LOG_ERROR);
    FILE *log_file = fopen("log.txt", "w+");
    if (log_file == NULL) {
        log_error("open log file failed");
    }
    log_add_fp(log_file, 0);
    log_info("hvisor init");
    fd = open("/dev/hvisor", O_RDWR);
    if (fd < 0) {
        log_error("open hvisor failed");
        exit(1);
    }
    // ioctl for init virtio
    err = ioctl(fd, HVISOR_INIT_VIRTIO);
    if (err) {
        log_error("ioctl failed, err code is %d", err);
        close(fd);
        exit(1);
    }

    // mmap: create shared memory
    device_region = (struct hvisor_device_region *) mmap(NULL, MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (device_region == (void *)-1) {
        log_error("mmap failed");
        goto unmap;
    }

    init_virtio_devices();
    log_info("hvisor init okay!");
    handle_virtio_requests();

unmap:
    munmap(device_region, MMAP_SIZE);
    return 0;
}

void handle_virtio_requests()
{
    unsigned int last_req_idx = device_region->last_req_idx;
    struct device_req *req;
    while (1) {
        if (last_req_idx < device_region->idx) {
            req = &device_region->req_list[last_req_idx % MAX_REQ];
            virtio_handle_req(req);
            last_req_idx++;
            device_region->last_req_idx = last_req_idx;
            ioctl(fd, HVISOR_FINISH);
        }
    }
}