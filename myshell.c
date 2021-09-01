/********************************
This is a template for assignment on writing a custom Shell. 

Students may change the return types and arguments of the functions given in this template,
but do not change the names of these functions.

Though use of any extra functions is not recommended, students may use new functions if they need to, 
but that should not make code unnecessorily complex to read.

Students should keep names of declared variable (and any new functions) self explanatory,
and add proper comments for every logical step.

Students need to be careful while forking a new process (no unnecessory process creations) 
or while inserting the single handler code (should be added at the correct places).

Finally, keep your filename as myshell.c, do not change this name (not even myshell.cpp, 
as you not need to use any features for this assignment that are supported by C++ but not by C).
*******************************/
//ENROLL - BT19CSE012  NAME - BARTAKKE ONKAR SUHAS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>			// exit()
#include <unistd.h>			// fork(), getpid(), exec()
#include <sys/wait.h>		// wait()
#include <signal.h>			// signal()
#include <fcntl.h>			// close(), open()
#include <ctype.h>

int parseInput(char *Input_cmd)
{
	// This function will parse the input string into multiple commands or a single command with arguments depending on the delimiter (&&, ##, >, or spaces).
    int ret_val;
    char *exit_cmd = "exit\n";
    if(strcmp(Input_cmd,exit_cmd)==0) //If the command is exit command return 0
    {
        ret_val = 0;
        return ret_val;
    }


    char *copy_cmd;
	copy_cmd = strdup(Input_cmd); 

    /*create a duplicate of our command as we are using strsep on input command it will change
    pointer to the next character after the passed delimeter*/

    copy_cmd = strsep(&Input_cmd,"\n"); 	//remove new line character from the string
    // In copy of command we have the actual entered command without new line character
	/*-----------------------------Parsing Logic-----------------------------------------------------*/

	if(strstr(copy_cmd, "&&") != NULL) // set ret_val=1 for parallel execution of multiple commands 
	{
		ret_val = 1;       			        
        return ret_val;
	}
	else if(strstr(copy_cmd, "##") != NULL) // set ret_val=2 for sequential execution of multiple commands
	{
		ret_val = 2;
        return ret_val;					
	}
	else if(strstr(copy_cmd, ">") != NULL) // set ret_val=3 for command redirection
	{
		ret_val = 3;
        return ret_val;							
	}
	else
	{
		ret_val = 4;
        return ret_val;							//setret_vals=4 for  single command execution
	}
	return ret_val;

}

char** parseSingleCmd(char *cmd)
{
    char *copy_of_cmd;
    char *delimeter = " ";

    cmd = strsep(&cmd,"\n"); // Removing new line character

    while(*(cmd) == ' ') //Removing White spaces from start
    {
        cmd++;
    }

    copy_of_cmd = strdup(cmd); //Copied the command after removing white spaces from start

    char *end; 
    // pointer to point the end of the copied command
    end = copy_of_cmd + strlen(copy_of_cmd)-1; 

    while(end > copy_of_cmd && (*end) == ' ') //Removing white spaces from end
    {
        end--;
    }

    end[1] = '\0'; //Terminating the command just after last non-whitespace character
    //printf("\n***** cmd after triming id  = %s+end",copy_of_cmd);
    char *first_token;

    char *tokens = strsep(&copy_of_cmd, delimeter);
    //Here we will get first token, the part of the command just before first white-space


    //first_token = strsep(&copy_of_cmd, delimeter);

    if(strcmp(tokens,"cd") == 0) //If that first part is equal to cd, it's special case, we need to change the directory
    {
        char *location = strsep(&copy_of_cmd,"\n"); //Location where we want to change directory
        chdir(location); //command to change directory
        return NULL;
    }
    else
    {
        size_t buff,iterator; //size_t is an unsigned integral data type, used to declare sizes of host files in c
        //Buff => buffer , to declare sizes of arguments

        buff = 50;
        char **arguments = (char**)malloc(buff*sizeof(char*));

        for(iterator = 0; iterator < buff ; iterator++)
        {
            arguments[iterator] = (char*)malloc(buff*sizeof(char));
        }

        //char *tokens;
        //tokens = strsep(&cmd,delimeter);
        iterator = 0;
        /*We already have extracted our 1st token while checking for 'cd' on line number 102
        so, we already have it in our tokens variable, we will just remove new-line char if present from
        it and store that token in arguments.
        As we know when we use strsep, now copy_of_cmd will be pointing to the next character after delimeter,
        so that's what happens in while loop, will go on till extracted token becomes null
        */
        while(tokens)
        {
            tokens = strsep(&tokens,"\n");
            arguments[iterator] = tokens;
            iterator++;
            tokens = strsep(&copy_of_cmd,delimeter);
        }

        arguments[iterator] = NULL;

        return arguments;
    }
}


