#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> 
#include<math.h>
#define DATA_SIZE 16
#define FDUMP_VERSION 1.0
#define PROGRAMMER "A0561"

//列印資料
void print_data(FILE* file, int *offset_ori,int *offset_begin) 
{
    int times = 0x140; //一次印出最大的量
    int read_bytes;    //fread讀到的bytes

    int offset_tilte = 0x00; //顯示在前面的offset
    int offset = 0x00;       //目前的offset
    offset = *offset_ori;
    offset_tilte = offset/16*16; //顯示在前面的offset，使用offset捨去個位
    *offset_begin = offset;  //offset的起始點

    int offset_asii = offset;
    int offset_tilte_asii = offset_tilte; //for ascii 使用
    //printf("*offset_begin %x", *offset_begin);
    
    //設定讀取開始位置(offset_tilte)
    fseek(file, offset_tilte, SEEK_SET);
    unsigned char data[DATA_SIZE]; 

    //開始讀檔print
    while ((read_bytes=fread(data, 1, DATA_SIZE, file)) > 0 && times > 0) 
    {
        //print title
        printf("%06X\t", offset_tilte);
       
        //print 16進制內容
        for (int i = 0; i < read_bytes; i++) 

        {
            //如果輸入offset不為整數位->offset_tilte仍顯示整數位，但從offset實際位置開始顯示數值，其餘空白。
            if (offset <= offset_tilte)
            {
                //printf("offset[%x]",offset);
                printf("%02X", data[i]);
                printf(" ");
                
                offset++;
                *offset_ori = offset;    
            }
            else
            {
                printf("   ");
            }
            offset_tilte++;
        }
        //print 一般內文
        for (int j = 0; j < read_bytes; j++)
        {
            //isprint去除換行符號
            if (isprint(data[j]) && offset_asii <= offset_tilte_asii)
            {
                printf("%c", data[j]);
                offset_asii++;
            }
            else if (offset_asii > offset_tilte_asii)
            {
                printf(" ");
            }
            else
            {
                printf(".");
            }
            offset_tilte_asii++;
        }
        printf("\n");
        times = times - DATA_SIZE;
    }
}
//寫入資料
void write_data(FILE* file, int *offect_ori ,int data) 
{
    //在offset寫入輸入資料
    int offset = 0x00;
    offset = *offect_ori;
    fseek(file, offset, SEEK_SET);
    fwrite(&data, 1, 1, file);
}
//搜尋資料
void search_data(FILE* file,int *offset_begin, int find_data)
{
    int offset = *offset_begin;
    int read_bytes;
    unsigned char data[DATA_SIZE];
    //從輸入的offset開始比對資料
    fseek(file, offset,SEEK_SET);
    while ((read_bytes = fread(data,1, DATA_SIZE,file))>0)      
    {
        /*printf("read_bytes %d\n", read_bytes);*/
        for (size_t i = 0; i < read_bytes; i++)
        {
            //printf("data[i] = %x ,find_data %x \n", data[i], find_data);
            if (data[i] == find_data) 
            {
                printf("Found the %x at offset %x\n", find_data,offset);
                goto key_s;   //跳脫大迴圈，找到則不再搜尋
            }
            offset++;
        }
    }
    key_s: 
    { 
        *offset_begin = offset;
    }
}
//列印按鍵功能
void print_usage()
{
    printf("d\t\t\tContinue dump data\n");
    printf("d [offset]\t\tDump data from [offset]\n");
    printf("f <offset><data>\tFill data from <offset>\n");
    printf("l <offset>\t\tLocate current position to <offset>\n");
    printf("s <pattern>\t\tSearch data pattern\n");
    printf("?\t\t\tHelp\n");
    printf("q\t\t\tQuit this DOS\n");
}
//判斷字串是否為16進制運算
int check_hex(char command[50])
{
    int lenth = strlen(command);
    for (size_t i = 2; i < lenth; i++)
    {
        if (isxdigit(command[i]) == 0)
        {
            return 0;
            break;
        }
    }
    return 1;
}
//判斷字串是否為16進制運算_for_f
int check_hex_f(char command[50])
{
    int lenth = strlen(command);
    for (size_t i = 0; i < lenth; i++)
    {
        if (isxdigit(command[i]) == 0)
        {
            return 0;
            break;
        }
    }
    return 1;
}

//尋找該行第一筆資料
int find_first_data(FILE* file, int* offset_ori)
{
    //offset_ori轉換成該行第一位->offset_title，以該行第一位尋找數據。
    int offset_title = 0x00;
    offset_title = *offset_ori / 16 * 16;
    unsigned int data = 0x00;
    fseek(file, offset_title, SEEK_SET);
    fread(&data, 1, 1, file);
    //printf("offset_title [%x]data[%02X]", offset_title, data);
    return data;
}
//尋找off的值
int find_off_data(FILE* file, int offset)
{
    unsigned int data = 0x00;
    fseek(file, offset, SEEK_SET);
    fread(&data, 1, 1, file);
    return data;
}

