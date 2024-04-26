#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

int alphasort_case_insensitive(const struct dirent ** a, const struct dirent **b) {
  return(strcasecmp((*(const struct dirent **)a)->d_name,
                    (*(const struct dirent **)b)->d_name));
}

static int one (const struct dirent *unused)
{
  return 1;
}

void printFileInfo(struct dirent *file, int mode, int size_type) {

    
    struct stat eStat;
    lstat(file->d_name, &eStat);
    struct passwd *pws;
    struct group *file_group;


    char file_perm[15] = {0};

    
    switch (eStat.st_mode & S_IFMT) {
        case S_IFBLK:  
            sprintf(file_perm, "%s%c", file_perm, 'b');
            break;
        case S_IFCHR:  
            sprintf(file_perm, "%s%c", file_perm, 'c'); 
            break;
        case S_IFDIR:                 
            sprintf(file_perm, "%s%c", file_perm, 'd');
            break;
        case S_IFIFO:            
            sprintf(file_perm, "%s%c", file_perm, 'p');
            break;
        case S_IFLNK:                   
            sprintf(file_perm, "%s%c", file_perm, 'l');
            break;
        case S_IFREG:          
            sprintf(file_perm, "%s%c", file_perm, '-');
            break;
        case S_IFSOCK:                   
            sprintf(file_perm, "%s%c", file_perm, 's');
            break;
        default:  
            sprintf(file_perm, "%s%c", file_perm, ' ');
            break;
    }   

    // user rwx 
    if (eStat.st_mode & S_IRUSR) {

        sprintf(file_perm, "%s%c", file_perm, 'r');
    } else {

        sprintf(file_perm, "%s%c", file_perm, '-');
    } 

    if (eStat.st_mode & S_IWUSR) {

        sprintf(file_perm, "%s%c", file_perm, 'w');
    } else {

        sprintf(file_perm, "%s%c", file_perm, '-');
    } 

    if (eStat.st_mode & S_IXUSR) {

        sprintf(file_perm, "%s%c", file_perm, 'x');
    } else {

        sprintf(file_perm, "%s%c", file_perm, '-');
    } 

    // group rwx
    if (eStat.st_mode & S_IRGRP) {

        sprintf(file_perm, "%s%c", file_perm, 'r');
    } else {

        sprintf(file_perm, "%s%c", file_perm, '-');
    } 

    if (eStat.st_mode & S_IWGRP) {
        
        sprintf(file_perm, "%s%c", file_perm, 'w');
    } else {

        sprintf(file_perm, "%s%c", file_perm, '-');
    } 

    if (eStat.st_mode & S_IXGRP) {
        
        sprintf(file_perm, "%s%c", file_perm, 'x');
    } else {

        sprintf(file_perm, "%s%c", file_perm, '-');
    } 

    // other rwx
    if (eStat.st_mode & S_IROTH) {
        
        sprintf(file_perm, "%s%c", file_perm, 'r');
    } else {

        sprintf(file_perm, "%s%c", file_perm, '-');
    } 

    if (eStat.st_mode & S_IWOTH) {
        
        sprintf(file_perm, "%s%c", file_perm, 'w');
    } else {

    char size_name[10];
        sprintf(file_perm, "%s%c", file_perm, '-');
    } 

    if (eStat.st_mode & S_IXOTH) {
        
        sprintf(file_perm, "%s%c", file_perm, 'x');
    } else {

        sprintf(file_perm, "%s%c", file_perm, '-');
    } 

    intmax_t size = (intmax_t) eStat.st_size;
    intmax_t del = 1024;
    imaxdiv_t i_div = {0};
    int pref = 0;
    
    if(size_type && !(size < del)) {

        while(size >= del && pref < 4) {           

            if (size/del < del) {

                i_div = imaxdiv(size, del); 
                ++pref;  
                break;
            } else {

                size /= del;
            }
            ++pref;
        }
    } else {

        i_div = imaxdiv(size, 1);  
    }

    char file_size_str[25];
    if (i_div.rem != 0) {

        sprintf(file_size_str, "%ju,%ju", i_div.quot, i_div.rem);
    } else {

        sprintf(file_size_str, "%ju", i_div.quot);
    }

    char size_name[10];
    switch(pref) {
        case 0:
            strcpy(size_name, "bytes");
            size_name[5] = '\0';
            break;
        case 1:
            strcpy(size_name, "M");
            size_name[1] = '\0';
            break;
        case 2:
            strcpy(size_name, "K");
            size_name[1] = '\0';
            break;
        case 3:
            strcpy(size_name, "GB");
            size_name[2] = '\0';
            break;
        case 4:
            strcpy(size_name, "TB");
            size_name[2] = '\0';
            break;
    }

    sprintf(file_size_str, "%s %s", file_size_str, size_name);

    char uid[5];
    sprintf(uid, "%u", eStat.st_uid);
    pws = getpwuid(atoi(uid));

    char gid[5];
    sprintf(gid, "%u", eStat.st_gid);
    file_group = getgrgid(atoi(gid));

    char *c_time = ctime(&eStat.st_mtime);
    char time[25] = {0};
    strncpy(time, c_time, strlen(c_time)-2);

    if(mode)
        printf("%s %s %s %15s %s %s\n", file_perm, pws->pw_name, file_group->gr_name, file_size_str, time, file->d_name);
    else 
        printf("%s ", file->d_name);

}


int main(int argc, char** argv) {

    struct dirent **files;

    int rev = 0;
    int mode = 0;
    int size_type = 0;
    int all = 0;

    char dir_path[128];
    if(argv[argc-1][0] != '-') {
        if(argc > 1) {

            if (argv[argc-2][0] != '-') {

                sprintf(dir_path, "%s", argv[argc-1]);
            } else {

                sprintf(dir_path, ".");
            }
        } else {

            sprintf(dir_path, ".");
        }
    } else {

        sprintf(dir_path, ".");
    }

    int opt = 0;    
    while( (opt = getopt(argc, argv, "a::l::r::h::")) != -1) {
        switch(opt) {

            case 'l':
                mode = 1;
                break;
            case 'a':
                all  = 1;
                break;
            case 'r':
                rev = 1;
                break;
            case 'h':
                size_type = 1;
                break;
            default:
        }   
    }

    int n;
    n = scandir (dir_path,  &files, one, alphasort_case_insensitive);
    if (n >= 0) {

        int cnt;
        if(!rev) {
            
            for (int i = 0; i < n; ++i) {

                if (!all & files[i]->d_name[0] == '.')
                    continue;
                printFileInfo(files[i], mode, size_type);
            }
        } else {

            for (int i = n-1; i >= 0; --i) {

                if (!all & files[i]->d_name[0] == '.')
                    continue;
                printFileInfo(files[i], mode, size_type); 
            }
        }
    }

    if(!mode) 
        printf("\n");

    while (n--) {
        free(files[n]);
    }           
    free(files);

    return 0;
}