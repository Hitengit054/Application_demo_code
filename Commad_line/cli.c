#include <stdio.h>
#include <stdint.h>
#include <pthread.h> 
#include <unistd.h>
#include <pthread.h>
#include "cli.h"
#include <string.h>


/*-------------------------------------------------------------------------
 * Type Declarations
 *-----------------------------------------------------------------------*/

QCLI_Context_t QCLI_Context;
pthread_mutex_t lock;

static QCLI_Command_Status_t Command_Ver(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t Command_Help(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t Command_Exit(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t Command_Up(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t Command_Root(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t platform_demo_reset(uint32_t parameters_count, QCLI_Parameter_t * parameters);
static void Display_Usage(uint32_t Command_Index, const QCLI_Command_t *Command);

static void Display_Command_List(const Group_List_Entry_t *Group_List_Entry);

const QCLI_Command_t Root_Command_List[] =
{
    { Command_Ver,  false, "Ver",  "",                     "Display Build Info" },
    { Command_Help, false, "Help", "[Command (optional)]", "Display Command list or usage for a command" },
    { Command_Exit, false, "Exit", "[Restart (1=Yes)]",    "Exits the application." },
    { platform_demo_reset, false, "Reset", "\n", "reset the platform\n" },
};

#define ROOT_COMMAND_LIST_SIZE                        (sizeof(Root_Command_List) / sizeof(QCLI_Command_t))

/* The following represents the list of global commands that are supported when
   in a group. */
const QCLI_Command_t Common_Command_List[] =
{
    { Command_Ver,  false, "Ver",  "",                     "Display Build Info" },
    { Command_Help, false, "Help", "[Command (optional)]", "Display Command list or usage for a command" },
    { Command_Up,   false, "Up",   "",                     "Exit command group (move to parent group)" },
    { Command_Root, false, "Root", "",                     "Move to top-level group list" },

    { platform_demo_reset, false, "Reset", "\n", "reset the platform\n" },
};

#define COMMON_COMMAND_LIST_SIZE                      (sizeof(Common_Command_List) / sizeof(QCLI_Command_t))


/*
 * @brief This function does a case-insensitive comparison of two buffers.
 *
 * @param Source1 is the first string to be compared.
 * @param Source2 is the second string to be compared.
 *
 * @return
 * - 0  if the two strings are equivalent up to the specified size.
 * - -1 if Source1 is "less than" Source2.
 * - 1  if Source1 is "greater than" Source2.
 */
int32_t Memcmpi(const void *Source1, const void *Source2, uint32_t size)
{
    int32_t  Ret_Val;
    uint8_t  Byte1;
    uint8_t  Byte2;
    uint32_t Index;

    Ret_Val = 0;

    /* Simply loop through each byte pointed to by each pointer and check to see
       if they are equal. */
    for(Index = 0; (Index < size) && (!Ret_Val); Index ++)
    {
        /* Note each Byte that we are going to compare. */
        Byte1 = ((uint8_t *)Source1)[Index];
        Byte2 = ((uint8_t *)Source2)[Index];

        /* If the Byte in the first array is lower case, go ahead and make it
           upper case (for comparisons below). */
        if((Byte1 >= 'a') && (Byte1 <= 'z'))
        {
            Byte1 = Byte1 - ('a' - 'A');
        }

        /* If the Byte in the second array is lower case, go ahead and make it
           upper case (for comparisons below). */
        if((Byte2 >= 'a') && (Byte2 <= 'z'))
        {
            Byte2 = Byte2 - ('a' - 'A');
        }

        /* If the two Bytes are equal then there is nothing to do. */
        if(Byte1 != Byte2)
        {
            /* Bytes are not equal, so set the return value accordingly. */
            if(Byte1 < Byte2)
            {
                Ret_Val = -1;
            }
            else
            {
                Ret_Val = 1;
            }
        }
    }

    /* Simply return the result of the above comparison(s). */
    return(Ret_Val);
}


/*
 * @brief This function searches the command and/or group lists for a match to the provided parameter.
 *
 * @param Group_List_Entry is the group to search.
 * @param Command_Parameter is the paramter to search for.
 * @param Find_Result is a pointer to where the found entry will be stored if successful (i.e., true was returned).
 *
 * @return
 * - true if a matching command or group was found in the list.
 * - false if the command or group was not found.
 */
static int Find_Command(Group_List_Entry_t *Group_List_Entry, QCLI_Parameter_t *Command_Parameter,
        Find_Result_t *Find_Result)
{
    int                   Ret_Val;
    uint32_t              Index;
    uint32_t              Command_Index;
    uint32_t              String_Length;
    const QCLI_Command_t *Command_List;
    uint32_t              Command_List_Length;
    Group_List_Entry_t   *Subgroup_List_Entry;

    /* Get the size of the string. Include the null byte so the comparison
       doesn't match substrings. */
    String_Length = strlen((const char *)(Command_Parameter->String_Value)) + 1;

    if(Group_List_Entry != NULL)
    {
        /* Determine which common command list is going to be used. */
        if(Group_List_Entry == &(QCLI_Context.Root_Group))
        {
            Command_List        = Root_Command_List;
            Command_List_Length = ROOT_COMMAND_LIST_SIZE;
        }
        else
        {
            Command_List        = Common_Command_List;
            Command_List_Length = COMMON_COMMAND_LIST_SIZE;
        }

        if(Command_Parameter->Integer_Is_Valid)
        {
            /* Command was specified as an integer. */
            if(Command_Parameter->Integer_Value >= COMMAND_START_INDEX)
            {
                Command_Index = Command_Parameter->Integer_Value - COMMAND_START_INDEX;

                /* If the integer is a valid value for the command group, use it. */
                if(Command_Index < Command_List_Length)
                {
                    /* Command is in the common command list. */
                    Ret_Val                   = true;
                    Find_Result->Is_Group     = false;
                    Find_Result->Data.Command = &(Command_List[Command_Index]);
                }
                else
                {
                    Command_Index -= Command_List_Length;

                    if((Group_List_Entry->Command_Group != NULL) &&
                            (Command_Index < Group_List_Entry->Command_Group->Command_Count))
                    {
                        /* Command is in the group's command list. */
                        Ret_Val                   = true;
                        Find_Result->Is_Group     = false;
                        Find_Result->Data.Command = &(Group_List_Entry->Command_Group->Command_List[Command_Index]);
                    }
                    else
                    {
                        if(Group_List_Entry->Command_Group != NULL)
                        {
                            Command_Index -= Group_List_Entry->Command_Group->Command_Count;
                        }

                        /* Search the group list. */
                        Group_List_Entry = Group_List_Entry->Subgroup_List;
                        while((Group_List_Entry != NULL) && (Command_Index != 0))
                        {
                            Group_List_Entry = Group_List_Entry->Next_Group_List_Entry;
                            Command_Index --;
                        }

                        if(Group_List_Entry != NULL)
                        {
                            /* Command is in the subgroup list. */
                            Ret_Val                            = true;
                            Find_Result->Is_Group              = true;
                            Find_Result->Data.Group_List_Entry = Group_List_Entry;
                        }
                        else
                        {
                            Ret_Val = false;
                        }
                    }
                }
            }
            else
            {
                Ret_Val = false;
            }
        }
        else
        {
            /* Command was specified as a string. */
            Command_Index = COMMAND_START_INDEX;
            Ret_Val       = false;

            /* Search the common command list. */
            for(Index = 0; (Index < Command_List_Length) && (!Ret_Val); Index ++)
            {
                if(Memcmpi(Command_Parameter->String_Value, Command_List[Index].Command_String, String_Length) == 0)
                {
                    /* Command found. */
                    Ret_Val                          = true;
                    Find_Result->Is_Group            = false;
                    Find_Result->Data.Command        = &(Command_List[Index]);
                    Command_Parameter->Integer_Value = Command_Index;
                }
                else
                {
                    Command_Index ++;
                }
            }

            /* Only search the command group if it isn't NULL. */
            if((!Ret_Val) && (Group_List_Entry->Command_Group != NULL))
            {
                /* If the comamnd wasn't found yet, search the group's command list. */
                for(Index = 0; (Index < Group_List_Entry->Command_Group->Command_Count) && (!Ret_Val); Index ++)
                {
                    if(Memcmpi(Command_Parameter->String_Value,
                                Group_List_Entry->Command_Group->Command_List[Index].Command_String,String_Length) == 0)
                    {
                        /* Command found. */
                        Ret_Val                           = true;
                        Find_Result->Is_Group             = false;
                        Find_Result->Data.Command         = &(Group_List_Entry->Command_Group->Command_List[Index]);
                        Command_Parameter->Integer_Value  = Command_Index;
                    }
                    else
                    {
                        Command_Index ++;
                    }
                }
            }

            if(!Ret_Val)
            {
                /* If the comamnd wasn't found yet, search the group's subgroup
                   list. */
                Subgroup_List_Entry = Group_List_Entry->Subgroup_List;
                while((Subgroup_List_Entry != NULL) && (!Ret_Val))
                {
                    if(Memcmpi(Command_Parameter->String_Value,
                                Subgroup_List_Entry->Command_Group->Group_String, String_Length) == 0)
                    {
                        /* Command found. */
                        Ret_Val                            = true;
                        Find_Result->Is_Group              = true;
                        Find_Result->Data.Group_List_Entry = Subgroup_List_Entry;
                        Command_Parameter->Integer_Value   = Command_Index;
                    }
                    else
                    {
                        Command_Index ++;
                        Subgroup_List_Entry = Subgroup_List_Entry->Next_Group_List_Entry;
                    }
                }
            }
        }
    }
    else
    {
        Ret_Val = false;
    }

    return(Ret_Val);
}

/*
 * @brief This function will display the group name, recursively displaying
 * the name of the groups parents.
 *
 * @param Group_List_Entry is the group list whose name should be
 * displayed.  If this isn't the root group, the parent group's
 * name will be displayed first.
 */
static void Display_Group_Name(const Group_List_Entry_t *Group_List_Entry)
{
    /* If the group's parent isn't the root, display the parent first. */
    if(Group_List_Entry->Parent_Group != &(QCLI_Context.Root_Group))
    {
        Display_Group_Name(Group_List_Entry->Parent_Group);

        printf("\\");
    }

    /* Display this group's name. */
    printf("%s", Group_List_Entry->Command_Group->Group_String);
}

/*
 * @brief This function will processes the help command, recursively
 * decending groups if necessary.
 *
 * @param Group_List_Entry is the current command group for the help
 * command.
 * @param Paramter_Count is the number of parameters in the paramter list.
 * @param Paramter_List is the paramter list provided to the help command.
 * As the groups are recursively decended, the first paramter will
 * be stripped off until the list is empty.
 *
 * @param Parameter_Count is no. of parameter given by user.
 * @param Parameter_List is list of actual parameter valuegiven by user.
 *
 * @return
 * - 0 if the help was displayed correctly.
 * - A positive value indicating the depth of the error if a paramter was invalid.
 */
static uint32_t Display_Help(Group_List_Entry_t *Group_List_Entry, uint32_t Parameter_Count,
        QCLI_Parameter_t *Parameter_List)
{
    uint32_t      Ret_Val;
    Find_Result_t Find_Result;

    /* If a parameter was specified, see if we can tie it to a command. */
    if(Parameter_Count >= 1)
    {
        if(Find_Command(Group_List_Entry, &(Parameter_List[0]), &Find_Result))
        {
            /* Command was found, assign it now. */
            if(Find_Result.Is_Group)
            {
                /* If this was a group, recurse into it. */
                Ret_Val = Display_Help(Find_Result.Data.Group_List_Entry, Parameter_Count - 1, &(Parameter_List[1]));

                /* If the recursive call returned an error, add one to it. */
                if(Ret_Val > 0)
                {
                    Ret_Val ++;
                }
            }
            else
            {
                /* If this was the last parameter specified, display the usage for
                   the command. If it wasn't, return an error. */
                if(Parameter_Count == 1)
                {
                    Display_Usage(Parameter_List[0].Integer_Value, Find_Result.Data.Command);

                    Ret_Val = 0;
                }
                else
                {
                    /* The error code indicates that the next parameter is invalid. */
                    Ret_Val = 2;
                }
            }
        }
        else
        {
            /* Command not found so return an error. */
            Ret_Val = 1;
        }
    }
    else
    {
        /* Display the command list for the current group. */
        Display_Command_List(Group_List_Entry);

        Ret_Val = 0;
    }

    return(Ret_Val);
}

/*
 * @brief This function processes the "Help" command from the CLI.
 *
 * The parameters specified indicate the command or group to display the
 * help message for.  If no parameters are specified, the list of commands
 * for the current command group will be displayed. If the paramters
 * specify a subgroup, the command list for that group will be displayed.
 * If the paramters specify a command, the usage message for that command
 * will be displayed.
 *
 * @param Parameter_Count is no. of parameter given by user.
 * @param Parameter_List is list of actual parameter valuegiven by user.
 */
static QCLI_Command_Status_t Command_Help(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t Ret_Val;
    uint32_t              Result;
    int32_t               Index;

    if( pthread_mutex_lock(&QCLI_Context.CLI_Mutex) )
    {
        Result = Display_Help(QCLI_Context.Executing_Group, Parameter_Count, Parameter_List);

        /* if there was an error parsing the command list, print out an error
           message here (this is in addition to the usage message that will be
           printed out). */
        if(Result > 0)
        {
            printf("Command \"%s", Parameter_List[0].String_Value);

            for(Index = 1; Index < Result; Index ++)
            {
                printf(" %s", Parameter_List[Index].String_Value);
            }

            printf("\" not found.\n");

            Ret_Val = QCLI_STATUS_USAGE_E;
        }
        else
        {
            Ret_Val = QCLI_STATUS_SUCCESS_E;
        }

        pthread_mutex_unlock(&QCLI_Context.CLI_Mutex);
    }
    else
    {
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

static void Display_Usage(uint32_t Command_Index, const QCLI_Command_t *Command)
{
    printf("\n");
    printf("%d: %s %s\n", Command_Index, Command->Command_String, Command->Usage_String);
    printf("    %s\n",    Command->Description);
    printf("\n");
}

static void Display_Command_List(const Group_List_Entry_t *Group_List_Entry)
{
    uint32_t              Index;
    uint32_t              Command_Index;
    const QCLI_Command_t *Command_List;
    uint32_t              Command_List_Size;
    Group_List_Entry_t   *Subgroup_List_Entry;

    printf("\n");

    if ( Group_List_Entry ) {
        
        Command_Index = COMMAND_START_INDEX;

        printf("Command List");

        /* Display the common commands. */
        if(Group_List_Entry == &(QCLI_Context.Root_Group))
        {
            Command_List      = Root_Command_List;
            Command_List_Size = ROOT_COMMAND_LIST_SIZE;
        }
        else
        {
            printf(" (");

            if(Group_List_Entry != &(QCLI_Context.Root_Group))
            {
                Display_Group_Name(Group_List_Entry);
            }

            printf(")");

            Command_List      = Common_Command_List;
            Command_List_Size = COMMON_COMMAND_LIST_SIZE;
        }

        printf(":\n");

        printf("  Commands:\n");
        for( Index = 0; Index < Command_List_Size; Index ++ )
        {
            printf("    %2d. %s\n", Command_Index, Command_List[Index].Command_String);
            Command_Index ++;
        }

        /* Display the command list. */
        if ( (Group_List_Entry->Command_Group != NULL) && (Group_List_Entry->Command_Group->Command_List != NULL) )
        {
            printf( "\n");

            for(Index = 0; Index < Group_List_Entry->Command_Group->Command_Count; Index ++)
            {
                printf("    %2d. %s\n", Command_Index,
                        Group_List_Entry->Command_Group->Command_List[Index].Command_String);
                Command_Index ++;
            }
        }

        /* Display the group list. */
        if(Group_List_Entry->Subgroup_List != NULL)
        {
            printf("\n");
            printf("  Subgroups:\n");

            Subgroup_List_Entry = Group_List_Entry->Subgroup_List;
            while(Subgroup_List_Entry != NULL)
            {
                printf("    %2d. %s\n", Command_Index,
                        Subgroup_List_Entry->Command_Group->Group_String);

                Subgroup_List_Entry = Subgroup_List_Entry->Next_Group_List_Entry;
                Command_Index ++;
            }
        }

        printf("\n");
    }
}

static QCLI_Command_Status_t Command_Ver(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    printf(" In Version Info");

    return QCLI_STATUS_SUCCESS_E;
}

static QCLI_Command_Status_t Command_Exit(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    printf(" In Command Exit");

    return QCLI_STATUS_SUCCESS_E;
}

static QCLI_Command_Status_t Command_Up(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    printf(" In Commands Up");

    return QCLI_STATUS_SUCCESS_E;
}

static QCLI_Command_Status_t Command_Root(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    printf(" In Version Root");

    return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t platform_demo_reset(uint32_t parameters_count, QCLI_Parameter_t * parameters)
{
    printf(" In Platform Demo Reset");

    return QCLI_STATUS_SUCCESS_E;

}



void* CLI_Handle()
{
    printf(" >>>>>>>>>>>>>>> IN CLI Handle <<<<<<<<<<<<<<<<<<<<<<<<<\n");

    Display_Command_List(QCLI_Context.Current_Group);

}


int main()
{
    pthread_t CLI_THD_ID, tid2; 

    if ( pthread_mutex_init(&QCLI_Context.CLI_Mutex, NULL) != 0 ) { 
        printf("\n mutex init has failed\n"); 
        return 1; 
    } 
 
    printf(" >>>>>>>>>>>>>>> Boot UP CLI Mode <<<<<<<<<<<<<<<<<<<<<<<<<\n");

    // Create thread 1 
    
    pthread_create(&CLI_THD_ID, NULL,CLI_Handle, NULL);

    QCLI_Process_Input_Data(PAL_Context.Rx_Buffer_Length[CurrentIndex], PAL_Context.Rx_Buffer[CurrentIndex]);

    pthread_join(CLI_THD_ID, NULL); 

    return 0;
}