//按鍵判斷
void enter_command(FILE* file,int size) 
{
    int offset_ori = 0x00;  //目前所在offset
    int offset_begin = 0x00; //目前offset起始點(定值)
    int* off = &offset_ori;
    int* off_b = &offset_begin;

    char command[50];   
    int new_data = 0x00;    //寫入的資料
    int find_data = 0x00;   //尋找的資料
    int tohex = 0;          //確認輸入為16進制數值
    while (1)
    {
        int err = 0;
        if (fgets(command, sizeof(command), stdin) == NULL) {
            perror("Error reading command");
            exit(EXIT_FAILURE);
        }
        command[strcspn(command, "\n")] = 0;
        
        if (strcmp(command, "q") == 0) {
            break;
        }
        //輸入d
        else if (strcmp(command, "d") == 0)
        {
            //只輸入d
            if (strlen(command) == 1)
            {
                if (offset_ori >= size)
                {
                    printf("End of file\n");
                }
                else
                {
                    print_data(file, off, off_b);
                }
            }
            else
            {
                fprintf(stderr, "Invalid offset.\n");
            }
        }
        //輸入d <offset>
        else if (strncmp(command, "d ", 2) == 0)
        {
            //判斷字串是否為16進制運算
            tohex = check_hex(command); 
            /*printf("tohex[%d]", tohex);*/
            
            //確認字符是否為16進制 && 是否大於size，如未符合條件，將位置定位在上一次開始位置offset_begin
            if (sscanf(command + 2, "%x", &offset_ori) == 1 && (offset_ori < size) && tohex == 1)
            {
                print_data(file, off, off_b);
            }
            else 
            {
                err = 1;
                goto err;
            }
        }
        //輸入f <offset> <data>
        else if(strncmp(command, "f ", 2) == 0)
        {
            //分割字串，判斷字串是否為16進制運算
            char command_copy[50];
            sprintf(command_copy, command);
            char* str_command = strtok(command_copy, " ");

            while (str_command = strtok(NULL, " ")) 
            {
                tohex = check_hex_f(str_command); 
                if (tohex == 0) 
                {
                    break;
                }
            }

            //確認字符是否為16進制 && 是否大於size，如未符合條件，將位置定位在上一次開始位置offset_begin
            if (sscanf(command + 2, "%x %x", &offset_ori, &new_data) == 2 && (offset_ori < size) && tohex == 1)
            {
                write_data(file, off, new_data);
                print_data(file, off, off_b);
            }
            else
            {
                err = 1;
                goto err;
            }
        }
        //輸入l <offset> 
        else if (strncmp(command, "l ", 2) == 0) 
        {
            //判斷字串是否為16進制運算
            tohex = check_hex(command); 
            //確認字符是否為16進制 && 是否大於size，如未符合條件，將位置定位在上一次開始位置offset_begin
            if (sscanf(command + 2, "%x", &offset_ori) == 1 && (offset_ori < size) && tohex == 1)
            {
                offset_begin = offset_ori;
            }
            else
            {
                err = 1;
                goto err;
            }
        }
        //輸入s <offset> 
        else if (strncmp(command, "s", 1) == 0)
        {
            //判斷字串是否為16進制運算
            tohex = check_hex(command); 
            //確認字符是否為16進制 && 是否大於size，如未符合條件，將位置定位在上一次開始位置offset_begin
            if (sscanf(command + 1, "%x", &find_data) == 1 && (offset_ori < size) && tohex == 1)
            {
                search_data(file, off_b, find_data);
                offset_ori = offset_begin;
                print_data(file, off, off_b);
            }
            else
            {
                err = 1;
                goto err;
            }
            
        }
        //輸入? <offset> 
        else if(strcmp(command, "?") == 0)
        {
            print_usage();
        }
        else
        {
            print_usage();
        }

        //錯誤指令處理
        err:
        {
            if (err == 1) {
                if (tohex == 0)
                {
                    offset_ori = offset_begin;
                    fprintf(stderr, "Invalid offset.\n");
                }
                else if (offset_ori >= size)
                {
                    offset_ori = offset_begin;
                    fprintf(stderr, "over size.\n");
                }
                else
                {
                    offset_ori = offset_begin;
                    fprintf(stderr, "Invalid command.\n");
                }
            }
        }
    }
}

int main(int argc, char* argv[])
{    
    int sizes;
    char file_n[50] = "";
     //檢查是否提供了檔案名
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
    }

    for (size_t i = 1; i < argc; i++)
    {
        strcat(file_n, argv[i]);
        if (i < argc - 1)
        {
            strcat(file_n, " ");
        }
    }

    
    const char *filename = file_n;
    
    // 開啟指定的檔案
    FILE* file = fopen(filename, "r+b");
    //開啟失敗or成功
    if (file == NULL) {
        printf("FDUMP File Dump Version %.1f\tProgrammer:%s\n"
               "File Open error\n", FDUMP_VERSION, PROGRAMMER);
        return 0;
    }
    else
    {
        //取得檔案size
        fseek(file, 0, SEEK_END);
        sizes = ftell(file);
        fseek(file, 0, SEEK_SET);

        printf("FDUMP File Dump Version %.1f\tProgrammer:%s\n"
                "File Open Successful!\n"
                "File Name : %s \tsize %d [%x]\n", FDUMP_VERSION, PROGRAMMER, filename,sizes,sizes);

        //按鍵判斷
        enter_command(file, sizes);
    }
    // 關閉檔案
    fclose(file);
    return 0;
}
//test_branch_test22222