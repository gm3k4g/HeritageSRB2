// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: command.c,v 1.4 2000/08/03 17:57:41 bpereira Exp $
//
// Copyright (C) 1998-2000 by DooM Legacy Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
// $Log: command.c,v $
// Revision 1.4  2000/08/03 17:57:41  bpereira
// no message
//
// Revision 1.3  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.2  2000/02/26 00:28:42  hurdler
// Mostly bug fix (see borislog.txt 23-2-2000, 24-2-2000)
//
//
// DESCRIPTION:
//      parse and execute commands from console input/scripts/
//      and remote server.
//
//      handles console variables, which is a simplified version
//      of commands, each consvar can have a function called when
//      it is modified.. thus it acts nearly as commands.
//
//      code shamelessly inspired by the QuakeC sources, thanks Id :)
//
//-----------------------------------------------------------------------------


#include "doomdef.h"
#include "doomstat.h"
#include "command.h"
#include "console.h"
#include "z_zone.h"
#include "d_clisrv.h"
#include "d_netcmd.h"
#include "m_misc.h"
#include "m_fixed.h"


//========
// protos.
//========
static boolean COM_Exists (const char *com_name);
static void    COM_ExecuteString (char *text);

static void    COM_Alias_f (void);
static void    COM_Echo_f (void);
static void    COM_Exec_f (void);
static void    COM_Wait_f (void);
static void    COM_Help_f (void);
static void    COM_Toggle_f (void);

static boolean     CV_Command (void);
static consvar_t  *CV_FindVar (const char *name);
static const char *CV_StringValue (const char *var_name);

static char    com_token[1024];
static char    *COM_Parse (char *data);

CV_PossibleValue_t CV_OnOff[] =    {{0,"Off"}, {1,"On"},    {0,NULL}};
CV_PossibleValue_t CV_YesNo[] =     {{0,"No"} , {1,"Yes"},   {0,NULL}};
CV_PossibleValue_t CV_Unsigned[]=   {{0,"MIN"}, {999999999,"MAX"}, {0,NULL}};

#define COM_BUF_SIZE    8192   // command buffer size

int     com_wait;       // one command per frame (for cmd sequences)


// command aliases
//
typedef struct cmdalias_s
{
    struct cmdalias_s   *next;
    char    *name;
    char    *value;     // the command string to replace the alias
} cmdalias_t;

cmdalias_t  *com_alias; // aliases list


// =========================================================================
//                            COMMAND BUFFER
// =========================================================================


vsbuf_t   com_text;     // variable sized buffer


//  Add text in the command buffer (for later execution)
//
void COM_BufAddText (char *text)
{
    int     l;

    l = strlen (text);

    if (com_text.cursize + l >= com_text.maxsize)
    {
        CONS_Printf ("Command buffer full!\n");
        return;
    }
    VS_Write (&com_text, text, l);
}


// Adds command text immediately after the current command
// Adds a \n to the text
//
void COM_BufInsertText (char *text)
{
    char    *temp;
    int     templen;

    // copy off any commands still remaining in the exec buffer
    templen = com_text.cursize;
    if (templen)
    {
        temp = ZZ_Alloc (templen);
        memcpy (temp, com_text.data, templen);
        VS_Clear (&com_text);
    }
    else
        temp = NULL;    // shut up compiler

    // add the entire text of the file (or alias)
    COM_BufAddText (text);

    // add the copied off data
    if (templen)
    {
        VS_Write (&com_text, temp, templen);
        Z_Free (temp);
    }
}


//  Flush (execute) console commands in buffer
//   does only one if com_wait
//
void COM_BufExecute (void)
{
    int     i;
    char    *text;
    char    line[1024];
    int     quotes;

    if (com_wait)
    {
        com_wait--;
        return;
    }

    while (com_text.cursize)
    {
        // find a '\n' or ; line break
        text = (char *)com_text.data;

        quotes = 0;
        for (i=0 ; i< com_text.cursize ; i++)
        {
            if (text[i] == '"')
                quotes++;
            if ( !(quotes&1) &&  text[i] == ';')
                break;  // don't break if inside a quoted string
            if (text[i] == '\n')
                break;
        }


        memcpy (line, text, i);
        line[i] = 0;

        // flush the command text from the command buffer, _BEFORE_
        // executing, to avoid that 'recursive' aliases overflow the
        // command text buffer, in that case, new commands are inserted
        // at the beginning, in place of the actual, so it doesn't
        // overflow
        if (i == com_text.cursize)
            // the last command was just flushed
            com_text.cursize = 0;
        else
        {
            i++;
            com_text.cursize -= i;
            memcpy (text, text+i, com_text.cursize);
        }

        // execute the command line
        COM_ExecuteString (line);

        // delay following commands if a wait was encountered
        if (com_wait)
        {
            com_wait--;
            break;
        }
    }
}


