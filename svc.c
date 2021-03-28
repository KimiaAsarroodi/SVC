#include "svc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


/*
 * Function : svc_init
 * ------------------------------------
 * Allocating memory for system and branch list and initializing attributes of first branch ("master") 
 *
 * This function is being used as starting point of rest of the functions 
 *
 * returns  pointer of type struct system
 */
void* svc_init(void) {

    struct system* system = malloc(sizeof(struct system));
    system->branch_list = malloc(sizeof(struct branch));

    //Initializing branch "master" attributes
    system->branch_list[0].name = strdup("master");
    system->branch_list[0].head = 1;
    system->branch_list[0].commits = NULL;
    system->branch_list[0].n_commits = 0;
    system->branch_list[0].files = NULL;
    system->branch_list[0].n_files = 0;
    system->branch_list[0].staging = NULL;
    system->branch_list[0].n_staging = 0;
    system->branch_num = 1;

    //Setting "master" branch as active branch
    system->active_branch = &(system->branch_list[0]);

    return system;
}
/*
 * Function : cleanup
 * ------------------------------------
 * Deallocates all memory which has been allocated dynamically by using:
 *
 * helper : Pointer of system struct
 *
 */
void cleanup(void* helper) {

    struct system* ptr = helper;
    for (int i = 0; i < ptr->branch_num; i++) {
        //Checking nullity of commits array to deallocate memory associated with 
        if (ptr->branch_list[i].commits != NULL) {
            for (int m = 0; m < ptr->branch_list[i].n_commits; m++) {
                for (int n = 0; n < ptr->branch_list[i].commits[m].n_cfiles; n++) {
                    free(ptr->branch_list[i].commits[m].commit_files[n].file_name);
                }
                for (int l = 0; l < ptr->branch_list[i].commits[m].n_tracked; l++) {
                    free(ptr->branch_list[i].commits[m].tracked_files[l].file_name);
                }
                free(ptr->branch_list[i].commits[m].msg);
                free(ptr->branch_list[i].commits[m].id);
                free(ptr->branch_list[i].commits[m].commit_files);
                free(ptr->branch_list[i].commits[m].tracked_files);
            }
        }
        //Checking nullity of staging array to deallocate memory associated with
        if (ptr->branch_list[i].staging != NULL) {
            for (int k = 0; k < ptr->branch_list[i].n_staging; k++) {
                free(ptr->branch_list[i].staging[k].file_name);
            }
        }
        //Checking nullity of files array to deallocate memory associated with
        if (ptr->branch_list[i].files != NULL) {
            for (int j = 0; j < ptr->branch_list[i].n_files; j++) {
                free(ptr->branch_list[i].files[j].file_name);
            }
        }
        free(ptr->branch_list[i].files);
        free(ptr->branch_list[i].commits);
        free(ptr->branch_list[i].staging);
        free(ptr->branch_list[i].name);
    }
    free(ptr->branch_list);
    free(ptr);
}
/*
 * Function : hash_file
 * ------------------------------------
 * Calculates hash value of a file using
 *
 *helper : Pointer of system struct
 *file_path : Path of a file
 *
 * returns  hash value of a file
 */
int hash_file(void* helper, char* file_path) {
    //Checking nulity of file_path
    if (file_path == NULL) {
        return -1;
    }
    //Checking existance of file
    FILE* file_pointer = fopen(file_path, "r");
    if (file_pointer == NULL) {
        return -2;
    }

    int path_len = strlen(file_path);
    int hash = 0;
    //Traversing through characters of file_path to calculate hash value
    for (int i = 0; i < path_len; i++) {
        hash = (hash + file_path[i]) % 1000;
    }
    //Traversing through characters of content of given file_path to update hash value
    int c = fgetc(file_pointer);
    while (c != EOF) {

        hash = (hash + c) % 2000000000;
        c = fgetc(file_pointer);
    }
    fclose(file_pointer);

    return hash;
}
/*
 * Function : convert_name
 * ------------------------------------
 * Converts a string known as file_name to lowercase of the name
 *
 * returns  lowercase name
 */
