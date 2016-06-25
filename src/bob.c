#include "bob.h"

int main (int argc, char** argv) {
	
	printf("bob v%u\n", VERSION);
	
	for (int i = 1; i< argc; i++) {
		unsigned long arghash = hash(*(argv+i));
		switch(arghash) {
		
		case ARG_CH : {
		
			i++;
			ch_file = *(argv+i);
			printf("Using change file: '%s'\n", ch_file);
			break;
		
		}
		case ARG_C : {
		
			i++;
			conf_fle = *(argv+i);
			printf("Using build file: '%s'\n", conf_fle);
			break;
		
		}
		default    : {
			printf("Can't parse argument: %s\nhash: %u\n", *(argv+i), arghash);
			bob_exit(2);
			break;
		}
			
		}
	}
	
	if(conf_fle == NULL){
		printf("Null configuration file!\n");
		bob_exit(1);
	}
	config = fopen(conf_fle,"r");
	if(config == NULL){
		printf("Can't open build configuration file!\n");
		bob_exit(1);
	}
	
	inc_paths = lnew();
	lib_paths = lnew();
	libs      = lnew();
	
	//Parse config file
	printf("-CONFIG FILE BEGIN-\n");
	unsigned short comments = 0;
	unsigned short blanks   = 0;
	char line[1024];
	while(fgets(line, sizeof line, config) != NULL) {
		if(line[0] == '\n') {
			blanks++;
			continue;
		}
		if(line[0] == '#') {
			comments++;
			continue;
		}
		//Strip \n
		line[strcspn(line, "\n")] = 0;
		char *sline = strtok(line,SEPARATOR);
		char *tmp = strtok(NULL,SEPARATOR);
		switch(hash(sline)) {
			
		case SRC_PATH  : {
			src_path = (char*)malloc(strlen(tmp));
			strcpy(src_path,tmp);
			printf("Source path:        %s\n", src_path);
			break;
		}
		case MAIN_SRC  : {
			main_src = (char*)malloc(strlen(tmp));
			strcpy(main_src,tmp);
			printf("Main source:        %s\n", main_src);
			break;
		}
		case INC_PATH  : {
			
			gvalue inc_path;
			inc_path._str = (char*)malloc(strlen(tmp));
			strcpy(inc_path._str,tmp);
			
			ladd(inc_paths,gvt_new(inc_path,T_STR));
			printf("Include path:       %s\n", tmp);
			break;
		}
		case EXE_PATH  : {
			exe_path = (char*)malloc(strlen(tmp));
			strcpy(exe_path,tmp);
			printf("Executable path:    %s\n", exe_path);
			break;
		}
		case OBJ_PATH  : {
			obj_path = (char*)malloc(strlen(tmp));
			strcpy(obj_path,tmp);
			printf("Object path:        %s\n", obj_path);
			break;
		}
		case LIB_PATH  : {
			
			gvalue lib_path;
			lib_path._str = (char*)malloc(strlen(tmp));
			strcpy(lib_path._str,tmp);
			
			ladd(lib_paths,gvt_new(lib_path,T_STR));
			printf("Library path:       %s\n", tmp);
			break;
		}
		case LIB       : {
			
			gvalue lib;
			lib._str = (char*)malloc(strlen(tmp));
			strcpy(lib._str,tmp);
			
			ladd(libs,gvt_new(lib,T_STR));
			printf("Library:            %s\n", tmp);
			break;
		}
		case VCVARSALL : {
			vcvarsall = (char*)malloc(strlen(tmp));
			strcpy(vcvarsall,tmp);
			printf("vcvarsall:          %s\n", tmp);
			break;
		}
		default : {
			printf("Syntax error, hash: %u\n", hash(sline));
			break;
		}
			
		}
	}
	printf("--CONFIG FILE END--\nComments:    %u\nBlank lines: %u\n", comments, blanks);
	
	if( main_src == NULL ||
	    src_path == NULL ||
		inc_paths->size == 0 ||
		exe_path == NULL ||
		obj_path == NULL) {

		printf("VARIABLE NOT SET IN BUILD FILE!.\n");
		bob_exit(1);
		
	}
	
	struct _stat buf;
	if(_stat(src_path,&buf) == 0) { //Exists
		if (buf.st_mode & _S_IFDIR) {
			printf("'%s' found.\n", src_path);
		} else {
			printf("'%s' is not a directory.\n", src_path);
			bob_exit(1);
		}
	} else {
		printf("'%s' not found.\n", src_path);
		bob_exit(1);
	}
	
	list_t *sources = bob_sources(src_path);
	lprint(sources);
	
	char *compile_comand = "cl /EHsc /MD /c ";
	compile_comand = bob_strcat(compile_comand, main_src);
	
	if(_stat(ch_file,&buf) == 0) {
		printf("Change file exists!\n");
		if(ch_file == NULL){
			printf("Null change file!\n");
			bob_exit(1);
		}
		FILE *chfile = fopen(ch_file,"r");
		if(chfile == NULL){
			printf("Can't open change file!\n");
			bob_exit(1);
		}
		while(fgets(line, sizeof line, config) != NULL) {
			char *filename = strtok(line,SEPARATOR);
			char *filehash = strtok(NULL,SEPARATOR);
		}
	} else {
		printf("Creating change file...\n");
	}
	
	printf("%zu\n", sizeof(gvalue));
	
	free(compile_comand);
	
	bob_exit(0);
	
}

list_t* bob_sources(const char *path) {
	
	DIR *dp;
	struct dirent *ep;
	list_t *src_list = lnew();
	
	dp = opendir(path);
	if (dp == NULL)
		return NULL;
	
	while (ep = readdir(dp)) {
		bool is_src = false;
		for(int i = 0; i < 3; i++){
			char *ext = strrchr(ep->d_name,'.');
			if(ext && strcmp(ext,SOURCE_EXTENSIONS[i]) == 0){
				is_src = true;
				break;
			}
		}
		if(is_src) {
			gvalue filename;
			filename._str = (char*)malloc(strlen(ep->d_name));
			strcpy(filename._str,ep->d_name);
			ladd(src_list,gvt_new(filename,T_STR));
		}
	}
		
	closedir(dp);
	
	return src_list;
	
}

char* bob_strcat(const char *str1, const char *str2) {
	
	unsigned short len1 = strlen(str1);
	unsigned short len2 = strlen(str2);
	
	char *newstr = (char*)malloc(len1 + len2 + 1);
	strcpy(newstr, str1);
	strcat(newstr, str2);
	
	return newstr;
	
}

void bob_exit(int exitcode) {
	if(config)
		fclose(config);
	
	if(main_src)
		free(main_src);
	if(src_path)
		free(src_path);
	if(exe_path)
		free(exe_path);
	if(obj_path)
		free(obj_path);
	
	if(inc_paths)
		lfree(inc_paths);
	if(lib_paths)
		lfree(lib_paths);
	if(libs)
		lfree(libs);
	
	system("PAUSE");
	exit(exitcode);
}