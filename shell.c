#include<stdio.h>	//Includes all standard I/O operations
#include<stdlib.h>	//Defines four variable types, several macros, and various functions for performing general functions Eg: malloc, exit, getenv, setenv, atoi
#include<string.h>	//Defines one variable type, one macro, and various functions for manipulating arrays of characters Eg: strcpy, strcat, strlen, strtok, strdup
#include<ctype.h>	//To check for white-space character using 'isspace()'
#include<fcntl.h>	//Includes file control options Eg: open, creat, used for history log
#include<unistd.h>	//Code's entry point to various constant, type and function declarations that comprise the POSIX operating system API Eg: read, write, close, chdir, getcwd, pipe, fork, execvp
#include<sys/types.h>	//Includes data types Eg: pid_t (represents process ids)
#include<sys/wait.h>	//Declarations for waiting Eg: signal, waitpid
#include<errno.h>	//Defines macros for reporting and retrieving error conditions using the symbol errno

//Global variable declarations
int flag_pipe=1, count_pipe=0, fd;
static char* args[512];
char *history_file;
char input_buffer[1024];
char *cmd_exec[100];
int flag, len;
char cwd[1024];
int no_of_lines;
int environment_flag;
int flag_pipe, flag_without_pipe,  output_redirect, input_redirect;
int bang_flag;
int pid, status;
char history_data[1000][1000];
char current_directory[1000];
char ret_file[3000];
char his_var[2000];
char *input_redirection_file;
char *output_redirection_file;
extern char** environ;
pid_t pid;

//Functions declarations
void clear_variables();
void fileprocess ();
void file_write();
void bang_execute();
void environment();
void set_environment_variables();
void change_directory();
void parent_directory();
void echo_calling(char *echo_val);
void history_execute_with_constants();
static char* skipwhite(char* s);
void tokenise_commands(char *com_exec);
void tokenise_redirect_input_output(char *cmd_exec);
void tokenise_redirect_input(char *cmd_exec);
void tokenise_redirect_output(char *cmd_exec);
char* skip_double_quote(char* str);
static int split(char *cmd_exec, int, int, int);
void with_pipe_execute();
static int command(int, int, int, char *cmd_exec);
void prompt();
void sigintHandler(int sig_num);


void sigintHandler(int sig_num)
{
	signal(SIGINT, sigintHandler);
	fflush(stdout);
}

void clear_variables()	//Initalize all variables to 0 or '\0'
{
	fd =0;
	flag=0;
	len=0;
	no_of_lines=0;
	count_pipe=0;
	flag_pipe=0;
	flag_without_pipe=0;
	output_redirect=0;
	input_redirect=0;
	input_buffer[0]='\0';
	cwd[0] = '\0';
	pid=0;
	environment_flag=0;
	bang_flag=0;
}
  
void fileprocess ()
{
	int fd;
	history_file=(char *)malloc(100*sizeof(char));
	strcpy(history_file,current_directory);
	strcat(history_file, "/");
	strcat(history_file, "history.txt");
	fd=open(history_file, O_RDONLY|O_CREAT,S_IRUSR|S_IWUSR);
	int bytes_read=0, i=0, x=0, index=0;
	char buffer[1], temp_data[1000];
	do
	{
		bytes_read = read (fd, buffer, sizeof (buffer));
		for (i=0; i<bytes_read; ++i)
		{
			temp_data[index]=buffer[i];
			index++;
			if(buffer[i]=='\n')
			{
				temp_data[index-1]='\0';
				no_of_lines++;
				index=0;
				strcpy(history_data[x], temp_data);
				x++;
				temp_data[0]='\0';
			}
		}
	}
	while (bytes_read == sizeof(buffer));
	close (fd);
}

void file_write()
{
	int fd_out,ret_write,str_len=0;
	char input_data[2000];
	no_of_lines++;
	char no[10];
	sprintf (no, "%d", no_of_lines );	//sprintf converts integer to string(here int no_of_lines is converted to char no)
	strcpy(input_data, " ");
	strcat(input_data, no);
	strcat(input_data, " ");
	strcat(input_data, input_buffer);

	str_len = strlen(input_data);
	fd_out=open(history_file,O_WRONLY|O_APPEND|O_CREAT,S_IRUSR|S_IWUSR);
	len=strlen(input_buffer);
	ret_write=write(fd_out,input_data,str_len);
	if(ret_write<0) 
	{
		printf("Error in writing file\n");
		return;
	}
	close(fd_out);
}

