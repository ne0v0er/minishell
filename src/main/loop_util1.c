/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   loop_util1.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: auzou <auzou@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/21 13:57:37 by auzou             #+#    #+#             */
/*   Updated: 2025/03/19 19:06:51 by auzou            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../incl/loop.h"
#include "../../incl/minishell.h"

/* helper function to extract a filename that follows a redirect ops */
void	extract_redirect_filename(char *input, int *i, t_token **tokens)
{
	int		start;
	char	*word;

	start = *i;
	while (input[*i] && !ft_isspace(input[*i])
		&& !is_quote(input[*i]) && input[*i] != '|')
		(*i)++;
	if (*i > start)
	{
		word = strndup(&input[start], *i - start);
		add_token(tokens, create_token(word, TOKEN_WORD));
		free(word);
	}
}

t_token	*tokenize(char *input)
{
	t_token		*tokens;
	int			i;
	int			type;

	tokens = NULL;
	i = 0;
	type = 0;
	while (input[i])
	{
		while (ft_isspace(input[i]))
			i++;
		if (is_special_char(input[i]))
			tokenize_util1(tokens, &i, input);
		else if (input[i] == '|')
		{
			char op[3] = {0};
			op[0] = input[i];
			add_token(&tokens, create_token(op, get_token_type(op)));
			i++;
		}
		else if (input[i] == '\'' || input[i] == '"')
		{
			char quote = input[i++];
			char *quoted = extract_quoted(input, &i, quote);
			if (!quoted)
			{
				exit(0);
				return (NULL);
			}
			if (quote == '\'')
				type = TOKEN_SINGLE_QUOTE;
			else
				type = TOKEN_DOUBLE_QUOTE;
			add_token(&tokens, create_token(quoted, type));
			free(quoted);
		}
		else
		{
			int start = i;
			while (input[i] && !ft_isspace(input[i]) && !is_special_char(input[i]) && !is_quote(input[i]))
				i++;
			if (i > start)
			{
				char *word = strndup(&input[start], i - start);
				while (input[i] && !ft_isspace(input[i]) && !is_special_char(input[i]))
				{
					while (input[i] && is_quote(input[i]))
					{
						start = ++i;
						if (input[i] && is_quote(input[i]))
							continue ;
						while (input[i] && !is_quote(input[i]))
							i++;
						word = ft_strjoin(word, strndup(&input[start], i - start));
						i++;
					}
				}
				add_token(&tokens, create_token(word, TOKEN_WORD));
				free(word);
			}
		}
	}
	return (tokens);
}

/* helper function: create a new redirect node */
t_redir *create_new_redir(char *file, int type)
{
	t_redir *redir;

	redir = ft_malloc(sizeof(t_redir));
	redir->file = ft_strdup(file);
	redir->type = type;
	redir->next = NULL;
	return (redir);
}

/* helper function: add a redirect to the redirection list */
void	add_redir(t_cmd *cmd, char *file, int type)
{
	t_redir *new_redir;
	t_redir *temp;

	new_redir = create_new_redir(file, type);
	if (!new_redir)
		return ;
	if (type == TOKEN_REDIRECT_IN)
	{
		if (cmd->infile)
			free(cmd->infile);
		cmd->infile = ft_strdup(file);
	}
	if (type == TOKEN_REDIRECT_APPEND || type == TOKEN_REDIRECT_OUT)
	{
		if (cmd->outfile)
			free(cmd->outfile);
		cmd->outfile = ft_strdup(file);
		cmd->append = (type == TOKEN_REDIRECT_APPEND);
	}
	if (!cmd->redirects)
		cmd->redirects = new_redir;
	else
	{
		temp = cmd->redirects;
		while (temp->next)
			temp = temp->next;
		temp->next = new_redir;
	}
}

/* helper function: free the redirect list */
void free_redir(t_redir *redir)
{
	t_redir *temp;

	while (redir)
	{
		temp = redir;
		free(redir->file);
		redir = redir->next;
		free(temp);
	}
}

t_cmd	*create_new_cmd(void)
{
	t_cmd	*cmd = calloc(1, sizeof(t_cmd));

	if (!cmd)
		return NULL;
	cmd->args = calloc(64, sizeof(char *));
	if (!cmd->args)
	{
		free(cmd);
		return (NULL);
	}
	cmd->args[0] = NULL;
	cmd->infile = NULL;
	cmd->outfile = NULL;
	cmd->append = 0;
	cmd->heredoc = 0;
	cmd->delimiter = NULL;
	cmd->fd_in = 0;
	cmd->redirects = NULL;
	cmd->in_pipe = 0;
	cmd->next = NULL;
	return cmd;
}

void	free_cmds(t_cmd *cmds)
{
	t_cmd	*tmp;
	int		i;

	while (cmds)
	{
		tmp = cmds;
		i = 0;
		while (cmds->args && cmds->args[i])
			free(cmds->args[i++]);
		free(cmds->args);
		free(cmds->infile);
		free(cmds->outfile);
		free(cmds->delimiter);
		free_redir(cmds->redirects);
		cmds = cmds->next;
		free(tmp);
	}
}

