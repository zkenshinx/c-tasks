#include <stdio.h>
#include <unistd.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

void ls_prefix_tab_print(const char* dir_path, int depth);
void ls(const char* dir_path);
void print_tabs(int count);
void print_error_and_exit(char* error_message);

int main() {
    char cwd[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)) == 0) {
        print_error_and_exit("Couldn't get cwd: getcwd()");
    }

    ls(cwd);
    
    return EXIT_SUCCESS;
}

void print_tabs(int count) {
    for (int i = 0; i < count; ++i) {
        printf("%c", '\t');
    }
}

void ls_prefix_tab_print(const char* dir_path, int depth) {
    char* absolute_path = malloc(PATH_MAX);
    
    // Better to be safe
    if (absolute_path == NULL) {
        print_error_and_exit("Out of heap memory");
    }

    strcpy(absolute_path, dir_path);
    DIR* dir = opendir(dir_path);
    struct dirent* dirent;
    while ((dirent = readdir(dir)) != NULL) {
        // ls by default doesn't print hidden files and ".", ".."
        if (dirent->d_name[0] == '.') {
            continue;
        }
        
        // In any case whether it's directory or any other type
        // We print it's name
        print_tabs(depth);
        printf("%s\n", dirent->d_name);

        // For directories let's recurse
        if (dirent->d_type == DT_DIR) {
            // Build the absolute path the directory we want to recurse into
            int dir_path_len = strlen(dir_path);
            absolute_path[dir_path_len] = '/';
            strcpy(absolute_path + dir_path_len + 1, dirent->d_name);

            ls_prefix_tab_print(absolute_path, depth + 1);
        } 
    }
    
    // Free used resources
    free(absolute_path);
    closedir(dir);
}

void ls(const char* dir_path) {
    // In the task statement it seems that the cwd absolute path is printed first
    printf("%s\n", dir_path);
    ls_prefix_tab_print(dir_path, 1);
}

void print_error_and_exit(char* error_message) {
    fprintf(stderr, "%s\n", error_message);
    exit(EXIT_FAILURE);
}