void bang_execute()
{
	char bang_val[1000];
	char *tokenise_bang[100], *num_ch[10];
	int i=1, n=1, num, index=0;
	if(input_buffer[i]=='!')
	{
		strcpy(bang_val, history_data[no_of_lines-1]);
	}
	else
	if(input_buffer[i]=='-')
	{
		n=1;
		num_ch[0]=strtok(input_buffer,"-");	//strtok() function is used to split a string into a series of tokens based on a particular delimiter in ""
		while((num_ch[n]=strtok(NULL,"-"))!=NULL)
			n++;
		num_ch[n]=NULL;
		num = atoi(num_ch[1]);	//atoi(str): converts str to an integer
		index = no_of_lines-num;
		strcpy(bang_val, history_data[index]);
	}
	else 
	{
		num_ch[0]=strtok(input_buffer,"!");
		num = atoi(num_ch[0]);
		strcpy(bang_val, history_data[num-1]);
	}
	tokenise_bang[0]=strtok(bang_val," ");
	while((tokenise_bang[n]=strtok(NULL,""))!=NULL)
		n++;
	tokenise_bang[n]=NULL;
	strcpy(bang_val, tokenise_bang[1]);
	printf("%s\n", bang_val);
	strcpy(input_buffer, bang_val);
}

void environment()	//For echo command with environment variables precided by '$'
{
	int i =1, index=0;
	char env_val[1000], *value;
	while(args[1][i]!='\0')
	{
		env_val[index]=args[1][i];
		index++;
		i++;
	}
	env_val[index]='\0';
	value=getenv(env_val);

	if(!value)
		printf("\n");
	else 
		printf("%s\n", value);
}

void set_environment_variables()	//For export command
{  
	int n=1;
	char *left_right[100];
	if(args[1]==NULL)
	{
		char** env;
		for (env = environ; *env != 0; env++)
		{
			char* value = *env;
			printf("declare -x %s\n", value);    
		}  
		return; 
	}
	left_right[0]=strtok(args[1],"=");
	while((left_right[n]=strtok(NULL,"="))!=NULL)
      		n++;
	left_right[n]=NULL;
	setenv(left_right[0], left_right[1], 0);
}

void change_directory()	//For cd command
{
	char *h="/home";   
	if(args[1]==NULL)
		chdir(h);
	else
	if ((strcmp(args[1], "~")==0) || (strcmp(args[1], "~/")==0))
		chdir(h);
	else
	if(chdir(args[1])<0)
		printf("bash: cd: %s: No such file or directory\n", args[1]);
}

void parent_directory()	//For pwd command
{
	if (getcwd(cwd, sizeof(cwd)) != NULL)
	{
		printf("%s\n", cwd );
	}
	else
		perror("getcwd() error");
}

void echo_calling(char *echo_val)	//For echo command
{
	int i=0, index=0;
	environment_flag=0;
	char new_args[1024],env_val[1000], *str[10];
	str[0]=strtok(echo_val," ");
	str[1]=strtok(NULL, "");
	strcpy(env_val, args[1]);
	if(str[1]==NULL)
	{
		printf("\n");
		return;
	}
	if(strchr(str[1], '$'))
		environment_flag=1;

	memset(new_args, '\0', sizeof(new_args));
	i=0;
	while(str[1][i]!='\0')
	{
		if(str[1][i]=='"')
		{
			index=0;     
			while(str[1][i]!='\0')
			{
				if(str[1][i]!='"')
				{
					new_args[index]=str[1][i];
					flag=1;
					index++;
				}
				i++;
			}             
		}
		else
		if(str[1][i]==39)	//39 is ASCII value for single quote(')
		{
			index=0;
			while(str[1][i]!='\0')
			{
				if(str[1][i]!=39)
				{
					new_args[index]=str[1][i];
					flag=1;
					index++;
				}
				i++;
			}
		}
		else
		if(str[1][i]!='"')
		{
			new_args[index]=str[1][i];
			index++;
			i++;
		}
		else
			i++;    
	}

	new_args[index]='\0';
	if((strcmp(args[1], new_args)==0)&&(environment_flag==0))
		printf("%s\n", new_args);
	else
	{
		strcpy(args[1], new_args);
		if(environment_flag==1)
			environment();

		else
		if(environment_flag==0)
			printf("%s\n", new_args ); 
	}  
}

void history_execute_with_constants()	//For history command
{
	int num, i, start_index;
	if(bang_flag==1)
	{
		for(i=0; i<no_of_lines; i++)
		printf("%s\n", history_data[i]);
	}
	else
	if(args[1]==NULL)
	{
		for(i=0; i<no_of_lines-1; i++)
			printf("%s\n", history_data[i]);
		printf(" %d %s\n", no_of_lines, his_var);
	}
	else
	{
		if(args[1]!=NULL)
			num = atoi(args[1]);
		if(num>no_of_lines)
		{
			for(i=0; i<no_of_lines-1; i++)
				printf("%s\n", history_data[i]);
			printf(" %d %s\n", no_of_lines, his_var);
		}
		start_index=no_of_lines-num;
		for(i=start_index; i<no_of_lines-1; i++)
			printf("%s\n", history_data[i] );
		printf(" %d %s\n", no_of_lines, his_var );
	}
}