// =========================================================================
//                            COMMAND EXECUTION
// =========================================================================

typedef struct xcommand_s
{
    const char         *name;
    struct xcommand_s  *next;
    com_func_t         function;
} xcommand_t;

static  xcommand_t  *com_commands = NULL;     // current commands


#define MAX_ARGS        80
static int         com_argc;
static char        *com_argv[MAX_ARGS];
static char        *com_null_string = "";
static char        *com_args = NULL;          // current command args or NULL

void Got_NetVar(char **p,int playernum);
//  Initialise command buffer and add basic commands
//
void COM_Init (void)
{
    // allocate command buffer
    VS_Alloc (&com_text, COM_BUF_SIZE);

    // add standard commands
    COM_AddCommand ("alias",COM_Alias_f);
    COM_AddCommand ("echo", COM_Echo_f);
    COM_AddCommand ("exec", COM_Exec_f);
    COM_AddCommand ("wait", COM_Wait_f);
    COM_AddCommand ("help", COM_Help_f);
    COM_AddCommand ("toggle", COM_Toggle_f);
    RegisterNetXCmd(XD_NETVAR,Got_NetVar);
}


// Returns how many args for last command
//
int COM_Argc (void)
{
    return com_argc;
}


// Returns string pointer for given argument number
//
char *COM_Argv (int arg)
{
    if ( arg >= com_argc || arg < 0 )
        return com_null_string;
    return com_argv[arg];
}


// Returns string pointer of all command args
//
char *COM_Args (void)
{
    return com_args;
}


int COM_CheckParm (char *check)
{
    int         i;

    for (i = 1;i<com_argc;i++)
    {
        if ( !strcasecmp(check, com_argv[i]) )
            return i;
    }
    return 0;
}


// Parses the given string into command line tokens.
//
// Takes a null terminated string.  Does not need to be /n terminated.
// breaks the string up into arg tokens.
static void COM_TokenizeString (char *text)
{
    int     i;

// clear the args from the last string
    for (i=0 ; i<com_argc ; i++)
        Z_Free (com_argv[i]);

    com_argc = 0;
    com_args = NULL;

    while (1)
    {
// skip whitespace up to a /n
        while (*text && *text <= ' ' && *text != '\n')
            text++;

        if (*text == '\n')
        {   // a newline means end of command in buffer,
            // thus end of this command's args too
            text++;
            break;
        }

        if (!*text)
            return;

        if (com_argc == 1)
            com_args = text;

        text = COM_Parse (text);
        if (!text)
            return;

        if (com_argc < MAX_ARGS)
        {
            com_argv[com_argc] = ZZ_Alloc (strlen(com_token)+1);
            strcpy (com_argv[com_argc], com_token);
            com_argc++;
        }
    }

}


// Add a command before existing ones.
//
void COM_AddCommand (const char *name, com_func_t func)
{
    xcommand_t  *cmd;

    // fail if the command is a variable name
    if (CV_StringValue(name)[0])
    {
        CONS_Printf ("%s is a variable name\n", name);
        return;
    }

    // fail if the command already exists
    for (cmd=com_commands ; cmd ; cmd=cmd->next)
    {
        if (!strcmp (name, cmd->name))
        {
            CONS_Printf ("Command %s already exists\n", name);
            return;
        }
    }

    cmd = ZZ_Alloc (sizeof(xcommand_t));
    cmd->name = name;
    cmd->function = func;
    cmd->next = com_commands;
    com_commands = cmd;
}


//  Returns true if a command by the name given exists
//
static boolean COM_Exists (const char *com_name)
{
    xcommand_t  *cmd;

    for (cmd=com_commands ; cmd ; cmd=cmd->next)
    {
        if (!strcmp (com_name,cmd->name))
            return true;
    }

    return false;
}


