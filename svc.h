#ifndef svc_h
#define svc_h

struct commit {
    char* msg;
    char* id;
    struct file* commit_files;
    int n_cfiles;
    struct file* tracked_files;
    int n_tracked;
};

struct branch {
    char* name;
    int head;
    struct commit* commits;
    int n_commits;
    struct file* files;
    int n_files;
    struct file* staging;
    int n_staging;
};

struct file {
    char* file_name;
    int hash;
    char change;
};

struct system {
    struct branch* branch_list;
    int branch_num;
    struct branch* active_branch;
};
typedef struct resolution {
    // NOTE: DO NOT MODIFY THIS STRUCT
    char* file_name;
    char* resolved_file;
} resolution;

void* svc_init(void);

void cleanup(void* helper);

int hash_file(void* helper, char* file_path);

char* convert_name(char* file_name);

void sort_staging(void* helper);

char* svc_commit(void* helper, char* message);

void* get_commit(void* helper, char* commit_id);

char** get_prev_commits(void* helper, void* commit, int* n_prev);

void print_commit(void* helper, char* commit_id);

int svc_branch(void* helper, char* branch_name);

int svc_checkout(void* helper, char* branch_name);

char** list_branches(void* helper, int* n_branches);

int svc_add(void* helper, char* file_name);

int svc_rm(void* helper, char* file_name);

int svc_reset(void* helper, char* commit_id);

char* svc_merge(void* helper, char* branch_name, resolution* resolutions, int n_resolutions);

#endif