static char* skipwhite(char* s)
{
	while(isspace(*s))
		++s;
	return s;
}

void tokenise_commands(char *com_exec)
{
	int m=1;
	args[0]=strtok(com_exec," ");       
	while((args[m]=strtok(NULL," "))!=NULL)
		m++;
}

void tokenise_redirect_input_output(char *cmd_exec)	//For input and output redirection
{
	char *io_token[100];
	char *new_cmd_exec1;  
	new_cmd_exec1=strdup(cmd_exec);	//strdup: duplicates string(cmd_exec) and returns pointer to the copied string(new_cmd_exec1)
	int m=1;
	io_token[0]=strtok(new_cmd_exec1,"<");       
	while((io_token[m]=strtok(NULL,">"))!=NULL)
		m++;
	io_token[1]=skipwhite(io_token[1]);
	io_token[2]=skipwhite(io_token[2]);
	input_redirection_file=strdup(io_token[1]);
	output_redirection_file=strdup(io_token[2]);
	tokenise_commands(io_token[0]);
}

void tokenise_redirect_input(char *cmd_exec)	//For input redirection
{
	char *i_token[100];
	char *new_cmd_exec1;  
	new_cmd_exec1=strdup(cmd_exec);
	int m=1;
	i_token[0]=strtok(new_cmd_exec1,"<");       
	while((i_token[m]=strtok(NULL,"<"))!=NULL)
		m++;
	i_token[1]=skipwhite(i_token[1]);
	input_redirection_file=strdup(i_token[1]);
	tokenise_commands(i_token[0]);
}

void tokenise_redirect_output(char *cmd_exec)	//For output redirection
{
	char *o_token[100];
	char *new_cmd_exec1;
	new_cmd_exec1=strdup(cmd_exec);
	int m=1;
	o_token[0]=strtok(new_cmd_exec1,">");
	while((o_token[m]=strtok(NULL,">"))!=NULL)
		m++;
	o_token[1]=skipwhite(o_token[1]);
	output_redirection_file=strdup(o_token[1]);
	tokenise_commands(o_token[0]);
}

char* skip_double_quote(char* str)
{
	int i=0, j=0;
	char temp[1000];
	while(str[i++]!='\0')
	{
		if(str[i-1]!='"')
		temp[j++]=str[i-1];
	}
	temp[j]='\0';
	str = strdup(temp);
	return str;
}

static int split(char *cmd_exec, int input, int first, int last)
{
	char *new_cmd_exec1;  
	new_cmd_exec1=strdup(cmd_exec);

	{
		int m=1;
		args[0]=strtok(cmd_exec," ");       
		while((args[m]=strtok(NULL," "))!=NULL)
			m++;
		args[m]=NULL;
		if(args[0] != NULL) 
		{
			if(strcmp(args[0], "exit") == 0)	//Check for exit command
				exit(0);
			if(strcmp(args[0], "echo") != 0)	//check for echo command
			{
				cmd_exec = skip_double_quote(new_cmd_exec1);
				int m=1;
				args[0]=strtok(cmd_exec," ");       
				while((args[m]=strtok(NULL," "))!=NULL)
					m++;
				args[m]=NULL;
			}
			if(strcmp("cd",args[0])==0)	//Check for cd command
			{
				change_directory();
				return 1;
			}
			else
			if(strcmp("pwd",args[0])==0)	//Check for pwd command
			{
				parent_directory();
				return 1;
			}
		}
	}
	return command(input, first, last, new_cmd_exec1);
}

void with_pipe_execute()	//For pipe '|' command
{
	int i, n=1, input=0, first=1;

	cmd_exec[0]=strtok(input_buffer,"|");
	while((cmd_exec[n]=strtok(NULL,"|"))!=NULL)
		n++;
	cmd_exec[n]=NULL;
	count_pipe=n-1;
	for(i=0; i<n-1; i++)
	{
		input = split(cmd_exec[i], input, first, 0);
		first=0;
	}
	input=split(cmd_exec[i], input, first, 1);
	input=0;
	return;
}