//  Command completion using TAB key like '4dos'
//  Will skip 'skips' commands
//
const char *COM_CompleteCommand (char *partial, int skips)
{
    xcommand_t  *cmd;
    int        len;

    len = strlen(partial);

    if (!len)
        return NULL;

// check functions
    for (cmd=com_commands ; cmd ; cmd=cmd->next)
        if (!strncmp (partial,cmd->name, len))
            if (!skips--)
                return cmd->name;

    return NULL;
}



// Parses a single line of text into arguments and tries to execute it.
// The text can come from the command buffer, a remote client, or stdin.
//
static void COM_ExecuteString (char *text)
{
    xcommand_t  *cmd;
    cmdalias_t *a;

    COM_TokenizeString (text);

// execute the command line
    if (!COM_Argc())
        return;     // no tokens

// check functions
    for (cmd=com_commands ; cmd ; cmd=cmd->next)
    {
        if (!strcmp (com_argv[0],cmd->name))
        {
            cmd->function ();
            return;
        }
    }

// check aliases
    for (a=com_alias ; a ; a=a->next)
    {
        if (!strcmp (com_argv[0], a->name))
        {
            COM_BufInsertText (a->value);
            return;
        }
    }

// check cvars
    if (!CV_Command ())
        CONS_Printf ("Unknown command '%s'\n", COM_Argv(0));
}



// =========================================================================
//                            SCRIPT COMMANDS
// =========================================================================


// alias command : a command name that replaces another command
//
static void COM_Alias_f (void)
{
    cmdalias_t  *a;
    char        cmd[1024];
    int         i, c;

    if (COM_Argc()<3)
    {
        CONS_Printf("alias <name> <command>\n");
        return;
    }

    a = ZZ_Alloc (sizeof(cmdalias_t));
    a->next = com_alias;
    com_alias = a;

    a->name = Z_StrDup (COM_Argv(1));

// copy the rest of the command line
    cmd[0] = 0;     // start out with a null string
    c = COM_Argc();
    for (i=2 ; i< c ; i++)
    {
        strcat (cmd, COM_Argv(i));
        if (i != c)
            strcat (cmd, " ");
    }
    strcat (cmd, "\n");

    a->value = Z_StrDup (cmd);
}


// Echo a line of text to console
//
static void COM_Echo_f (void)
{
    int     i;

    for (i=1 ; i<COM_Argc() ; i++)
        CONS_Printf ("%s ",COM_Argv(i));
    CONS_Printf ("\n");
}


// Execute a script file
//
static void COM_Exec_f (void)
{
    UINT8*   buf=NULL;

    if (COM_Argc () != 2)
    {
        CONS_Printf ("exec <filename> : run a script file\n");
        return;
    }

// load file

    FIL_ReadFile (COM_Argv(1), &buf);

    if (!buf)
    {
        CONS_Printf ("couldn't execute file %s\n",COM_Argv(1));
        return;
    }

    CONS_Printf ("executing %s\n",COM_Argv(1));

// insert text file into the command buffer

    COM_BufInsertText ((char*)buf);

// free buffer

    Z_Free(buf);

}


// Delay execution of the rest of the commands to the next frame,
// allows sequences of commands like "jump; fire; backward"
//
static void COM_Wait_f (void)
{
    if (COM_Argc()>1)
        com_wait = atoi(COM_Argv(1));
    else
        com_wait = 1;   // 1 frame
}