char* convert_name(char* file_name) {

    char* name = strdup(file_name);
    //Converts every character in file_name to lowercase
    for (int i = 0; i < strlen(file_name); i++) {
        name[i] = tolower(name[i]);
    }

    return name;
}
/*
 * Function : sort_staging
 * ------------------------------------
 * Sorts name of files in staging area based on alphabetical order using:
 *
 * helper : Pointer of system struct
 *
 * returns  lowercase name
 */
void sort_staging(void* helper) {

    struct system* ptr = helper;
    char** array = malloc(sizeof(char*) * ptr->active_branch->n_staging);
    //Storing lowercase name of files existing in "staging", to a temporary array
    for (int i = 0; i < ptr->active_branch->n_staging; i++) {
        array[i] = (convert_name(ptr->active_branch->staging[i].file_name));
    }

    struct file temp_file;
    //Traversing through "staging" array and swapping struct objects of files based on index of files in "array"
    for (int i = 0; i < ptr->active_branch->n_staging; i++) {
        for (int j = i + 1; j < ptr->active_branch->n_staging; j++) {
            if (strcmp(array[i], array[j]) > 0) {

                temp_file = ptr->active_branch->staging[i];
                ptr->active_branch->staging[i] = ptr->active_branch->staging[j];
                ptr->active_branch->staging[j] = temp_file;
            }
        }
    }
    //Deallocating memory which was assigned for array and its elements
    for (int i = 0; i < ptr->active_branch->n_staging; i++) {
        free(array[i]);
    }
    free(array);
}
/*
 * Function : svc_commit
 * ------------------------------------
 * Creates commit and generates commit id using:
 *
 * helper : Pointer of system struct
 * message : Message of commit
 *
 * returns  commit id in hexadecimal format
 */
