/*  lesyange - Lexical and Syntatic Analyzers Generator.
    Copyright (C) 2016  Jean Jung

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "lesyange.h"
#include "ebnf_parser.h"
#include "cextensions.h"

char *next_identifier(char *source, int *col, ebnf_token_t *tk) 
{   
    char *psource = source;
    TK_SETID(tk, IDENTIFIER);
    while (*psource != '\0') 
    {
        if (isalpha(*psource) || isdigit(*psource) || *psource == '_')
        {
            INCP(col);
        }
        else
        {
            return --psource;
        }
        psource++;
    }
    return psource;
}

char *next_comment(char *source, int *line, int *col, ebnf_token_t *tk) 
{
    char *psource = source;
    TK_SETID(tk, COMMENT);
    char last = 0;
    while (*psource != '\0') 
    {
        INCP(col);
        if (last == '*' && *psource == ')') 
            return ++psource;

        if (*psource == '\n') 
        {
            INCP(line);
            *col = 1;
        }
        last = *psource;
        psource++;
    }
    UNEXPECTED_ERROR(ERROR_UNTERMINATED_COMMENT, 
        "The comment at line %d, col %d was not correctly closed", 
        tk->line, tk->col);
}

char *next_group_or_comment(char *source,int *line,int *col,ebnf_token_t *tk) 
{
    char *psource = source;
    psource++;
    char c = *psource;
    if (c == '\0') 
    {
        UNEXPECTED_ERROR(ERROR_UNEXPECTED_EOF, 
            "Unexpected eof at line %d, col %d", *line, *col);
    } else if (c == '*') 
    {
        return next_comment(psource, line, col, tk);
    } else 
    {
        psource--;
        TK_SETID(tk, OPEN_GROUP);
        return psource;
    }
}

char *next_terminal(char *source,int *line,int *col,const char close,ebnf_token_t *tk) 
{
    char *psource = source;
    INCP(col);
    psource++;
    while (*psource != '\0') 
    {
        if (*psource == close) 
        {
            return psource; 
        } else if (*psource == '\n')
        {
            INCP(line);
            *col = 1;
        }
        psource++;
        INCP(col);
    }
    UNEXPECTED_ERROR(ERROR_UNTERMINATED_LITERAL, 
        "The literal at line %d, col %d was not correctly closed", tk->line, tk->col);
}

ebnf_token_t next_token(char **psource, int *line, int *col) 
{
    ebnf_token_t tk;
    TK_SETID(&tk, UNKNOWN);
    char *start = *psource;
    tk.lexeme = NULL; 
    tk.line = *line;
    tk.col = *col;
    while (**psource != '\0') 
    {
        if (isalpha(**psource)) 
        {
            *psource = next_identifier(*psource, col, &tk);
            goto bingo;
        } else if (**psource == '\t' || **psource == '\r' || **psource == ' ') 
        {
            // nothing to do;
        } else if (**psource == '\n')
        {
            INCP(line);
            *col = 1;
        } else if (**psource == '=') 
        {
            TK_SETID(&tk, DEFINE);
            goto bingo;
        } else if (**psource == ',') 
        {
            TK_SETID(&tk, CAT);
            goto bingo;
        }  else if (**psource == ';')
        {
            TK_SETID(&tk, TERMINATION_SC);
            goto bingo;
        } else if (**psource == '|') 
        {
            TK_SETID(&tk, UNION);
            goto bingo;
        } else if (**psource == '[') {
            TK_SETID(&tk, OPEN_OPTION);
            goto bingo;
        }  else if (**psource == ']') {
            TK_SETID(&tk, CLOSE_OPTION);
            goto bingo;
        } else if (**psource == '{') {
            TK_SETID(&tk, OPEN_REPETITION);
            goto bingo;
        } else if (**psource == '}') {
            TK_SETID(&tk, CLOSE_REPETITION);
            goto bingo; 
        } else if (**psource == '(') {
            *psource = next_group_or_comment(*psource, line, col, &tk);
            if (tk.id == COMMENT) 
            {
                return next_token(psource, line, col);
            }
            goto bingo;
        } else if (**psource == ')') {
            TK_SETID(&tk, CLOSE_GROUP);
            goto bingo;
        } else if (**psource == '\"') {
            TK_SETID(&tk, TERMINAL_DQ);
            *psource = next_terminal(*psource, line, col, '\"', &tk); 
            goto bingo;
        } else if (**psource == '\'') {
            TK_SETID(&tk, TERMINAL_SQ);
            *psource = next_terminal(*psource, line, col, '\'', &tk);
            goto bingo;
        } else if (**psource == '<') {
            *psource = next_identifier(*psource, col, &tk);
            (*psource)++;
            INCP(col);
            if (**psource != '>')
                UNEXPECTED_ERROR(ERROR_UNEXPECTED_CHAR, 
                  "Expecting '<' but found '%c' at line %d, col %d", **psource,
                  *line, *col);
            goto bingo;
        } else if (**psource == '-') {
            TK_SETID(&tk, EXCEPTION);
            goto bingo;
        } else if (**psource == '.') {
            TK_SETID(&tk, TERMINATION_DOT);
            goto bingo;
        } else 
        {
            TK_SETID(&tk, UNKNOWN);
            goto bingo;
        }
        (*psource)++;
        start = *psource;
        tk.line = *line;
        tk.col = INCP(col);
        if (**psource == '\0') 
        {
            TK_SETID(&tk, DOLLAR);
            TK_SETLEX(&tk, '$');
            return tk;
        }
    }
bingo:{
    int size = 1;
    char *pstart = start;
    while (pstart != *psource){size++; pstart++;}
    tk.lexeme = calloc((size_t)size + 1, sizeof(char));
    strncpy(tk.lexeme, start, (unsigned)size);
    tk.lexeme[size] = '\0';
}
    return tk;
}

void parse_ebnf(OPT_CALL) 
{
    FILE *fp = fopen(opt.ebnf_file, "r");
    if (fp == NULL) 
    {   
        UNEXPECTED_ERROR(ERROR_OPENING_FILE, "%s\n", opt.ebnf_file);
    }    
    char *source = fcat(fp);
    if (source == NULL)
    {
        if (ferror(fp)) 
        {
            UNEXPECTED_ERROR(ERROR_READING_FILE, 
                "Error reading the file %s\n", opt.ebnf_file);    
        } else
        {
            UNEXPECTED_ERROR(ERROR_NOT_ENOUGH_MEMORY, 
               "Insufficient memory to store the file contents: %s\n", 
               opt.ebnf_file);
        }
    }
    fclose(fp);
    DEBUG_LOG(opt, "Source:\n%s\n", source);
    char *psource = source;
    int line = 1, col = 1;
    int *lltable[] = LL_TABLE;
    int *productions[] = PRODUCTIONS;
    ilstack_t stack;
    ilstack_init(&stack);
    ilstack_push(&stack, DOLLAR);
    ilstack_push(&stack, NT_GRAMMAR);
    while (stack.top != NULL)
    {
        char *pos = psource;
        int tline = line, tcol = col;
        ebnf_token_t tk = next_token(&psource, &tline, &tcol);
        DEBUG_LOG(opt, 
            "Lexer: %s(%s) at line %d, col %d", 
                tk.class, tk.lexeme, tk.line, tk.col);
        if (opt.d)
        {
            char *stack_str = ilstack_toa(&stack);
            DEBUG_LOG(opt, "Stack: %s", stack_str);
            free(stack_str);
        }
        int top = ilstack_pop(&stack);
        if (tk.id == top)
        {
            psource++;
            line = tline;
            col = tcol;
            DEBUG_LOG(opt, 
                "Syntatic: Reduce %s(%s) at line %d, col %d", 
                    tk.class, tk.lexeme, tk.line, tk.col);
            
        } else if (IS_NT(top))
        {
            int shift = lltable[top-FIRST_NT][tk.id];
            if (shift == -1) 
            {
                UNEXPECTED_ERROR(ERROR_UNEXPECTED_TOKEN, 
                     "Unexpected token %s(%s) at line %d, col %d", 
                        tk.class, tk.lexeme, tk.line, tk.col);
            }
            DEBUG_LOG(opt, 
                "Syntatic: Shift %d -> %d at line %d, col %d", 
                    top, shift, tk.line, tk.col);
            int *prod = productions[shift];
            while (*prod != UNKNOWN) 
            {
                ilstack_push(&stack, *prod);
                prod++;
            }
            psource = pos;
        } else 
        {
            execute_ebnf_action(opt, top, &tk);
        }
    }
    free(source);    
}