void executeCommand(char *cmd)
{
	// This function will fork a new process to execute a command
    char** cmd_args = parseSingleCmd(cmd);
	int rc;
	rc = fork();

    if(rc < 0)  // fork failed; exit
    {
        exit(0);
    }
    else if(rc == 0)
    {
        // EXEC system call

        //we have to enable signals again for child processes
		signal(SIGTSTP, SIG_DFL); 
        signal(SIGINT, SIG_DFL);
        /*SIGINT is the interrupt signal (ctrl+C). Its default behaviour is to terminate the process.
        SIGINT signal can be dis-positioned, means one can change the default behaviour 
        ( By calling sighandler, or setting it SIG_IGN ) Now once the action is changes and you want to set it
        again the default behaviour of this signal then you should write*/

        int ret_val = execvp(cmd_args[0],cmd_args);

        if(ret_val < 0) //execvp error code
        {
            printf("Shell: Incorrect command\n");
            exit(1);
        }
    }
    else
    {
        int wait_rc = wait(NULL);
    }
}

void executeParallelCommands(char *cmd)
{
	// This function will run multiple commands in parallel
    int cmd_count=0,i=0;
    //printf("\n\nInside parallel cmds\n\n");

    //To get count of parallel commands separated by &&
    while(1)
    {
        if(cmd[i]=='&' && cmd[i+1]=='&')
        {
            cmd_count++;
            i++;
        }
        else if(cmd[i]=='\0')
        {
            break; // End of command so break
        }

        i++;
    }

    char *cmd_seprator = "&&";
    char *cmd_ptr = strstr(cmd,cmd_seprator);
    //command pointer will point to the string staring from &&-- as we have used strstr which returns the 
    //first pointer of the charecter of matched substring, if present in main string

    char **cmd_container = (char**)malloc((cmd_count+2)*sizeof(char*));

    i=0;
    while(i<=cmd_count)
    {
        cmd_container[i] = (char*)malloc(150*sizeof(char));
        i++;
    }
    i=0;
    while(cmd_ptr!=NULL)
    {
        cmd_ptr[0] = '\0';          //replace & with \0 to get first command before the delimiter
        char *temp = strdup(cmd);   //temp stores the individual command
        cmd_container[i] = temp;    //stores that command in temp in commands_container
        cmd_ptr[0] = ' ';           //delete the '\0' so we can move forward in the string
        cmd = cmd_ptr + 2;          //move to next command after delimiter
        cmd_ptr = strstr(cmd,cmd_seprator);
        i++;
    }

    cmd_container[i] = cmd;
    //printf("\n\nStarting execution\n\n");
    for(i=0;i<=cmd_count;)
    {
        int rc = fork();  //making child process

        if(rc < 0)    // fork failed; exit
        {
            exit(0);
        }   
        else if(rc == 0)
        {
            //we have to enable signals again for child processes
            signal(SIGINT, SIG_DFL);
			signal(SIGTSTP, SIG_DFL);

            char **cmd_args = parseSingleCmd(cmd_container[i]);

            if(cmd_args == NULL)
            {
                exit(0);
            }

            int ret_val = execvp(cmd_args[0],cmd_args);

            if(ret_val < 0)  //execvp error code
            {
               printf("Shell: Incorrect command\n");
			   exit(1); 
            }
        }
        else
        {
            //no wait statement here as it needs to run parallely
			//parent process
			i++;
        }
    }
}