char* svc_commit(void* helper, char* message) {
    //Checking nullity of "message"
    if (message == NULL) {
        return NULL;
    }
    if (strcmp(message, "No changes") == 0) {
        return NULL;
    }
    int id = 0;
    char* hex = malloc(sizeof(char) * 7);
    int msg_len = strlen(message);

    //Calculating "id" by traversing characters of string "message"
    for (int i = 0; i < msg_len; i++) {
        id = (id + message[i]) % 1000;
    }
    struct system* ptr = helper;
    int check_file = 0;
    int inx_file = 0;
    int check_staging = 0;
    int inx_staging = 0;

    //Checking for deleted files in array "files" which records tracked files of system
    for (int i = 0; i < ptr->active_branch->n_files; i++) {
        FILE* file_pointer = fopen(ptr->active_branch->files[i].file_name, "r");
        if (file_pointer == NULL) {
            check_file = 1;
            inx_file = i;
            break;
        }
    }
    //When a file which was being tracked by system has been deleted manually 
    if (check_file == 1) {
        for (int j = 0; j < ptr->active_branch->n_staging; j++) {

            if (strcmp(ptr->active_branch->files[inx_file].file_name, ptr->active_branch->staging[j].file_name) == 0
                && ptr->active_branch->staging[j].change == '+') {

                check_staging = 1;
                inx_staging = j;
                break;
            }
        }
        //When deleted file exists in "staging" array as recently added file
        if (check_staging == 1) {
            //Removing file from "staging" array 
            free(ptr->active_branch->staging[inx_staging].file_name);
            ptr->active_branch->n_staging--;
            for (int i = inx_staging; i < ptr->active_branch->n_staging; i++) {
                ptr->active_branch->staging[i] = ptr->active_branch->staging[i + 1];
                ptr->active_branch->staging = realloc(ptr->active_branch->staging, sizeof(struct file) * ptr->active_branch->n_staging);
            }

        }
        //When file is not in "staging" array 
        if (check_staging == 0) {
            //Adding file to "staging: to be committed 
            ptr->active_branch->n_staging++;
            ptr->active_branch->staging = realloc(ptr->active_branch->staging, sizeof(struct file) * ptr->active_branch->n_staging);
            ptr->active_branch->staging[ptr->active_branch->n_staging - 1].file_name = strdup(ptr->active_branch->files[inx_file].file_name);
            ptr->active_branch->staging[ptr->active_branch->n_staging - 1].hash = ptr->active_branch->files[inx_file].hash;
            ptr->active_branch->staging[ptr->active_branch->n_staging - 1].change = '-';
        }
        //Removing deleted file from "files" array 
        free(ptr->active_branch->files[inx_file].file_name);
        ptr->active_branch->n_files--;
        for (int m = inx_file; m < ptr->active_branch->n_files; m++) {
            ptr->active_branch->files[m] = ptr->active_branch->files[m + 1];
        }
    }

    check_file = 0;
    inx_file = 0;
    check_staging = 0;
    inx_staging = 0;

    //Checking for modified files in "files"
    for (int i = 0; i < ptr->active_branch->n_files; i++) {
        //Comparing recorded and current hash values of tracked files
        if (ptr->active_branch->files[i].hash != hash_file(helper, ptr->active_branch->files[i].file_name)) {
            check_file = 1;
            inx_file = i;
            ptr->active_branch->files[i].hash = hash_file(helper, ptr->active_branch->files[i].file_name);
        }
    }
    //When a file of system has been modified
    if (check_file == 1) {
        //Checking whether the modified file exists in "staging" array 
        for (int j = 0; j < ptr->active_branch->n_staging; j++) {
            //Modifying the hash value of the file 
            if (strcmp(ptr->active_branch->files[inx_file].file_name, ptr->active_branch->staging[j].file_name) == 0) {
                ptr->active_branch->staging[j].hash = hash_file(helper, ptr->active_branch->files[inx_file].file_name);
                check_staging = 1;
                inx_staging = j;
            }
        }
        //When file does not exist in "staging"
        if (check_staging == 0) {
            //Adding file to staging as modified file 
            ptr->active_branch->n_staging++;
            ptr->active_branch->staging = realloc(ptr->active_branch->staging, sizeof(struct file) * ptr->active_branch->n_staging);
            ptr->active_branch->staging[ptr->active_branch->n_staging - 1].file_name = strdup(ptr->active_branch->files[inx_file].file_name);
            ptr->active_branch->staging[ptr->active_branch->n_staging - 1].hash = ptr->active_branch->files[inx_file].hash;
            ptr->active_branch->staging[ptr->active_branch->n_staging - 1].change = '/';
        }
    }
    //Sorting "staging" array
    sort_staging(helper);
    //Calculating id based on "change" type of files existing in "staging array"
    for (int i = 0; i < ptr->active_branch->n_staging; i++) {

        if (ptr->active_branch->staging[i].change == '+') {
            id += 376591;

        }
        if (ptr->active_branch->staging[i].change == '-') {
            id += 85973;
        }
        for (int j = 0; j < strlen(ptr->active_branch->staging[i].file_name); j++) {
            id = (id * (ptr->active_branch->staging[i].file_name[j] % 37)) % 15485863 + 1;
        }
    }

    //Storing id as hexadecimal string
    sprintf(hex, "%06x", id);

    //Adding a commit to "commits" array of the active branch and initializing its attributes
    ptr->active_branch->n_commits++;
    int index = ptr->active_branch->n_commits - 1;
    if (ptr->active_branch->commits == NULL) {
        ptr->active_branch->commits = malloc(sizeof(struct commit));

    }
    else {
        ptr->active_branch->commits = realloc(ptr->active_branch->commits, sizeof(struct commit) * ptr->active_branch->n_commits);
    }
    ptr->active_branch->commits[index].n_cfiles = 0;
    ptr->active_branch->commits[index].n_tracked = 0;
    ptr->active_branch->commits[index].id = strdup(hex);
    free(hex);
    ptr->active_branch->commits[index].msg = strdup(message);
    ptr->active_branch->commits[index].commit_files = NULL;
    ptr->active_branch->commits[index].tracked_files = NULL;

    //Adding files of "staging" to the "commit_files" of the recently added commit 
    for (int i = 0; i < ptr->active_branch->n_staging; i++) {
        ptr->active_branch->commits[index].n_cfiles++;
        if (ptr->active_branch->commits[index].n_cfiles == 1) {
            ptr->active_branch->commits[index].commit_files = malloc(sizeof(struct file));
        }
        else {
            ptr->active_branch->commits[index].commit_files =
                realloc(ptr->active_branch->commits[index].commit_files, sizeof(struct file) * ptr->active_branch->commits[index].n_cfiles);

        }

        ptr->active_branch->commits[index].commit_files[ptr->active_branch->commits[index].n_cfiles - 1].file_name = strdup(ptr->active_branch->staging[i].file_name);
        ptr->active_branch->commits[index].commit_files[ptr->active_branch->commits[index].n_cfiles - 1].change = ptr->active_branch->staging[i].change;
        ptr->active_branch->commits[index].commit_files[ptr->active_branch->commits[index].n_cfiles - 1].hash = ptr->active_branch->staging[i].hash;
        //Deallocating memory which was assigned for name of files in "staging"
        free(ptr->active_branch->staging[i].file_name);
    }
    //Removing everything from "staging"
    free(ptr->active_branch->staging);
    ptr->active_branch->staging = NULL;
    ptr->active_branch->n_staging = 0;

    //Adding tracked files of system to "tracked" array of recently added commit 
    for (int i = 0; i < ptr->active_branch->n_files; i++) {
        ptr->active_branch->commits[index].n_tracked++;
        if (ptr->active_branch->commits[index].n_tracked == 1) {
            ptr->active_branch->commits[index].tracked_files = malloc(sizeof(struct file));
        }
        else {
            ptr->active_branch->commits[index].tracked_files = realloc(ptr->active_branch->commits[index].tracked_files, sizeof(struct file) * ptr->active_branch->commits[index].n_tracked);
        }
        ptr->active_branch->commits[index].tracked_files[ptr->active_branch->commits[index].n_tracked - 1].file_name = strdup(ptr->active_branch->files[i].file_name);
        ptr->active_branch->commits[index].tracked_files[ptr->active_branch->commits[index].n_tracked - 1].change = ptr->active_branch->files[i].change;
        ptr->active_branch->commits[index].tracked_files[ptr->active_branch->commits[index].n_tracked - 1].hash = ptr->active_branch->files[i].hash;
    }

    return ptr->active_branch->commits[index].id;
}
/*
 * Function : get_commit
 * ------------------------------------
 * Searches for a commit in system and returns its address using:
 *
 * helper : Pointer of system struct
 * commit_id : Id of a commit
 *
 * returns  pointer to the address of given commit
 */
