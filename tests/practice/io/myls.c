/* ls 1.0 by YangZhu at 2009/12/18
 * functions:
 * 1  support options: -dlira
 * 2  support all file type including c and b
 * 3  supoort all permissions including t and s
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <dirent.h>

#include <linux/kdev_t.h>

/* √¸¡Ó––—°œÓ∫Í */
#define A ~(~0<<1)<<0
#define B ~(~0<<1)<<1
#define C ~(~0<<1)<<2
#define D ~(~0<<1)<<3
#define E ~(~0<<1)<<4
#define F ~(~0<<1)<<5
#define G ~(~0<<1)<<6
#define H ~(~0<<1)<<7
#define I ~(~0<<1)<<8
#define J ~(~0<<1)<<9
#define K ~(~0<<1)<<10
#define L ~(~0<<1)<<11
#define M ~(~0<<1)<<12
#define N ~(~0<<1)<<13
#define O ~(~0<<1)<<14
#define P ~(~0<<1)<<15
#define Q ~(~0<<1)<<16
#define R ~(~0<<1)<<17
#define S ~(~0<<1)<<18
#define T ~(~0<<1)<<19
#define U ~(~0<<1)<<20
#define V ~(~0<<1)<<21
#define W ~(~0<<1)<<22
#define X ~(~0<<1)<<23
#define Y ~(~0<<1)<<24
#define Z ~(~0<<1)<<25
#define IS_A(mode) ((mode)&(~(~0<<1)<<0))  
#define IS_B(mode) ((mode)&(~(~0<<1)<<1))  
#define IS_C(mode) ((mode)&(~(~0<<1)<<2))  
#define IS_D(mode) ((mode)&(~(~0<<1)<<3))  
#define IS_E(mode) ((mode)&(~(~0<<1)<<4))  
#define IS_F(mode) ((mode)&(~(~0<<1)<<5))  
#define IS_G(mode) ((mode)&(~(~0<<1)<<6))  
#define IS_H(mode) ((mode)&(~(~0<<1)<<7))  
#define IS_I(mode) ((mode)&(~(~0<<1)<<8))  
#define IS_J(mode) ((mode)&(~(~0<<1)<<9))  
#define IS_K(mode) ((mode)&(~(~0<<1)<<10))  
#define IS_L(mode) ((mode)&(~(~0<<1)<<11))  
#define IS_M(mode) ((mode)&(~(~0<<1)<<12))  
#define IS_N(mode) ((mode)&(~(~0<<1)<<13))  
#define IS_O(mode) ((mode)&(~(~0<<1)<<14))  
#define IS_P(mode) ((mode)&(~(~0<<1)<<15))  
#define IS_Q(mode) ((mode)&(~(~0<<1)<<16))  
#define IS_R(mode) ((mode)&(~(~0<<1)<<17))  
#define IS_S(mode) ((mode)&(~(~0<<1)<<18))  
#define IS_T(mode) ((mode)&(~(~0<<1)<<19))  
#define IS_U(mode) ((mode)&(~(~0<<1)<<20))  
#define IS_V(mode) ((mode)&(~(~0<<1)<<21))  
#define IS_W(mode) ((mode)&(~(~0<<1)<<22))  
#define IS_X(mode) ((mode)&(~(~0<<1)<<23))  
#define IS_Y(mode) ((mode)&(~(~0<<1)<<24))  
#define IS_Z(mode) ((mode)&(~(~0<<1)<<25))  

/*defined this struct FILEPATH mainly to deal with 
 * the display of file which in the sub directory */
typedef struct FILEPATH
{
	char *full_path;
	char *name;
} Fpath;

int my_get_opt(const int argc, const char **argv);
int get_file_list(const int argc, const char **argv, Fpath **file_list);
int show_dir(Fpath *file_list, const long int mode);
int show_file(Fpath *file_list, struct stat *buf, const long int mode);
int show_link_content(Fpath *file_list, struct stat *buf);

int main(int argc, char **argv)
{
	int i = 0;
	Fpath *file_list = NULL; /*get the file list from the 
				   command line and store them in a struct Fpath. */
	int file_num = 0; /*the file num that get from the command line */
	struct stat buf;
	long int mode = 0; /*option status */
	
	mode = my_get_opt(argc, (const char **)argv); /*get the option and store the in a long int 
					 variable define in "myopt.h" file */
	file_num = get_file_list(argc, (const char **)argv, &file_list);

/*----------------------------------------------------------------------*/
/*display every file that store in the file_list */
	for ( i = 0; i < file_num; i++ )
	{
		if ( lstat(file_list[i].full_path, &buf) == -1 )
		{
			perror(file_list[i].full_path);
			continue;
		}
/*if file is a symbol file, raad the name of its refer 
 * and stroe it in the buf point by *real_path */
		if ( S_ISLNK(buf.st_mode) )
		{
			show_file((file_list + i), &buf, mode);
			if ( IS_L(mode) )
			{
				show_link_content((file_list + i), &buf);
				printf("\n");
			}
		}
/*if file is a directory, make decision from the options 
 * whether display the directory itself or the files 
 * it contains. this  is judge by a macro IS_D(mode) */
		else if ( S_ISDIR(buf.st_mode) )
		{
			if ( IS_D(mode) )
			{
				show_file((file_list + i), &buf, mode);
				if ( IS_L(mode) )
					printf("\n");
				continue;
			}
			else
			{
				show_dir((file_list + i), mode);
				if ( IS_L(mode) )
					printf("\n");
			}
		}
		else 
		{
			show_file((file_list + i), &buf, mode);
			if ( IS_L(mode) )
				printf("\n");
		}
	}
	
	free(file_list);
	if ( !(IS_L(mode)) )
		printf("\n");
	
	return 0;
}