static int command(int input, int first, int last, char *cmd_exec)
{
	int mypipefd[2], ret, input_fd, output_fd;
	ret = pipe(mypipefd);	//pipe(): create pipe, mypipefd[0] is the read pipe-end and mypipefd[1] is the write pipe-end
	if(ret == -1)
	{
		perror("pipe");
		return 1;
	}
	pid = fork();

	if (pid == 0) 
	{
		if (first==1 && last==0 && input==0) 
		{
			dup2( mypipefd[1], 1 );	//dup2(): duplicate open file descriptor onto another file descriptor, uses the descriptor number specified by the user(unlike dup() which uses the lowest-numbered unused file-descriptor). Here mypipefd[1] is the oldfd and 1 is the newfd(new file-descriptor), newfd is the new file descriptor which is used by dup2() to create a copy
		} 
		else
		if (first==0 && last==0 && input!=0) 
		{
			dup2(input, 0);
			dup2(mypipefd[1], 1);
		} 
		else 
		{
			dup2(input, 0);
		}
		if (strchr(cmd_exec, '<') && strchr(cmd_exec, '>'))	//Check for both input and oputput redirections
		{
			input_redirect=1;
			output_redirect=1;
			tokenise_redirect_input_output(cmd_exec);
		}
		else
		if (strchr(cmd_exec, '<'))	//Check for input redirection
		{
			input_redirect=1;
			tokenise_redirect_input(cmd_exec);
		}
		else
		if (strchr(cmd_exec, '>'))	//Check for output redirection
		{
			output_redirect=1;
			tokenise_redirect_output(cmd_exec);
		}
		if(output_redirect == 1)	//On output redirection
		{                    
			output_fd= creat(output_redirection_file, 0644);	//creat(): create a new file or rewrite an existing one, output_redirection_file is the file-name and 0644 are the permissions of new file
			if (output_fd < 0)
			{
				fprintf(stderr, "Failed to open %s for writing\n", output_redirection_file);
				return(EXIT_FAILURE);
			}
			dup2(output_fd, 1);
			close(output_fd);	//close(): closes file
			output_redirect=0;
		}
		if(input_redirect == 1)		//On input redirection
		{
			input_fd=open(input_redirection_file,O_RDONLY, 0);	//open(): opens file, 1st parameter- file name, 2nd parameter- flag(s), 3rd parameter- mode
			if(input_fd < 0)
			{
				fprintf(stderr, "Failed to open %s for reading\n", input_redirection_file);
				return(EXIT_FAILURE);
			}
			dup2(input_fd, 0);
			close(input_fd);
			input_redirect=0;
		}
		if(strcmp(args[0], "export") == 0)	//Check for export command
		{
			set_environment_variables();
			return 1;
		}
		if (strcmp(args[0], "echo") == 0)	//Check for echo command
		{
			echo_calling(cmd_exec);
		} 
		else
		if (strcmp(args[0], "history") == 0)	//Check for history command
		{
			history_execute_with_constants();
		} 
		else
		if(execvp(args[0], args)<0) printf("%s: command not found\n", args[0]);	//If wrong command is entered
			exit(0);
	}
	else
	{
		waitpid(pid, 0, 0);
	}
	if (last == 1)
		close(mypipefd[0]);
	if (input != 0) 
		close(input);
	close(mypipefd[1]);
	return mypipefd[0];
}

void prompt()	//For commmand prompt
{
	char shell[1000];
	if(getcwd(cwd, sizeof(cwd)) != NULL)	//getcwd(): get pathname of current working directory
	{
		strcpy(shell, "My_shell:");
		strcat(shell, cwd);
		strcat(shell, "$ ");
		printf("%s", shell);
	}
	else
		perror("getcwd() error");
}

int main()
{

	int status;
	char ch[2]={"\n"};
	getcwd(current_directory, sizeof(current_directory));  //Get the PATHNAME for current working directory
	signal(SIGINT, sigintHandler);	//Signal handling
	while (1)
	{
		clear_variables();
		prompt();
		fgets(input_buffer, 1024, stdin);	//Reading a string value with spaces
		if(strcmp(input_buffer, ch)==0)
		{
			continue;
		}
		if(input_buffer[0]!='!')
		{
			fileprocess();
			file_write();
		}
		len = strlen(input_buffer);
		input_buffer[len-1]='\0';
		strcpy(his_var, input_buffer);
		if(strcmp(input_buffer, "exit") == 0)	//Check for exit command
		{
			flag = 1;
			break;
		}
		if(input_buffer[0]=='!')  
		{
			fileprocess();
			bang_flag=1;
			bang_execute();
		}
		with_pipe_execute();
		waitpid(pid,&status,0);
	}
	if(flag==1)	//For exit command
	{
		printf("Bye...\n");
		exit(0);
		return 0;
	}
	return 0;
}