void* get_commit(void* helper, char* commit_id) {

    //Checking for nullity of "commit_id"
    if (commit_id == NULL) {
        return NULL;
    }

    struct system* ptr = helper;
    int count = 0;
    //Checking for commit id in list of branches 
    for (int i = 0; i < ptr->branch_num; i++) {
        //Checking "commits" array of each branch
        for (int j = 0; j < ptr->branch_list[i].n_commits; j++) {
            if (strcmp(ptr->branch_list[i].commits[j].id, commit_id) != 0) {
                count++;
                //When commit id is not existing in system 
                if (ptr->branch_list[i].n_commits == count) {
                    return NULL;
                }
            }
            //When commit id has been found 
            else {
                return &(ptr->branch_list[i].commits[j]);
            }
        }
    }
    return NULL;
}
/*
 * Function : get_prev_commits
 * ------------------------------------
 * Searching for parent commits of a particular commit in system and returning their names using:
 *
 * helper : Pointer of system struct
 * commit : Pointer to address where commit is located
 * n_prev : Number of parent commits
 *
 * returns dynamic memory of strings storing names of parent commits of the given commit 
 */
char** get_prev_commits(void* helper, void* commit, int* n_prev) {
    //Checking nullity of "n_prev"
    if (n_prev == NULL) {
        return NULL;
    }
    struct system* ptr = helper;
    //Checking nullity of "commit" and whether the commit is the very first commit in system
    if (commit == NULL || (&(ptr->branch_list[0].commits[0]) == commit)) {
        n_prev[0] = 0;
        return NULL;
    }
    //Looking for commit in branch list
    for (int i = 0; i < ptr->branch_num; i++) {
        //Checking for commi in "commits" of branches
        for (int j = 1; i < ptr->branch_list[i].n_commits; j++) {
            if (&(ptr->branch_list[i].commits[j]) == commit) {
                n_prev[0] = j;
                char** prev_commits = malloc(sizeof(char*) * n_prev[0]);
                //Storing id of parent commits into "prev_commits" array
                for (int k = j - 1; k >= 0; k--) {
                    prev_commits[k] = ptr->branch_list[i].commits[k].id;
                }
                return prev_commits;
            }
        }
    }
    return NULL;
}
/*
 * Function : print_commit
 * ------------------------------------
 * Prints out information about a certain commit using:
 *
 * helper : Pointer of system struct
 * commit_id : Id of a particular commit 
 *
 * Prints out list of files of the commit along with their hash values, its branch name, commit id and message
 */