int show_link_content(Fpath *file_list, struct stat *buf)
{
	char real_path_buf[512] = { '\0' };
	char *real_path = real_path_buf;
	int real_path_len = 0;
	
	if ( (real_path_len = readlink(file_list->full_path, 
				real_path_buf, 512)) == -1 )
	{
		perror(file_list->full_path);
		return -1;
	}
	*(real_path + real_path_len) = '\0';

	printf(" -> %s", real_path);
	return 0;
}


int show_file(Fpath *file_list, struct stat *buf, const long int mode)
{
	if ( IS_I(mode) )
	{
		printf("%lu ", (unsigned long)(buf->st_ino));
	}
	if ( IS_L(mode) )
	{
		mode_t file_mode = (*buf).st_mode;
			
		char file_type = '\0';
/*file type */
		if ( S_ISREG(file_mode) )
			file_type = '-';
		else if ( S_ISDIR(file_mode) )
			file_type = 'd';
		else if ( S_ISCHR(file_mode) )
			file_type = 'c';
		else if ( S_ISBLK(file_mode) )
			file_type = 'b';
		else if ( S_ISFIFO(file_mode) )
			file_type = 'p';
		else if ( S_ISSOCK(file_mode) )
			file_type = 's';
		else if ( S_ISLNK(file_mode) )
			file_type = 'l';
		printf("%c", file_type);
		
		char perm_buf[10] = { '-', '-', '-', '-', '-','-', '-', '-', '-','\0' };
/*file permission */
		if ( file_mode & S_IRUSR )
			perm_buf[0] = 'r';
		if ( file_mode & S_IWUSR )
			perm_buf[1] = 'w';
		if ( file_mode & S_IXUSR )
		{
			if ( file_mode & S_ISUID )
				perm_buf[2] = 's';
			else
				perm_buf[2] = 'x';
		}
		else
		{
			if ( file_mode & S_ISUID )
				perm_buf[2] = 'S';
		}
		
		if ( file_mode & S_IRGRP )
			perm_buf[3] = 'r';
		if ( file_mode & S_IWGRP )
			perm_buf[4] = 'w';
		if ( file_mode & S_IXGRP )
		{
			if ( file_mode & S_ISGID )
				perm_buf[5] = 's';
			else
				perm_buf[5] = 'x';
		}
		else
		{
			if ( file_mode & S_ISGID )
				perm_buf[5] = 'S';
		}
		
		if ( file_mode & S_IROTH )
			perm_buf[6] = 'r';
		if ( file_mode & S_IWOTH )
			perm_buf[7] = 'w';
		if ( file_mode & S_IXOTH )
		{
			if ( file_mode & S_ISVTX )
				perm_buf[8] = 't';
			else
				perm_buf[8] = 'x';
		}
		else
		{
			if ( file_mode & S_ISVTX )
				perm_buf[8] = 'T';
		}
		printf("%s  ", perm_buf);

		printf("%d  ", (*buf).st_nlink);

		struct passwd *passwd_file;
		struct group *group_file;
/* file owner and group */		
		if ((passwd_file = getpwuid((*buf).st_uid)) != NULL)
		{
			printf("%s  ", (passwd_file -> pw_name));
		}
		else
		{
			printf("%d  ", (*buf).st_uid);
		}
		if ((group_file = getgrgid((*buf).st_gid)) != NULL)
		{
			printf("%s  ", (group_file -> gr_name));
		}
		else
		{
			printf("%d  ", (*buf).st_gid);
		}

/* file size or device number */
		switch (file_type)
		{
		case '-':
		case 'd':
		case 'l':
		case 's':
		case 'p':
			printf("%8ld  ", (*buf).st_size);
			break;
		case 'c':
		case 'b':
			printf("%lu    %lu    ", (unsigned long)MAJOR(buf->st_rdev), (unsigned long)MINOR(buf->st_rdev));
			break;
		default:
			break;
		}

/* file time */
		time_t *get_time = &((*buf).st_mtime);
		char time_buf[100] = { '\0' }, *my_time = time_buf;
		int time_len = 0;
		strcpy(time_buf, ctime(get_time));
		time_len = strlen(my_time);
		*(my_time + time_len - 1) = '\0';		
		printf("%s  ", my_time);
		
		printf("%s", file_list->name);
	}
	else
	{
		printf("%s\t", file_list->name);
	}

	return 0;
}