t_cmd *parse_tokens(t_token *tokens)
{
    t_cmd *cmd_head = NULL;
    t_cmd *current_cmd = NULL;
    int i;

    if (!validate_tokens(tokens))
        return NULL;

    while (tokens)
    {
        if (!current_cmd)
        {
            current_cmd = create_new_cmd(); // 创建一个新的命令
            if (!cmd_head)
                cmd_head = current_cmd;
        }
		// printf("debug parser: token = %s, type = %d\n", tokens->value, tokens->type);
        if (tokens->type == TOKEN_WORD
			|| tokens->type == TOKEN_SINGLE_QUOTE
			|| tokens->type == TOKEN_DOUBLE_QUOTE
			|| tokens->type == TOKEN_COMMAND)
        {
            i = 0;
			/* only add to args if it's not going to be used as a redirect targets */
			if ((i == 0 || !tokens->next || (tokens->next->type != TOKEN_REDIRECT_IN
					&& tokens->next->type != TOKEN_REDIRECT_OUT
					&& tokens->next->type != TOKEN_REDIRECT_APPEND))
					&& tokens->value[0] != '\0') // skip empty values
				{
					while (current_cmd->args[i]) // 依次找到当前命令中空闲的位置
						i++;
					current_cmd->args[i] = strdup(tokens->value); // 将当前词添加到命令的参数中
				}
        }
        /* output redirect */
		else if (tokens->type == TOKEN_REDIRECT_OUT)
		{
			tokens = tokens->next;
			if (tokens && (tokens->type == TOKEN_WORD || tokens->type == TOKEN_DOUBLE_QUOTE || tokens->type == TOKEN_SINGLE_QUOTE))
			{
				add_redir(current_cmd, tokens->value, TOKEN_REDIRECT_OUT);
				// current_cmd->outfile = ft_strdup(tokens->value);
				// current_cmd->append = 0;
				// printf("debug parser: set outfile = %s\n", tokens->value);
			}
		}
		else if (tokens->type == TOKEN_REDIRECT_APPEND)
		{
			tokens = tokens->next;
			if (tokens && (tokens->type == TOKEN_WORD || tokens->type == TOKEN_DOUBLE_QUOTE || tokens->type == TOKEN_SINGLE_QUOTE))
			{
				add_redir(current_cmd, tokens->value, TOKEN_REDIRECT_APPEND);
				// current_cmd->outfile = ft_strdup(tokens->value);
				// current_cmd->append = 1;
				// printf("debug parser: set outfile (append) = %s\n", tokens->value);
			}
		}
		/* input redirect */
		else if (tokens->type == TOKEN_REDIRECT_IN)
		{
			tokens = tokens->next;
			if (tokens && (tokens->type == TOKEN_WORD || tokens->type == TOKEN_DOUBLE_QUOTE || tokens->type == TOKEN_SINGLE_QUOTE))
			{
				add_redir(current_cmd, tokens->value, TOKEN_REDIRECT_IN);
				// current_cmd->infile = ft_strdup(tokens->value);
				// printf("debug parser: set infile = %s\n", tokens->value);
			}	
		}
        else if (tokens->type == TOKEN_PIPE) // 如果遇到管道符号，创建新的命令
        {
            if (tokens->next && (tokens->next->type == TOKEN_COMMAND || tokens->next->type == TOKEN_SINGLE_QUOTE))
            {
                current_cmd->next = create_new_cmd(); // 创建新的命令块
                current_cmd = current_cmd->next; // 设置当前命令为新的命令块
				// printf("debug parser: created new cmd for pipe\n");
            }
        }
		/* add case for heredoc separately
			if the token after ">>" exits, meaning there is a delimiter
			- put heredoc as 1
			- append token value as the delimiter
		*/
		else if (tokens->type == TOKEN_HEREDOC)
		{
			tokens = tokens->next;
			if (tokens && (tokens->type == TOKEN_WORD || tokens->type == TOKEN_DOUBLE_QUOTE || tokens->type == TOKEN_SINGLE_QUOTE))
			{
				current_cmd->heredoc = 1;
				current_cmd->delimiter = ft_strdup(tokens->value);
				// printf("debug parser: set heredoc delimiter = %s\n", tokens->value);
			}
		}
		tokens = tokens->next;
    }

    return cmd_head;
}

void	check_format_command(t_token *tokens)
{
	while (tokens)
	{
		if (tokens->type == TOKEN_WORD)
		{
			if ((strcmp(tokens->value, "cd") == 0)
				|| (strcmp(tokens->value, "ls") == 0)
				|| (strcmp(tokens->value, "cp") == 0)
				|| (strcmp(tokens->value, "mv") == 0)
				|| (strcmp(tokens->value, "rm") == 0)
				|| (strcmp(tokens->value, "mkdir") == 0)
				|| (strcmp(tokens->value, "rmdir") == 0)
				|| (strcmp(tokens->value, "man") == 0)
				|| (strcmp(tokens->value, "more") == 0)
				|| (strcmp(tokens->value, "cat") == 0)
				|| (strcmp(tokens->value, "echo") == 0)
				|| (strcmp(tokens->value, "grep") == 0)
				|| (strcmp(tokens->value, "pwd") == 0)
				|| (strcmp(tokens->value, "export") == 0)
				|| (strcmp(tokens->value, "unset") == 0)
				|| (strcmp(tokens->value, "env") == 0)
				|| (strcmp(tokens->value, "exit") == 0))
				tokens->type = TOKEN_COMMAND;
		}
		tokens = tokens->next;
	}
}