void print_commit(void* helper, char* commit_id) {
    //Checking nullity of commit id
    if (commit_id == NULL) {
        printf("Invalid commit id\n");
        return;
    }
    struct system* ptr = helper;
    int count = 0;

    for (int i = 0; i < ptr->branch_num; i++) {
        if (ptr->branch_list[i].n_commits == 0) {
            printf("Invalid commit id\n");
            return;
        }

        for (int j = 0; j < ptr->branch_list[i].n_commits; j++) {
            //When given commit id is found in "commits" array of a branch in branch_list
            if (strcmp(ptr->branch_list[i].commits[j].id, commit_id) == 0) {

                printf("%s [%s]: %s\n", ptr->branch_list[i].commits[j].id, ptr->branch_list[i].name, ptr->branch_list[i].commits[j].msg);
                for (int k = 0; k < ptr->branch_list[i].commits[j].n_cfiles; k++) {

                    if (ptr->branch_list[i].commits[j].commit_files[k].change == '+') {

                        printf("    + %s\n", ptr->branch_list[i].commits[j].commit_files[k].file_name);
                    }
                    if (ptr->branch_list[i].commits[j].commit_files[k].change == '-') {

                        printf("    - %s\n", ptr->branch_list[i].commits[j].commit_files[k].file_name);
                    }
                }
                printf("\n    Tracked files (%d):\n", ptr->branch_list[i].commits[i].n_tracked);

                for (int m = 0; m < ptr->branch_list[i].commits[j].n_tracked; m++) {

                    printf("    [%10d] %s\n", ptr->branch_list[i].commits[j].tracked_files[m].hash, ptr->branch_list[i].commits[j].tracked_files[m].file_name);
                }
            }
            else {
                count++;
                //When given commit id does not exists in any of branches
                if (count == ptr->branch_list[i].n_commits) {
                    printf("Invalid commit id\n");
                    return;
                }
            }
        }
    }
}
/*
 * Function : svc_branch
 * ------------------------------------
 * Adds a branch to system if there is no uncommited changes in active branch and if it is not already existing in system, using:
 *
 * helper : Pointer of system struct
 * branch_name : Name of a branch to be created
 *
 * returns  0 branch addition to system is successful 
 */
