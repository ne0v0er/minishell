/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   loop_util1.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: auzou <auzou@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/21 13:57:37 by auzou             #+#    #+#             */
/*   Updated: 2025/03/21 14:13:46 by auzou            ###   ########.fr       */
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

	tokens = NULL;
	i = 0;
	while (input[i])
	{
		while (ft_isspace(input[i]))
			i++;
		if (is_special_char(input[i]))
			tokenize_util1(tokens, &i, input);
		else if (input[i] == '|')
			tokenize_util2(tokens, &i, input);
		else if (input[i] == '\'' || input[i] == '"')
		{
			if (tokenize_util3(tokens, &i, input) == 1)
				return (NULL);
		}
		else
		{
			tokenize_util4(&tokens, &i, input);
		}
	}
	return (tokens);
}


t_redir	*create_new_redir(char *file, int type)
{
	t_redir	*redir;

	redir = ft_malloc(sizeof(t_redir));
	redir->file = ft_strdup(file);
	redir->type = type;
	redir->next = NULL;
	return (redir);
}

void	add_redir(t_cmd *cmd, char *file, int type)
{
	t_redir	*new_redir;
	t_redir	*temp;

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

void	free_redir(t_redir *redir)
{
	t_redir	*temp;

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
		return (NULL);
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
	return (cmd);
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

t_cmd	*parse_tokens(t_token *tokens)
{
	t_cmd	*cmd_head = NULL;
	t_cmd	*current_cmd = NULL;
	int		i;

	if (!validate_tokens(tokens))
		return (NULL);
	while (tokens)
	{
		if (!current_cmd)
		{
			current_cmd = create_new_cmd();
			if (!cmd_head)
				cmd_head = current_cmd;
		}
		if (tokens->type == TOKEN_WORD
			|| tokens->type == TOKEN_SINGLE_QUOTE
			|| tokens->type == TOKEN_DOUBLE_QUOTE
			|| tokens->type == TOKEN_COMMAND)
		{
			i = 0;
			if ((i == 0 || !tokens->next
					|| (tokens->next->type != TOKEN_REDIRECT_IN
						&& tokens->next->type != TOKEN_REDIRECT_OUT
						&& tokens->next->type != TOKEN_REDIRECT_APPEND))
				&& tokens->value[0] != '\0')
			{
				while (current_cmd->args[i])
					i++;
				current_cmd->args[i] = strdup(tokens->value);
			}
		}
		else if (tokens->type == TOKEN_REDIRECT_OUT)
		{
			tokens = tokens->next;
			if (tokens && (tokens->type == TOKEN_WORD
					|| tokens->type == TOKEN_DOUBLE_QUOTE
					|| tokens->type == TOKEN_SINGLE_QUOTE))
				add_redir(current_cmd, tokens->value, TOKEN_REDIRECT_OUT);
		}
		else if (tokens->type == TOKEN_REDIRECT_APPEND)
		{
			tokens = tokens->next;
			if (tokens && (tokens->type == TOKEN_WORD
					|| tokens->type == TOKEN_DOUBLE_QUOTE
					|| tokens->type == TOKEN_SINGLE_QUOTE))
				add_redir(current_cmd, tokens->value, TOKEN_REDIRECT_APPEND);
		}
		else if (tokens->type == TOKEN_REDIRECT_IN)
		{
			tokens = tokens->next;
			if (tokens && (tokens->type == TOKEN_WORD
					|| tokens->type == TOKEN_DOUBLE_QUOTE
					|| tokens->type == TOKEN_SINGLE_QUOTE))
				add_redir(current_cmd, tokens->value, TOKEN_REDIRECT_IN);
		}
		else if (tokens->type == TOKEN_PIPE)
		{
			if (tokens->next && (tokens->next->type == TOKEN_COMMAND
					|| tokens->next->type == TOKEN_SINGLE_QUOTE))
			{
				current_cmd->next = create_new_cmd();
				current_cmd = current_cmd->next;
			}
		}
		else if (tokens->type == TOKEN_HEREDOC)
		{
			tokens = tokens->next;
			if (tokens && (tokens->type == TOKEN_WORD
					|| tokens->type == TOKEN_DOUBLE_QUOTE
					|| tokens->type == TOKEN_SINGLE_QUOTE))
			{
				current_cmd->heredoc = 1;
				current_cmd->delimiter = ft_strdup(tokens->value);
			}
		}
		tokens = tokens->next;
	}
	return (cmd_head);
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
