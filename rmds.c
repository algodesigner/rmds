/*
 * rmds.c - A utility to recursively remove .DS_Store files
 * Copyright (c) 2026, Vlad Shurupov. All rights reserved.
 *
 * Licensed under the 3-Clause BSD License.
 * See the LICENSE file in the project root for full license text.
 *
 * This program scans a specified directory (or the user's home directory by
 * default) and recursively deletes all .DS_Store files found within it. These
 * files are commonly created by macOS to store folder metadata but can clutter
 * directories, especially in shared or version-controlled environments.
 *
 * Features:
 * - Recursively scans directories for .DS_Store files.
 * - Deletes identified .DS_Store files and logs the action.
 * - Provides error messages for files that cannot be deleted.
 * - Command-line flags for dry-run, quiet, verbose, and interactive modes.
 */

#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct {
    bool dry_run;
    bool quiet;
    bool verbose;
    bool interactive;
    int max_depth;
    bool one_file_system;
    dev_t root_dev;
} Options;

void print_usage(const char *progname)
{
    printf("Usage: %s [options] [start_path]\n", progname);
    printf("\nOptions:\n");
    printf("  -n, --dry-run          Show what would be deleted without "
           "actually deleting\n");
    printf("  -q, --quiet            Suppress all output except errors\n");
    printf("  -v, --verbose          Display directories as they are "
           "scanned\n");
    printf("  -i, --interactive      Prompt for confirmation before deleting "
           "each file\n");
    printf("  -d, --max-depth <N>    Only scan directories at most N levels "
           "deep\n");
    printf("  -x, --one-file-system  Do not traverse directories on different "
           "filesystems\n");
    printf("  -h, --help             Display this help menu\n");
    printf("\nArguments:\n");
    printf("  start_path             The directory to start scanning (defaults "
           "to $HOME)\n");
}

// Recursively deletes all `.DS_Store` files in the specified directory,
// including any subdirectories.
void remove_dsstore(const char *path, const Options *opts, int current_depth)
{
    // Check depth limit
    if (opts->max_depth != -1 && current_depth > opts->max_depth) {
        return;
    }

    struct dirent *entry;
    DIR *dir = opendir(path);
    if (!dir) {
        if (!opts->quiet) {
            fprintf(stderr, "Error opening directory '%s': %s\n", path,
                    strerror(errno));
        }
        return;
    }

    if (opts->verbose && !opts->quiet) {
        printf("Scanning: %s\n", path);
    }

    char fullpath[4096];

    while ((entry = readdir(dir)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        struct stat statbuf;
        if (lstat(fullpath, &statbuf) == -1) {
            if (!opts->quiet) {
                fprintf(stderr, "Error stating '%s': %s\n", fullpath,
                        strerror(errno));
            }
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            // Check filesystem boundary
            if (opts->one_file_system && statbuf.st_dev != opts->root_dev) {
                if (opts->verbose && !opts->quiet) {
                    printf("Skipping (different filesystem): %s\n", fullpath);
                }
                continue;
            }

            // Recurse into directory
            remove_dsstore(fullpath, opts, current_depth + 1);
        } else if (strcmp(entry->d_name, ".DS_Store") == 0) {
            bool should_delete = true;

            if (opts->interactive) {
                printf("Delete %s? (y/N): ", fullpath);
                char response = getchar();
                // Clear input buffer
                if (response != '\n' && response != EOF) {
                    int c;
                    while ((c = getchar()) != '\n' && c != EOF)
                        ;
                }
                if (response != 'y' && response != 'Y') {
                    should_delete = false;
                }
            }

            if (should_delete) {
                if (opts->dry_run) {
                    if (!opts->quiet) {
                        printf("(dry-run) Would delete: %s\n", fullpath);
                    }
                } else {
                    if (unlink(fullpath) == 0) {
                        if (!opts->quiet) {
                            printf("Deleted: %s\n", fullpath);
                        }
                    } else {
                        fprintf(stderr, "Error deleting '%s': %s\n", fullpath,
                                strerror(errno));
                    }
                }
            }
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[])
{
    Options opts = {.dry_run = false,
            .quiet = false,
            .verbose = false,
            .interactive = false,
            .max_depth = -1,
            .one_file_system = false,
            .root_dev = 0};

    static struct option long_options[] = {{"dry-run", no_argument, 0, 'n'},
            {"quiet", no_argument, 0, 'q'}, {"verbose", no_argument, 0, 'v'},
            {"interactive", no_argument, 0, 'i'},
            {"max-depth", required_argument, 0, 'd'},
            {"one-file-system", no_argument, 0, 'x'},
            {"help", no_argument, 0, 'h'}, {0, 0, 0, 0}};

    int opt;
    while ((opt = getopt_long(argc, argv, "nqvihd:x", long_options, NULL)) !=
            -1) {
        switch (opt) {
        case 'n':
            opts.dry_run = true;
            break;
        case 'q':
            opts.quiet = true;
            break;
        case 'v':
            opts.verbose = true;
            break;
        case 'i':
            opts.interactive = true;
            break;
        case 'd':
            opts.max_depth = atoi(optarg);
            break;
        case 'x':
            opts.one_file_system = true;
            break;
        case 'h':
            print_usage(argv[0]);
            return 0;
        default:
            print_usage(argv[0]);
            return 1;
        }
    }

    const char *start_path = (optind < argc) ? argv[optind] : getenv("HOME");
    if (!start_path) {
        fprintf(stderr, "Could not determine starting path.\n");
        return 1;
    }

    struct stat root_stat;
    if (stat(start_path, &root_stat) == -1) {
        fprintf(stderr, "Error stating starting path '%s': %s\n", start_path,
                strerror(errno));
        return 1;
    }
    opts.root_dev = root_stat.st_dev;

    if (!opts.quiet) {
        printf("Scanning for .DS_Store files in: %s\n", start_path);
        if (opts.dry_run) {
            printf("Running in Dry Run mode. No files will be deleted.\n");
        }
    }

    remove_dsstore(start_path, &opts, 0);

    return 0;
}