static void    COM_Help_f (void)
{
    xcommand_t  *cmd;
    consvar_t  *cvar;
    int i=0;

    if(COM_Argc()>1)
    {
        cvar = CV_FindVar (COM_Argv(1));
        if( cvar && cvar->PossibleValue )
        {
            if(stricmp(cvar->PossibleValue[0].strvalue,"MIN")==0)
            {
                for(i=1;cvar->PossibleValue[i].strvalue!=NULL;i++)
                    if(!stricmp(cvar->PossibleValue[i].strvalue,"MAX"))
                        break;
                CONS_Printf("Variable %s is range %d to %d\n",cvar->name,cvar->PossibleValue[0].value,cvar->PossibleValue[i].value);
            }
            else
            {
                CONS_Printf("Variable %s have possible value :\n",cvar->name);
                while(cvar->PossibleValue[i].strvalue)
                {
                     CONS_Printf("%-2d : %s\n",cvar->PossibleValue[i].value,cvar->PossibleValue[i].strvalue);
                     i++;
                }
            }
        }
        else
            CONS_Printf("No Help for this command/variable\n");
    }
    else
    {
        // commands
        CONS_Printf("Commands\n");
        for (cmd=com_commands ; cmd ; cmd=cmd->next)
        {
            CONS_Printf("%s ",cmd->name);
            i++;
        }

            // varibale
        CONS_Printf("\nVariable\n");
        for (cvar=consvar_vars; cvar; cvar = cvar->next)
        {
            CONS_Printf("%s ",cvar->name);
            i++;
        }

        CONS_Printf("\nread console.txt for more or type help <command or variable>\n");

        if( devparm )
            CONS_Printf("Total : %d\n",i);
    }
}

static void COM_Toggle_f(void)
{
    consvar_t  *cvar;

    if(COM_Argc()!=2 && COM_Argc()!=3)
    {
        CONS_Printf("Toggle <cvar_name> [-1]\n"
                    "Toggle the value of a cvar\n");
        return;
    }
    cvar = CV_FindVar (COM_Argv(1));
    if(!cvar)
    {
        CONS_Printf("%s is not a cvar\n",COM_Argv(1));
        return;
    }

    // netcvar don't change imediately
    cvar->flags |= CV_SHOWMODIFONETIME;
    if( COM_Argc()==3 )
        CV_AddValue(cvar,atol(COM_Argv(2)));
    else
        CV_AddValue(cvar,+1);
}

// =========================================================================
//                      VARIABLE SIZE BUFFERS
// =========================================================================

#define VSBUFMINSIZE   256

void VS_Alloc (vsbuf_t *buf, int initsize)
{
    if (initsize < VSBUFMINSIZE)
        initsize = VSBUFMINSIZE;
    buf->data = Z_Malloc (initsize, PU_STATIC, NULL);
    buf->maxsize = initsize;
    buf->cursize = 0;
}


void VS_Free (vsbuf_t *buf)
{
//  Z_Free (buf->data);
    buf->cursize = 0;
}


void VS_Clear (vsbuf_t *buf)
{
    buf->cursize = 0;
}


void *VS_GetSpace (vsbuf_t *buf, int length)
{
    void    *data;

    if (buf->cursize + length > buf->maxsize)
    {
        if (!buf->allowoverflow)
            I_Error ("overflow 111");

        if (length > buf->maxsize)
            I_Error ("overflow l%i 112", length);

        buf->overflowed = true;
        CONS_Printf ("VS buffer overflow");
        VS_Clear (buf);
    }

    data = buf->data + buf->cursize;
    buf->cursize += length;

    return data;
}


//  Copy data at end of variable sized buffer
//
void VS_Write (vsbuf_t *buf, void *data, int length)
{
    memcpy (VS_GetSpace(buf,length),data,length);
}


//  Print text in variable size buffer, like VS_Write + trailing 0
//
void VS_Print (vsbuf_t *buf, char *data)
{
    int     len;

    len = strlen(data)+1;

    if (buf->data[buf->cursize-1])
        memcpy ((UINT8 *)VS_GetSpace(buf, len),data,len); // no trailing 0
    else
        memcpy ((UINT8 *)VS_GetSpace(buf, len-1)-1,data,len); // write over trailing 0
}

// =========================================================================
//
//                           CONSOLE VARIABLES
//
//   console variables are a simple way of changing variables of the game
//   through the console or code, at run time.
//
//   console vars acts like simplified commands, because a function can be
//   attached to them, and called whenever a console var is modified
//
// =========================================================================

consvar_t         *consvar_vars;       // list of registered console variables
static char       *cv_null_string = "";


//  Search if a variable has been registered
//  returns true if given variable has been registered
//
static consvar_t *CV_FindVar (const char *name)
{
    consvar_t  *cvar;

    for (cvar=consvar_vars; cvar; cvar = cvar->next)
        if ( !strcmp(name,cvar->name) )
            return cvar;

    return NULL;
}