int svc_branch(void* helper, char* branch_name) {
    //Checkin for nullity of branch name
    if (branch_name == NULL) {
        return -1;
    }
    struct system* ptr = helper;
    int counter = 0;

    for (int i = 0; i < ptr->branch_num; i++) {
        //When branch name already exists   
        if (strcmp(ptr->branch_list[i].name, branch_name) == 0) {

            return -2;
        }
    }
    int valid_check = 0;
    size_t name_len = strlen(branch_name);
    for (size_t i = 0; i < name_len; i++) {
        //Checking for validity of given branch name
        if ((branch_name[i] <= 'z' && branch_name[i] >= 'a')
            || (branch_name[i] <= 'Z' && branch_name[i] >= 'A') ||
            (branch_name[i] <= '9' && branch_name[i] >= '0') || branch_name[i] == '_' || branch_name[i] == '-' ||
            branch_name[i] == '/') {
            counter++;
            if (counter == name_len) {
                valid_check = 1;
            }
        }
        else {
            return -1;
        }
    }
    //When there exists uncommitted changes in active branch
    if (ptr->active_branch->staging != NULL) {
        return -3;
    }
    else {
        if (valid_check == 1) {
            //Adding a new branch and initializing its attributes
            ptr->branch_num++;
            if (ptr->branch_list == NULL) {
                ptr->branch_list = malloc(sizeof(struct branch));
            }
            else {
                ptr->branch_list = realloc(ptr->branch_list, sizeof(struct branch) * ptr->branch_num);
            }
            for (int i = 0; i < ptr->branch_num; i++) {
                if (ptr->branch_list[i].head == 1) {
                    ptr->active_branch = &(ptr->branch_list[i]);
                }
            }
            ptr->branch_list[ptr->branch_num - 1].files = NULL;
            ptr->branch_list[ptr->branch_num - 1].n_files = 0;
            ptr->branch_list[ptr->branch_num - 1].name = strdup(branch_name);
            ptr->branch_list[ptr->branch_num - 1].head = 0;
            ptr->branch_list[ptr->branch_num - 1].commits = NULL;
            ptr->branch_list[ptr->branch_num - 1].staging = NULL;
            ptr->branch_list[ptr->branch_num - 1].n_commits = 0;
            ptr->branch_list[ptr->branch_num - 1].n_staging = 0;
        }
    }

    return 0;
}
/*
 * Function : svc_checkout
 * ------------------------------------
 * Makes the branch called "branch_name" the active branch using:
 *
 * helper : Pointer of system struct
 * branch_name : Name of a branch to be activated
 *
 * returns 0 if activation is successful 
 */
int svc_checkout(void* helper, char* branch_name) {
    //Checking nullity of branch_name
    if (branch_name == NULL) {
        return -1;
    }
    struct system* ptr = helper;
    int count = 0;
    //Looking for given branch name in branch list
    for (int i = 0; i < ptr->branch_num; i++) {
        if (strcmp(ptr->branch_list[i].name, branch_name) == 0) {

            if (ptr->active_branch->n_staging != 0) {
                return -2;
            }
            //Assigning new address to active branch pointer 
            else {
                ptr->branch_list[i].head = 1;
                ptr->active_branch->head = 0;
                ptr->active_branch = &(ptr->branch_list[i]);
                return 0;
            }

        }
        //When branch does not exist in branch list 
        else {
            count++;
            if (count == ptr->branch_num) {
                return -1;
            }
        }
    }
    return 0;
}
/*
 * Function : list_branches
 * ------------------------------------
 * Prints all the branches in the order they were created using:
 *
 * helper : Pointer of system struct
 * n_branches : Number of branches in system 
 *
 * returns a dynamic array of strings containing system branch names 
 */
char** list_branches(void* helper, int* n_branches) {
    //Checking for nullity of n_branches
    if (n_branches == NULL) {
        return NULL;
    }
    struct system* ptr = helper;
    //Creating array of strings and allocating memory for it 
    char** branch_names = malloc(sizeof(char*) * ptr->branch_num);

    for (int i = 0; i < ptr->branch_num; i++) {
        //Storing name of branches into the array created
        branch_names[i] = ptr->branch_list[i].name;
        printf("%s\n", branch_names[i]);
    }
    n_branches[0] = ptr->branch_num;

    return branch_names;
}
/*
 * Function : svc_add 
 * ------------------------------------
 * Adds a file to the system if it is not currently being tracked using:
 *
 * helper : Pointer of system struct
 * file_name : Name of a file to be added
 *
 * returns hash value of the recently added file 
 */
