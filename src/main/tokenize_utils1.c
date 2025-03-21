/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokenize_utils1.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: auzou <auzou@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/27 21:01:36 by auzou             #+#    #+#             */
/*   Updated: 2025/03/19 19:20:07 by auzou            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../incl/loop.h"

int	ft_isspace(char a)
{
	if (a == ' ' || a == '\t' || a == '\n')
		return (1);
	return (0);
}

int	is_special_char(char c)
{
	return (c == '|' || c == '<' || c == '>' || c == '&');
}

int	is_quote(char c)
{
	return (c == '"' || c == '\'');
}

static void	tokenize_util1_util(t_token *tokens, int *i, char *input, char *op)
{
	add_token(&tokens, create_token(op, get_token_type(op)));
	(*i)++;
	extract_redirect_filename(input, i, &tokens);
}

void	tokenize_util1(t_token *tokens, int *i, char *input)
{
	char	op[3];

	op[0] = input[*i];
	op[1] = 0;
	op[2] = 0;
	if ((input[*i] == '<' || input[*i] == '>') && input[*i + 1] == input[*i])
	{
		op[1] = input[++(*i)];
		tokenize_util1_util(tokens, i, input, op);
	}
	else if (input[*i] == '<' || input[*i] == '>')
		tokenize_util1_util(tokens, i, input, op);
	else if (input[*i] == '&' && input[*i + 1] == '&')
	{
		op[1] = '&';
		(*i)++;
		add_token(&tokens, create_token(op, get_token_type(op)));
		(*i)++;
	}
	else
	{
		add_token(&tokens, create_token(op, get_token_type(op)));
		(*i)++;
	}
}