//  Build a unique Net Variable identifier number, that is used
//  in network packets instead of the fullname
//
unsigned short CV_ComputeNetid (const char *s)
{
    unsigned short ret;
    static int premiers[16] = {2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53};
    int i;

    ret=0;
    i=0;
    while(*s)
    {
        ret += (*s)*premiers[i];
        s++;
        i = (i+1)%16;
    }
    return ret;
}


//  Return the Net Variable, from it's identifier number
//
static consvar_t *CV_FindNetVar (unsigned short netid)
{
    consvar_t  *cvar;

    for (cvar=consvar_vars; cvar; cvar = cvar->next)
        if (cvar->netid==netid)
            return cvar;

    return NULL;
}

void CV_SendModifiedNetVars(void)
{
    consvar_t  *cvar;
    char       buf[128];

    for (cvar=consvar_vars; cvar; cvar = cvar->next)
        if (((cvar->flags & CV_NETVAR)!=0) && ((cvar->flags & CV_MODIFIED)!=0))
        {
//            CONS_Printf("send %s because differant (%s)\n",cvar->name,cvar->string);
            *((unsigned short *)buf) = cvar->netid;
            strcpy (&buf[2], cvar->string);
            SendNetXCmd (XD_NETVAR, &buf, 2+strlen(&buf[2])+1);
        }
}

void Setvalue (consvar_t *var, const char *valstr, boolean stealth);

//  Register a variable, that can be used later at the console
//
void CV_RegisterVar (consvar_t *variable)
{
    // first check to see if it has allready been defined
    if (CV_FindVar (variable->name))
    {
        CONS_Printf ("Variable %s is already defined\n", variable->name);
        return;
    }

    // check for overlap with a command
    if (COM_Exists (variable->name))
    {
        CONS_Printf ("%s is a command name\n", variable->name);
        return;
    }

    // check net variables
    if (variable->flags & CV_NETVAR)
    {
        variable->netid = CV_ComputeNetid (variable->name);
        if (CV_FindNetVar(variable->netid))
            I_Error("Variable %s have same netid\n",variable->name);
    }

    // link the variable in
    variable->next = consvar_vars;
    consvar_vars = variable;
    variable->string = variable->zstring = NULL;

#ifdef PARANOIA
    if ((variable->flags & CV_NOINIT) && !(variable->flags & CV_CALL))
        I_Error("variable %s have CV_NOINIT without CV_CALL\n");
#endif
    if (variable->flags & CV_NOINIT)
        variable->flags &=~CV_CALL;

    Setvalue(variable,variable->defaultvalue,false);

    if (variable->flags & CV_NOINIT)
        variable->flags |= CV_CALL;

    // the SetValue will set this bit
    variable->flags &= ~CV_MODIFIED;
}


//  Returns the string value of a console var
//
static const char *CV_StringValue (const char *var_name)
{
    consvar_t *var;

    var = CV_FindVar (var_name);
    if (!var)
        return cv_null_string;
    return var->string;
}


//  Completes the name of a console var
//
const char *CV_CompleteVar (char *partial, int skips)
{
    consvar_t   *cvar;
    int         len;

    len = strlen(partial);

    if (!len)
        return NULL;

    // check functions
    for (cvar=consvar_vars ; cvar ; cvar=cvar->next)
        if (!strncmp (partial,cvar->name, len))
            if (!skips--)
                return cvar->name;

    return NULL;
}