int show_dir(Fpath *file_list, const long int mode)
{
	DIR *dir;
	struct dirent *dir_str;
	struct stat sub_buf; 
	char path_buf[512] = { '\0' }, *ppath_buf = path_buf;
	char path[512] = { '\0' }, *ppath = path;

	ppath_buf = strcpy(ppath_buf, file_list->full_path);
	
	if ( (dir = opendir(file_list->full_path)) == NULL )
	{
		perror(file_list->full_path);
		return -1;
	}
	
	if ( IS_R(mode) )
		printf("%s:\n", file_list->full_path);
	
	while ( (dir_str = readdir(dir)) != NULL )
	{
		ppath = strcpy(ppath, ppath_buf);
		if ( !IS_A(mode)  && *(dir_str->d_name) == '.' )
			continue;
		if (ppath[strlen(ppath) - 1] != '/')
			ppath = strcat(ppath, "/");
		ppath = strcat(ppath, dir_str->d_name);
		file_list->full_path = ppath;
		file_list->name = dir_str->d_name;
		if ( lstat(file_list->full_path, &sub_buf) == -1 )
		{
			perror(file_list->full_path);
			continue;
		}
		show_file(file_list, &sub_buf, mode);
		if (S_ISLNK(sub_buf.st_mode) && (IS_L(mode)))
			show_link_content(file_list, &sub_buf);
		if ( IS_L(mode) )
			printf("\n");
	}

/* traverse directory again, recursively show all sub directories */ 
	if ( IS_R(mode) )
	{
		rewinddir(dir);
		while ( (dir_str = readdir(dir)) != NULL )
		{
			ppath = strcpy(ppath, ppath_buf);
			if ( !IS_A(mode)  && *(dir_str->d_name) == '.' )
				continue;
			if (ppath[strlen(ppath) - 1] != '/')
				ppath = strcat(ppath, "/");
			ppath = strcat(ppath, dir_str->d_name);
			file_list->full_path = ppath;
			file_list->name = dir_str->d_name;
			if ( lstat(file_list->full_path, &sub_buf) == -1 )
			{
				perror(file_list->full_path);
				continue;
			}
			if ( S_ISDIR(sub_buf.st_mode)
			          && strcmp(dir_str->d_name, ".") != 0
			          && strcmp(dir_str->d_name, "..") != 0 )
			{
				printf("\n");
				show_dir(file_list, mode);
			}
		}
	}

	closedir(dir);
	return 0;
}

/*-----------------------------------------------------------------------*/
/* scan the arg list to  get the cmd line args start with the '-' */
/*long int my_get_opt(const int argc, const char *argv[])*/
int my_get_opt(const int argc, const char **argv)
{
	int i = 0;
	int opt_sub_num = 0;
	long int mode = 0;
	char ch = '\0';
	for ( i = 1; i < argc; i++)
	{
		ch = **(argv + i);

		if ( (ch  == '-') && ((opt_sub_num = strlen(*(argv + i))) > 1) )
		{	
			int j = 0;
			while ( --opt_sub_num )
			{
				switch ( *(*(argv + i) + (++j)))
				{
					case 'l': 
						mode |= L; /* L is defined in the header "myopt.h" */
						break;
					case 'd':
						mode |= D;
						break;
					case 'r':
						mode |= R;
						break;
					case 'i':
						mode |= I;
						break;
					case 'a':
						mode |= A;
						break;
					default :
						break;
				}
			}
		}
	}

	return mode;
}

/*-------------------------------------------------------------------------------*/
/* get the file list which would list by the command */
int get_file_list(const int argc, const char **argv, Fpath **file_list)
{
	int file_num = 0, i = 0, file_index = 0;
	char ch = '\0';
	for ( i = 1; i < argc; i++ )
	{	
		ch = **(argv + i);
		
		if ( ch == '-' )
		{
			continue;
		}
		file_num++;
	}
/*	printf("file_num = %d\n", file_num);*/
	if ( file_num == 0 )
	{
		file_num = 1;
		*file_list = (Fpath *) malloc(sizeof (Fpath));
		(*file_list + file_index)->full_path = ".";
		(*file_list + file_index++)->name = ".";
	}
	else
	{
		*file_list = (Fpath *) malloc(file_num * sizeof (Fpath));
	
		for ( i = 1; i < argc; i++ )
		{	
			ch = **(argv + i);
		
			if ( ch == '-' )
			{
				continue;
			}
			(*file_list + file_index)->full_path = (char *)*(argv + i);
			(*file_list + file_index++)->name = (char *)*(argv + i);
		}
	}
/*	
	for ( i = 0; i < file_index; i++ )
	{
		printf("file %d : %s\n", i, file_list[i].full_path);
	}
*/
	return file_num;	
}
