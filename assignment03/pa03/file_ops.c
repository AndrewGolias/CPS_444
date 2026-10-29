#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "file_ops.h"
int file_info(const char *path)
{
    struct stat metadata;

    // Get file metadata
    if (stat(path, &metadata) == -1) {
        perror(path);
        return 1;
    }

    // Check the file type
    const char *type = S_ISREG(metadata.st_mode) ? "regular" :
                       S_ISDIR(metadata.st_mode) ? "directory" :
                       "other";

    // Display file information
    printf("path=%s\n", path);
    printf("type=%s\n", type);
    printf("size=%" PRIdMAX "\n", (intmax_t)metadata.st_size);
    printf("mode=0%03o\n", (unsigned int)(metadata.st_mode & 0777));

    return 0;
}

int file_copy(const char *source, const char *dest)
{
    int in_fd;
    int out_fd;
    int status = 0;
    int close_in;
    int close_out;
    char buffer[4096];
    ssize_t nread;

    // Open the source before creating the destination
    if (source[0] == '-' && source[1] == '\0') {
        in_fd = STDIN_FILENO;
    } else {
        in_fd = open(source, O_RDONLY);
        if (in_fd == -1) {
            perror(source);
            return 1;
        }
    }

    // Avoid overwriting an existing destination
    if (dest[0] == '-' && dest[1] == '\0') {
        out_fd = STDOUT_FILENO;
    } else {
        out_fd = open(dest, O_WRONLY | O_CREAT | O_EXCL, 0644);
        if (out_fd == -1) {
            perror(dest);

            if (in_fd != STDIN_FILENO && close(in_fd) == -1) {
                perror("close source");
            }

            return 1;
        }
    }

    // Read until EOF, writing every byte read
    while ((nread = read(in_fd, buffer, sizeof buffer)) > 0) {
        ssize_t written = 0;

        while (written < nread) {
            ssize_t nwritten = write(
                out_fd,
                buffer + written,
                (size_t)(nread - written)
            );

            if (nwritten == -1) {
                perror("write");
                status = 1;
                break;
            }

            if (nwritten == 0) {
                fprintf(stderr, "write: no bytes written\n");
                status = 1;
                break;
            }

            written += nwritten;
        }

        if (status != 0) {
            break;
        }
    }

    if (nread == -1) {
        perror("read");
        status = 1;
    }

    // Close only descriptors opened by this function
    if (in_fd != STDIN_FILENO) {
        close_in = close(in_fd);
        if (close_in == -1) {
            perror("close source");
            status = 1;
        }
    }

    if (out_fd != STDOUT_FILENO) {
        close_out = close(out_fd);
        if (close_out == -1) {
            perror("close destination");
            status = 1;
        }
    }

    return status;
}