// set value to the variable, no check only for internal use
//
void Setvalue (consvar_t *var, const char *valstr, boolean stealth)
{
	char *valstrbuf = Z_StrDup (valstr);

    if(var->PossibleValue)
    {
        int v=atoi(valstr);

        if(!stricmp(var->PossibleValue[0].strvalue,"MIN"))
        {   // bounded cvar
            int i;
            // search for maximum
            for(i=1;var->PossibleValue[i].strvalue!=NULL;i++)
                if(!stricmp(var->PossibleValue[i].strvalue,"MAX"))
                    break;
#ifdef PARANOIA
            if(var->PossibleValue[i].strvalue==NULL)
                I_Error("Bounded cvar \"%s\" without Maximum !",var->name);
#endif
            if(v<var->PossibleValue[0].value)
            {
				v=var->PossibleValue[0].value;
				snprintf(valstrbuf,strlen(valstrbuf)+1,"%d",v);
            }
            if(v>var->PossibleValue[i].value)
            {
				v=var->PossibleValue[i].value;
				snprintf(valstrbuf,strlen(valstrbuf)+1,"%d",v);
			}
        }
        else
        {
            // waw spaghetti programming ! :)
            int i;

            // check first strings
            for(i=0;var->PossibleValue[i].strvalue!=NULL;i++)
                if(!stricmp(var->PossibleValue[i].strvalue,valstr))
                    goto found;
            if(!v)
               if(strcmp(valstr,"0")!=0) // !=0 if valstr!="0"
                    goto error;
            // check int now
            for(i=0;var->PossibleValue[i].strvalue!=NULL;i++)
                if(v==var->PossibleValue[i].value)
                    goto found;

error:      // not found
            CONS_Printf("\"%s\" is not a possible value for \"%s\"\n", valstr, var->name);
            if(var->defaultvalue==valstr)
                I_Error("Variable %s default value \"%s\" is not a possible value\n",var->name,var->defaultvalue);
            return;
found:
            var->value =var->PossibleValue[i].value;
            var->string=var->PossibleValue[i].strvalue;
            goto finish;
        }
    }

    // free the old value string
    if(var->zstring)
		Z_Free (var->zstring);

    var->string = var->zstring = Z_StrDup (valstrbuf);
    free(valstrbuf);

    if (var->flags & CV_FLOAT)
    {
        double d;
        d = atof (var->string);
        var->value = d * FRACUNIT;
    }
    else
        var->value = atoi (var->string);

finish:
    if( var->flags & CV_SHOWMODIFONETIME || var->flags & CV_SHOWMODIF)
    {
        CONS_Printf("%s set to %s\n",var->name,var->string);
        var->flags &= ~CV_SHOWMODIFONETIME;
    }
    DEBFILE(va("%s set to %s\n",var->name,var->string));
    var->flags |= CV_MODIFIED;
    // raise 'on change' code
    if (var->flags & CV_CALL && !stealth)
        var->func ();
}


//
// Use XD_NETVAR argument :
//      2 byte for variable identification
//      then the value of the variable followed with a 0 byte (like str)
//
void Got_NetVar(char **p,int playernum)
{
    consvar_t  *cvar;
    char *svalue;

    cvar = CV_FindNetVar (*((unsigned short *)(*p)));
    *p+=2;
    svalue = *p;
    *p+=strlen(svalue)+1;
    if(cvar==NULL)
    {
        CONS_Printf("Netvar not found\n");
        return;
    }
    Setvalue(cvar,svalue,false);
}


//  does as if "<varname> <value>" is entered at the console
//
static void SetCVAR (consvar_t *var, const char *value, boolean stealth)
{
    //changed = strcmp(var->string, value);
#ifdef PARANOIA
    if(!var)
        I_Error("CV_Set : no variable\n");
    if(!var->string)
        I_Error("cv_Set : %s no string set ?!\n",var->name);
#endif
    if (stricmp(var->string, value)==0)
        return; // no changes

    if (var->flags & CV_NETVAR)
    {
        // send the value of the variable
        char buf[128];
        if (!server)
        {
            CONS_Printf("Only the server can change this variable: "); // Tails 05-19-2001
			CONS_Printf(var->name); // Tails 05-19-2001
			CONS_Printf(" "); // Tails 05-19-2001
			CONS_Printf(var->string); // Tails 05-19-2001
			CONS_Printf("\n"); // Tails 05-19-2001
            return;
        }
        *((unsigned short *)buf) = var->netid;
        strcpy (&buf[2], value);
        SendNetXCmd (XD_NETVAR, &buf, 2+strlen(&buf[2])+1);
    }
    else
        if ((var->flags & CV_NOTINNET) && netgame)
        {
            CONS_Printf("This Variable can't be changed while in netgame\n");
            return;
        }
        else
            Setvalue(var,value,stealth);
}


//  Expands value to string before calling CV_Set ()
//
static void SetCVARValue (consvar_t *var, int value, boolean stealth)
{
    char    val[32];

    sprintf (val, "%d", value);
    SetCVAR (var, val, stealth);
}