void executeSequentialCommands(char *cmd)
{	
	// This function will run multiple commands in parallel

    int i,cmd_count=0;
    //Counting number of sequential commands are t be executed separated by ##
    for(i=0;cmd[i]!='\0';i++)
    {
        if(cmd[i] == '#')
        {
            cmd_count++;
            i++;
        }
    }
    //Now seperate all individual commands by delimiter ## and store it in cmd_container for furture execution
    char **cmd_container = (char**)malloc((cmd_count+2)*sizeof(char*));

    i=0;
    while(i<=cmd_count)
    {
        cmd_container[i] = (char*)malloc((50)*sizeof(char));
        i++;
    }

    i=0;
    char *delimeter = "##";
    char *cmd_ptr = strstr(cmd,delimeter); //return the pointer to the presence of the first delimiter

    while(cmd_ptr != NULL ) //Go through all commands
    {
        cmd_ptr[0] = '\0';          //replace & with \0 to get first command before the delimiter
        char *temp = strdup(cmd);   //temp stores the individual command
        cmd_container[i] = temp;    	//stores that command in temp in command_container
        i++;
        cmd_ptr[0] = ' ';           //Replace the '\0' so we can move forward in the string
        cmd = cmd_ptr +2 ;
        cmd_ptr = strstr(cmd,delimeter);//move to next command after delimiter
    }

    cmd_container[i] = cmd;


    i=0;
    //Now Execute each command present in cmd_container as a single command by passing it to   executeCommand(copy_cmd);
    // executeCommand(copy_cmd) this will fork a child proccess and make parent wait until, child is over, after that
    // parent also ends and control is back to this function and 'i' is incremented for next command to be executed
    while(i<=cmd_count)
    {
        char *copy_cmd = strdup(cmd_container[i]);
        executeCommand(copy_cmd);
        i++;
    }
 
}


void executeCommandRedirection(char *cmd)
{
    // This function will run a single command with output redirected to an output file specificed by user
    char *command = strsep(&cmd,">");
    /*strsep will return ptr to the 1st part of the command before > , which is our actual command to
    be executed, and now, cmd will be pointing to the part which is just after delimeter '>', which is out file name*/

    char *filename = cmd;
    while(*filename == ' ') //Removing whitespaces if present
    {
        filename++;
    }

    int rc = fork(); //Forking child process
    if(rc < 0) // In case fork fails coz of lack of memory
    {
        exit(0);
    }
    else if(rc == 0) //Child process
    {
        close(STDOUT_FILENO);  //closing the STDOUT

	    
        //opening the file where we want to write the output
        open(strsep(&filename,"\n"),O_CREAT | O_RDWR, S_IRWXU); 

        char **arguments = parseSingleCmd(command);

        int retval = execvp(arguments[0],arguments);

        if (retval < 0)
		{ //execvp error code
			printf("Shell: Incorrect command\n");
			exit(1);
		}
        
    }
    else
    {
        int waiting_rc = wait(NULL); //waiting for child to be over
    }

}

int main()
{
	// Initial declarations
    signal(SIGINT, SIG_IGN);	// Ignore SIGINT signal
	signal(SIGTSTP, SIG_IGN);	//disable signals so shell will only terminate by exit command
    char working_directory[100];
    size_t buffer_size = 150; //size_t is an unsigned integral data type, used to declare sizes of host files in c
    char *Input_command;
	
	while(1)	// This loop will keep your shell running until user exits.
	{
		// Print the prompt in format - currentWorkingDirectory$
		if(getcwd(working_directory,100)!=NULL) //To get working directory
        {
            printf("%s$",working_directory);
        }
        else
        {
            printf("Error...\n");
            break;
        }
		// accept input with 'getline()'
        Input_command = (char*)malloc(buffer_size * sizeof(char));
        size_t characters;

        //Accept the input with getline
		characters = getline(&Input_command,&buffer_size,stdin);


		// Parse input with 'strsep()' for different symbols (&&, ##, >) and for spaces.
		int ret_val = parseInput(Input_command); 	

		/*printf("\n\n** ret_val = %d **\n\n",ret_val)*/

    
		if(ret_val == 0)	// When user uses exit command.
		{
			printf("Exiting shell...\n");
			break;
		}		
		else if(ret_val == 1)
			executeParallelCommands(Input_command);		// This function is invoked when user wants to run multiple commands in parallel (commands separated by &&)
		else if(ret_val == 2)
			executeSequentialCommands(Input_command);	// This function is invoked when user wants to run multiple commands sequentially (commands separated by ##)
		else if(ret_val == 3)
			executeCommandRedirection(Input_command);	// This function is invoked when user wants redirect output of a single command to and output file specificed by user
		else
			executeCommand(Input_command);		// This function is invoked when user wants to run a single commands
				
	}
	
	return 0;
}