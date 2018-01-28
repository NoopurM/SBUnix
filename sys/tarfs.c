#include<sys/tarfs.h>
#include<sys/defs.h>
#include<sys/kprintf.h>
#include<sys/paging.h>

extern int strcmp(char *s1, char *s2);
extern void strcpy(char *s1, char *s2);
size_t strlen(const char *s);

char filename[300];

void start_tarfs() {

    struct posix_header_ustar *tarheader;
    tarheader = (struct posix_header_ustar *)(&_binary_tarfs_start);

    parse_tarheader(tarheader);

}

void parse_tarheader(struct posix_header_ustar *tarheader) {

    int offset = 0;
    int typeflag = 0;
    init_tree();
    while(1){
        tarheader = (struct posix_header_ustar *)(&_binary_tarfs_start + offset);

        if(tarheader->name[0] == '\0')
            break;

        uint64_t filesize = parse_filesize(tarheader);

        parse_filename(tarheader);
        typeflag = atoi(tarheader->typeflag);
        typeflag = typeflag;
        //kprintf("filesize: %d, filename: %s, type: %d", filesize, filename, typeflag);
        insert_node(filesize, filename, typeflag, tarheader);    
        
        offset += ((filesize / 512) + 1) * 512;
        if(filesize % 512) {
            offset += 512;
        }
    }
}

uint64_t parse_filesize(struct posix_header_ustar *tarheader) {
    uint64_t filesize = 0;
    filesize = OctToDec((atoi(tarheader->size)));
    return filesize;
}

void  parse_filename(struct posix_header_ustar *tarheader) {
    int i; int k=0;
    for(i=0;i<100;i++) {
        if(tarheader->name[0] == 0) {
            break;
        }
        filename[k++] = tarheader->name[i];
    }
    filename[i] = '\0';
}

uint64_t atoi(char *c) {
    uint64_t no = 0;
    while( *c >= '0' && *c <= '9') {
        no = no * 10 + (*c - '0');
        c++;
    }
    return no;
}

uint64_t power(uint64_t x, int e) {
    if(e == 0)
        return 1;

    return x * power(x, e-1);
}

uint64_t OctToDec( uint64_t oct) {
    uint64_t dec = 0;
    int i=0;
    while(oct!=0) {
        dec = dec + (oct % 10) * power(8,i++);
        oct = oct/10;
    }
    return dec;
}

void init_tree() {

    rootfs = (struct elf_node *)kmalloc(sizeof(struct elf_node));
    strcpy("rootfs",rootfs->filename);
    rootfs->type = DIRECTORY_TYPE;
    rootfs->parent = rootfs;
}

int strtok(char filename[50], char res[20][50]){
    int i=0,k=0,j=0;
    while(filename[i] != '\0') {
        if(filename[i] == '/') {
            res[j][k] = '\0';
            j++;
            k=0;
            i++;
        } 
        else {
            res[j][k++] = filename[i++];
        }
    }
    res[j][k]='\0';
    return j;
}

struct elf_node *insert_node(int filesize, char filename[50], int type, struct posix_header_ustar *tarheader) {
        
        char res[20][50];
        strtok(filename, res);
        
        struct elf_node *ptr=NULL;
        ptr = rootfs;
        int i=0, k=0, found=0;
        while(ptr != NULL) {
            k = found = 0;
            while(ptr->child[k] != NULL && k<30){
                if(strcmp(res[i], ptr->child[k]->filename) == 0){
                   ptr = ptr->child[k];
                   i++;
                   found=1;
                   break;
                }
                k++;
            }
            if ((found == 0) && (k<30)) {
                ptr->child[k] = (struct elf_node *)kmalloc(sizeof(struct elf_node));
                strcpy(res[i], ptr->child[k]->filename);
                ptr->child[k]->type = type;
                ptr->child[k]->header_address = (uint64_t)tarheader;
                ptr->child[k]->last_read=ptr->child[k]->header_address + 512;
                ptr->child[k]->filesize = filesize;
                ptr->child[k]->parent = ptr;
                    return ptr->child[k];
            }
        }
        return NULL;
}

struct elf_node *search_node(char filename[50]) {
    struct elf_node *ptr = rootfs;
    int i=0,k;
    char res[20][50];
    int no = strtok(filename, res);
    //no = no - 1;//ignore empty string

    while(ptr != NULL) {
        k=0;
        while(strcmp(res[i], ptr->child[k]->filename) != 0) {
            k++;
        }
        if(ptr->child[k] != NULL && k<30){
            i++;
            if(i > no) {
                return ptr->child[k];
            }
            ptr = ptr->child[k];
        }
        else {
            return NULL;
        }
    }
    return NULL;
}

struct posix_header_ustar *get_elf_file_from_tarfs(char given_file_name[50]) {

	struct posix_header_ustar *tarheader;
	int offset = 512;
    while(1){
        tarheader = (struct posix_header_ustar *)(&_binary_tarfs_start + offset);

        if(tarheader->name[0] == '\0')
            break;

        uint64_t filesize = parse_filesize(tarheader);
		parse_filename(tarheader);
		if(strcmp(filename, given_file_name) == 0)
			return tarheader;
		
        offset += ((filesize / 512) + 1) * 512;
        if(filesize % 512) {
            offset += 512;
        }
    }
	return 0;
}
