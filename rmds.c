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
 *
 * Usage:
 * - Compile: gcc -o rmds rmds.c
 * - Run: ./rmds [start_path]
 *   - [start_path]: Optional. The directory to start scanning. Defaults to
 * $HOME.
 *
 * Notes:
 * - Ensure you have the necessary permissions to delete files in the target
 * directories.
 * - Use with caution, as deleted files cannot be recovered.
 */

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// Recursively deletes all `.DS_Store` files in the specified directory,
// including any subdirectories.
void remove_dsstore(const char *path)
{
    struct dirent *entry;
    DIR *dir = opendir(path);
    if (!dir)
        return;

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
            remove_dsstore(fullpath);
        } else if (strcmp(entry->d_name, ".DS_Store") == 0) {
            if (unlink(fullpath) == 0) {
                printf("Deleted: %s\n", fullpath);
            } else {
                perror("Error deleting file");
            }
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[])
{
    const char *start_path = (argc > 1) ? argv[1] : getenv("HOME");
    if (!start_path) {
        fprintf(stderr, "Could not determine starting path.\n");
        return 1;
    }

    printf("Scanning for .DS_Store files in: %s\n", start_path);
    remove_dsstore(start_path);

    return 0;
}
