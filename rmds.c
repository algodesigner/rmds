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
    char **excludes;
    int exclude_count;
    const char *target_name;
    bool clean_all;
} Options;

void print_usage(const char *progname)
{
    printf("Usage: %s [options] [path1] [path2] ...\n", progname);
    printf("\nOptions:\n");
    printf("  -A, --clean-all        Remove both .DS_Store and ._* "
           "(AppleDouble) files\n");
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
    printf("  -e, --exclude <DIR>    Exclude directory name from scan (can be "
           "used multiple times)\n");
    printf("  -m, --name <NAME>      Target filename to delete (defaults to "
           ".DS_Store)\n");
    printf("  -h, --help             Display this help menu\n");
    printf("\nArguments:\n");
    printf("  paths                  One or more directories to scan (defaults "
           "to $HOME)\n");
}

bool is_excluded(const char *name, const Options *opts)
{
    for (int i = 0; i < opts->exclude_count; i++) {
        if (strcmp(name, opts->excludes[i]) == 0) {
            return true;
        }
    }
    return false;
}

bool is_target(const char *name, const Options *opts)
{
    if (opts->clean_all) {
        return (strcmp(name, ".DS_Store") == 0 || strncmp(name, "._", 2) == 0);
    }
    return (strcmp(name, opts->target_name) == 0);
}

// Recursively deletes target files in the specified directory,
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
            // macOS often returns EPERM for protected Library folders (TCC)
            // EACCES is standard permission denied.
            if (errno == EACCES || errno == EPERM) {
                if (opts->verbose) {
                    printf("Skipping (Access Denied): %s\n", path);
                }
            } else {
                fprintf(stderr, "Error opening directory '%s': %s\n", path,
                        strerror(errno));
            }
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
            // Check exclusion
            if (is_excluded(entry->d_name, opts)) {
                if (opts->verbose && !opts->quiet) {
                    printf("Skipping (excluded): %s\n", fullpath);
                }
                continue;
            }

            // Check filesystem boundary
            if (opts->one_file_system && statbuf.st_dev != opts->root_dev) {
                if (opts->verbose && !opts->quiet) {
                    printf("Skipping (different filesystem): %s\n", fullpath);
                }
                continue;
            }

            // Recurse into directory
            remove_dsstore(fullpath, opts, current_depth + 1);
        } else if (is_target(entry->d_name, opts)) {
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
            .root_dev = 0,
            .excludes = NULL,
            .exclude_count = 0,
            .target_name = ".DS_Store",
            .clean_all = false};

    static struct option long_options[] = {{"clean-all", no_argument, 0, 'A'},
            {"dry-run", no_argument, 0, 'n'}, {"quiet", no_argument, 0, 'q'},
            {"verbose", no_argument, 0, 'v'},
            {"interactive", no_argument, 0, 'i'},
            {"max-depth", required_argument, 0, 'd'},
            {"one-file-system", no_argument, 0, 'x'},
            {"exclude", required_argument, 0, 'e'},
            {"name", required_argument, 0, 'm'}, {"help", no_argument, 0, 'h'},
            {0, 0, 0, 0}};

    int opt;
    while ((opt = getopt_long(
                    argc, argv, "Anqvihd:xe:m:", long_options, NULL)) != -1) {
        switch (opt) {
        case 'A':
            opts.clean_all = true;
            break;
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
        case 'e':
            opts.excludes = realloc(
                    opts.excludes, sizeof(char *) * (opts.exclude_count + 1));
            if (opts.excludes == NULL) {
                fprintf(stderr, "Memory allocation failed for excludes.\n");
                return 1;
            }
            opts.excludes[opts.exclude_count++] = optarg;
            break;
        case 'm':
            opts.target_name = optarg;
            break;
        case 'h':
            print_usage(argv[0]);
            return 0;
        default:
            print_usage(argv[0]);
            return 1;
        }
    }

    if (optind >= argc) {
        // Default to HOME if no paths provided
        const char *home = getenv("HOME");
        if (!home) {
            fprintf(stderr, "Could not determine starting path ($HOME).\n");
            return 1;
        }

        struct stat root_stat;
        if (stat(home, &root_stat) == -1) {
            fprintf(stderr, "Error stating starting path '%s': %s\n", home,
                    strerror(errno));
            return 1;
        }
        opts.root_dev = root_stat.st_dev;

        if (!opts.quiet) {
            if (opts.clean_all) {
                printf("Cleaning all metadata (.DS_Store and ._*) in: %s\n",
                        home);
            } else {
                printf("Scanning for %s files in: %s\n", opts.target_name,
                        home);
            }
        }
        remove_dsstore(home, &opts, 0);
    } else {
        // Process all provided paths
        for (int i = optind; i < argc; i++) {
            const char *path = argv[i];
            struct stat root_stat;
            if (stat(path, &root_stat) == -1) {
                fprintf(stderr, "Error stating path '%s': %s\n", path,
                        strerror(errno));
                continue;
            }
            opts.root_dev = root_stat.st_dev;

            if (!opts.quiet) {
                if (opts.clean_all) {
                    printf("Cleaning all metadata (.DS_Store and ._*) in: %s\n",
                            path);
                } else {
                    printf("Scanning for %s files in: %s\n", opts.target_name,
                            path);
                }
            }
            remove_dsstore(path, &opts, 0);
        }
    }

    free(opts.excludes);
    return 0;
}