int svc_add(void* helper, char* file_name) {
    //Checking for nullity of file_name
    if (file_name == NULL) {
        return -1;
    }
    struct system* ptr = helper;
    FILE* file_pointer = fopen(file_name, "r");
    //Checking for existance of given file name
    if (file_pointer == NULL) {
        return -3;
    }

    else {
        //Checking for the file in existing tracking files in "files" array
        for (int i = 0; i < ptr->active_branch->n_files; i++) {
            if (strcmp(ptr->active_branch->files[i].file_name, file_name) == 0) {
                return -2;
            }
        }
        //Adding file to "files" and initializing its attributes
        ptr->active_branch->n_files++;
        if (ptr->active_branch->files == NULL) {
            ptr->active_branch->files = malloc(sizeof(struct file));
        }
        if (ptr->active_branch->files != NULL) {
            ptr->active_branch->files = realloc(ptr->active_branch->files, sizeof(struct file) * ptr->active_branch->n_files);
        }

        ptr->active_branch->files[ptr->active_branch->n_files - 1].change = '+';
        ptr->active_branch->files[ptr->active_branch->n_files - 1].file_name = strdup(file_name);
        ptr->active_branch->files[ptr->active_branch->n_files - 1].hash = hash_file(helper, file_name);
        ptr->active_branch->n_staging++;
        if (ptr->active_branch->staging == NULL) {
            ptr->active_branch->staging = malloc(sizeof(struct file));
        }
        else {
            ptr->active_branch->staging = realloc(ptr->active_branch->staging, sizeof(struct file) * ptr->active_branch->n_staging);
        }

        ptr->active_branch->staging[ptr->active_branch->n_staging - 1].file_name = strdup(file_name);
        ptr->active_branch->staging[ptr->active_branch->n_staging - 1].hash = hash_file(helper, file_name);
        ptr->active_branch->staging[ptr->active_branch->n_staging - 1].change = '+';

        return ptr->active_branch->files[ptr->active_branch->n_files - 1].hash;
    }
    return 0;
}
/*
 * Function : svc_rm
 * ------------------------------------
 * Removing a file with given name from the list of tracked files using:
 *
 * helper : Pointer of system struct
 * file_name : Name of the file
 *
 * returns  latest hash value of the file
 */
int svc_rm(void* helper, char* file_name) {

    //Checking nullity of file_name provided
    if (file_name == NULL) {
        return -1;
    }

    struct system* ptr = helper;
    int counter = 0;
    int found = 0;
    int index = 0;
    //Looking for the given file in "files" array of active branch
    for (int i = 0; i < ptr->active_branch->n_files; i++) {
        if (strcmp(ptr->active_branch->files[i].file_name, file_name) == 0 && ptr->active_branch->files[i].change != '-') {
            found = 1;
            index = i;
            //Adding file to "staging" array as removed file and initializing its attributes
            ptr->active_branch->files[i].change = '-';
            ptr->active_branch->n_staging++;
            if (ptr->active_branch->staging == NULL) {
                ptr->active_branch->staging = malloc(sizeof(struct file));

            }
            else {
                ptr->active_branch->staging = realloc(ptr->active_branch->staging, sizeof(struct file) * ptr->active_branch->n_staging);
            }
            ptr->active_branch->staging[ptr->active_branch->n_staging - 1].file_name = strdup(file_name);
            ptr->active_branch->staging[ptr->active_branch->n_staging - 1].hash = hash_file(helper, file_name);
            ptr->active_branch->staging[ptr->active_branch->n_staging - 1].change = '-';
            return hash_file(helper, file_name);

        }
        //When file does not exist in active branch "files" array 
        else {
            counter++;
            if (counter == ptr->active_branch->n_files) {

                return -2;
            }
        }
    }
    //Removing the file from "files" array 
    if (found == 1) {
        free(ptr->active_branch->files[index].file_name);
        ptr->active_branch->n_files--;
        for (int m = index; m < ptr->active_branch->n_files; m++) {
            ptr->active_branch->files[m] = ptr->active_branch->files[m + 1];
        }
    }
    return 0;
}

int svc_reset(void* helper, char* commit_id) {
    // TODO: Implement
    return 0;
}


char* svc_merge(void* helper, char* branch_name, struct resolution* resolutions, int n_resolutions) {
    // TODO: Implement
    return NULL;
}
