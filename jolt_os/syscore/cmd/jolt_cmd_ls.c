#include "stdio.h"
#include "esp_spiffs.h"
#include "esp_vfs_dev.h"
#include "syscore/filesystem.h"

int jolt_cmd_ls(int argc, char** argv) {
    const char path[] = SPIFFS_BASE_PATH;

    DIR *dir = NULL;
    struct dirent *ent;
    char type;
    char size[9];
    char tpath[255];
    char tbuffer[80];
    struct stat sb;
    struct tm *tm_info;
    int statok;

    printf("LIST of DIR [%s]\r\n", path);
    // Open directory
    dir = opendir(path);
    if (!dir) {
        printf("Error opening directory\r\n");
        return  1;
    }

    // Read directory entries
    uint64_t total = 0;
    int nfiles = 0;
    printf("T      Size    Date/Time         Name\r\n");
    printf("-----------------------------------\r\n");
    while ((ent = readdir(dir)) != NULL) {
        sprintf(tpath, path);
        if (path[strlen(path)-1] != '/') strcat(tpath,"/");
        strcat(tpath,ent->d_name);
        tbuffer[0] = '\0';

        // Get file stat
        statok = stat(tpath, &sb);

        if (statok == 0) {
            tm_info = localtime(&sb.st_mtime);
            strftime(tbuffer, 80, "%d/%m/%Y %R", tm_info);
        }
        else sprintf(tbuffer, "                ");

        if (ent->d_type == DT_REG) {
            type = 'f';
            nfiles++;
            if (statok) strcpy(size, "       ?");
            else {
                total += sb.st_size;
                if (sb.st_size < (1024*1024)) sprintf(size,"%8d", (int)sb.st_size);
                else if ((sb.st_size/1024) < (1024*1024)) sprintf(size,"%6dKB", (int)(sb.st_size / 1024));
                else sprintf(size,"%6dMB", (int)(sb.st_size / (1024 * 1024)));
            }
        }
        else {
            type = 'd';
            strcpy(size, "       -");
        }

        printf("%c  %s  %s  %s\r\n",
            type,
            size,
            tbuffer,
            ent->d_name
        );
    }
    if (total) {
        printf("-----------------------------------\r\n");
        if (total < (1024*1024)) printf("   %8d", (int)total);
        else if ((total/1024) < (1024*1024)) printf("   %6dKB", (int)(total / 1024));
        else printf("   %6dMB", (int)(total / (1024 * 1024)));
        printf(" in %d file(s)\r\n", nfiles);
    }
    printf("-----------------------------------\r\n");

    closedir(dir);

    uint32_t tot, used;
    esp_spiffs_info(NULL, &tot, &used);
    printf("SPIFFS: free %d KB of %d KB\r\n", (tot-used) / 1024, tot / 1024);
    return 0;
}
