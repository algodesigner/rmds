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
} Options;

void print_usage(const char *progname)
{
    printf("Usage: %s [options] [start_path]\n", progname);
    printf("\nOptions:\n");
    printf("  -n, --dry-run      Show what would be deleted without actually "
           "deleting\n");
    printf("  -q, --quiet        Suppress all output except errors\n");
    printf("  -v, --verbose      Display directories as they are scanned\n");
    printf("  -i, --interactive  Prompt for confirmation before deleting each "
           "file\n");
    printf("  -h, --help         Display this help menu\n");
    printf("\nArguments:\n");
    printf("  start_path         The directory to start scanning (defaults to "
           "$HOME)\n");
}

// Recursively deletes all `.DS_Store` files in the specified directory,
// including any subdirectories.
void remove_dsstore(const char *path, const Options *opts)
{
    struct dirent *entry;
    DIR *dir = opendir(path);
    if (!dir) {
        if (!opts->quiet) {
            perror("Error opening directory");
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
        if (lstat(fullpath, &statbuf) == -1)
            continue;

        if (S_ISDIR(statbuf.st_mode)) {
            // Recurse into directory
            remove_dsstore(fullpath, opts);
        } else if (strcmp(entry->d_name, ".DS_Store") == 0) {
            bool should_delete = true;

            if (opts->interactive) {
                printf("Delete %s? (y/N): ", fullpath);
                char response = getchar();
                if (response != 'y' && response != 'Y') {
                    should_delete = false;
                }
                // Clear input buffer
                if (response != '\n' && response != EOF) {
                    while (getchar() != '\n')
                        ;
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
                        perror("Error deleting file");
                    }
                }
            }
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[])
{
    Options opts = {false, false, false, false};

    static struct option long_options[] = {{"dry-run", no_argument, 0, 'n'},
            {"quiet", no_argument, 0, 'q'}, {"verbose", no_argument, 0, 'v'},
            {"interactive", no_argument, 0, 'i'}, {"help", no_argument, 0, 'h'},
            {0, 0, 0, 0}};

    int opt;
    while ((opt = getopt_long(argc, argv, "nqvih", long_options, NULL)) != -1) {
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

    if (!opts.quiet) {
        printf("Scanning for .DS_Store files in: %s\n", start_path);
        if (opts.dry_run) {
            printf("Running in Dry Run mode. No files will be deleted.\n");
        }
    }

    remove_dsstore(start_path, &opts);

    return 0;
}
