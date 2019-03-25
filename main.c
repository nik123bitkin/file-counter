#include <stdbool.h>
#include <libgen.h>
#include <stdio.h>
#include <sys/stat.h>
#include <memory.h>
#include <dirent.h>
#include <stdlib.h>

#define VALID_ARGS 3

#define MISSING_ARG "Missing argument\n"
#define DIR_ERR "Directory not found/ not a directory\n"
#define FILE_ERR "File cannot be opened\n"
#define OPEN_ERR "Unable to open dir"

#define EXIT_ERR(M)     do{ throwError(M); return 1;} while(0)
#define CMP(A,B)            _Generic((A), char*:   strcmp(A, B),\
                                          float: ((abs((A) - (B)) > 0.0001) - (abs((A) - (B)) < 0.0001)),\
                                          double: ((abs((A) - (B)) > 0.0001) - (abs((A) - (B)) < 0.0001)),\
                                          default: (((A) > (B))-((A) < (B))))
#define string            char*
#define const_string      const char*
#define MALLOC_STR(S)     (char *)malloc((S))
#define REALLOC_STR(P, S) do{free(P); P = MALLOC_STR((S));} while(0)

void throwError(const_string msg);
bool validateDir(const_string path);
void parseDir(const_string path);
string getFullPath(const_string path, const_string name);

string PROG_NAME;
FILE *out;

int main(int argc, string argv[], string envp[])
{
    PROG_NAME = basename(argv[0]);
    if(argc < VALID_ARGS)
        EXIT_ERR(MISSING_ARG);

    if(!validateDir(argv[1]))
        EXIT_ERR(DIR_ERR);

    if(!(out = fopen(argv[2], "w+")))
        EXIT_ERR(FILE_ERR);

    parseDir(argv[1]);

    return 0;
}

inline void throwError(const_string msg){
    fprintf(stderr, "%s: %s\n", PROG_NAME, msg);
}

inline bool validateDir(const_string path){
    struct stat statbuf;
    return stat(path, &statbuf) == 0 && S_ISDIR(statbuf.st_mode);
}

string getFullPath(const_string path, const_string name){
    size_t pathLength = sizeof(char) * (strlen(path) + strlen(name) + 2);
    string fullPath = MALLOC_STR(pathLength);
    strcpy(fullPath, path);
    strcpy(fullPath + strlen(path), "/");
    strcpy(fullPath + strlen(path) + 1, name);
    return fullPath;
}

void parseDir(const_string path){
    DIR *dir;
    if ((dir = opendir(path))){
        struct dirent* direntbuf;
        long size = 0, max = -1, count = 0;
        string maxName = NULL;
        while ((direntbuf = readdir(dir))) {
            if (CMP(direntbuf->d_name, ".") && CMP(direntbuf->d_name, "..") && direntbuf->d_type != DT_UNKNOWN) {
                if (direntbuf->d_type == DT_DIR) {
                    string name = getFullPath(path, direntbuf->d_name);
                    parseDir(name);
                    free(name);
                } else if(direntbuf->d_type == DT_REG){

                    struct stat statbuf;
                    string name = getFullPath(path, direntbuf->d_name);

                    if (stat(name, &statbuf) == 0) {
                        size += statbuf.st_size;
                        count++;
                        if (CMP(statbuf.st_size,max) > 0) {
                            max = statbuf.st_size;
                            REALLOC_STR(maxName, strlen(direntbuf->d_name) + 1);
                            strcpy(maxName, direntbuf->d_name);
                        }
                    }
                    free(name);
                }
            }
        }

        printf("%s %ld %ld %ld %s\n", path, count, size,  max, maxName ? maxName : "");
        fprintf(out, "%s %ld %ld %ld %s\n", path, count, size, max, maxName ? maxName : "");
        closedir(dir);
    }
    else {
        fprintf(stderr, "%s: %s %s\n", PROG_NAME, OPEN_ERR, path);//throwError();
        return;
    }
}