void CV_Set (consvar_t *var, const char *value)
{
    SetCVAR(var, value, false);
}

void CV_SetValue (consvar_t *var, int value)
{
    SetCVARValue(var, value, false);
}

void CV_StealthSet (consvar_t *var, const char *value)
{
    SetCVAR(var, value, true);
}

void CV_StealthSetValue (consvar_t *var, int value)
{
    SetCVARValue(var, value, true);
}

void CV_AddValue (consvar_t *var, int increment)
{
    int   newvalue=var->value+increment;

    if( var->PossibleValue )
    {
#define MIN 0

        if( strcmp(var->PossibleValue[MIN].strvalue,"MIN")==0 )
    {
        int max;
        // seach the next to last
        for(max=0;var->PossibleValue[max+1].strvalue!=NULL;max++)
            ;

            if( newvalue<var->PossibleValue[MIN].value )
                newvalue+=var->PossibleValue[max].value-var->PossibleValue[MIN].value+1;   // add the max+1
            newvalue=var->PossibleValue[MIN].value +
                     (newvalue-var->PossibleValue[MIN].value) %
                       (var->PossibleValue[max].value -
                        var->PossibleValue[MIN].value+1);

            CV_SetValue(var,newvalue);
        }
        else
        {
            int max,currentindice=-1,newindice;

            // this code do not support more than same value for differant PossibleValue
            for(max=0;var->PossibleValue[max].strvalue!=NULL;max++)
                if( var->PossibleValue[max].value==var->value )
                    currentindice=max;
            max--;
#ifdef PARANOIA
            if( currentindice==-1 )
                I_Error("CV_AddValue : current value %d not found in possible value\n",var->value);
#endif
            newindice=(currentindice+increment+max+1) % (max+1);
            CV_Set(var,var->PossibleValue[newindice].strvalue);
        }
    }
    else
        CV_SetValue(var,newvalue);
}


//  Allow display of variable content or change from the console
//
//  Returns false if the passed command was not recognised as
//  console variable.
//
static boolean CV_Command (void)
{
    consvar_t      *v;

    // check variables
    v = CV_FindVar (COM_Argv(0));
    if (!v)
        return false;

    // perform a variable print or set
    if (COM_Argc() == 1)
    {
        CONS_Printf ("\"%s\" is \"%s\" default is \"%s\"\n", v->name, v->string, v->defaultvalue);
        return true;
    }

    CV_Set (v, COM_Argv(1));
    return true;
}


//  Save console variables that have the CV_SAVE flag set
//
void CV_SaveVariables (FILE *f)
{
    consvar_t      *cvar;

    for (cvar = consvar_vars ; cvar ; cvar=cvar->next)
        if (cvar->flags & CV_SAVE)
            fprintf (f, "%s \"%s\"\n", cvar->name, cvar->string);
}


//============================================================================
//                            SCRIPT PARSE
//============================================================================

static char    com_token[1024];        // a single token at a time


//  Parse a token out of a string, handles script files too
//  returns the data pointer after the token
static char *COM_Parse (char *data)
{
    int     c;
    int     len;

    len = 0;
    com_token[0] = 0;

    if (!data)
        return NULL;

// skip whitespace
skipwhite:
    while ( (c = *data) <= ' ')
    {
        if (c == 0)
            return NULL;            // end of file;
        data++;
    }

// skip // comments
    if (c=='/' && data[1] == '/')
    {
        while (*data && *data != '\n')
            data++;
        goto skipwhite;
    }


// handle quoted strings specially
    if (c == '\"')
    {
        data++;
        while (1)
        {
            c = *data++;
            if (c=='\"' || !c)
            {
                com_token[len] = 0;
                return data;
            }
            com_token[len] = c;
            len++;
        }
    }

// parse single characters
    if (c=='{' || c=='}'|| c==')'|| c=='(' || c=='\'' || c==':')
    {
        com_token[len] = c;
        len++;
        com_token[len] = 0;
        return data+1;
    }

// parse a regular word
    do
    {
        com_token[len] = c;
        data++;
        len++;
        c = *data;
        if (c=='{' || c=='}'|| c==')'|| c=='(' || c=='\'' || c==':')
            break;
    } while (c>32);

    com_token[len] = 0;
    return data;
}
