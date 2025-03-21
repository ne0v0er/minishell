/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokenize_utils3.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: auzou <auzou@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 16:43:12 by auzou             #+#    #+#             */
/*   Updated: 2025/03/20 16:43:27 by auzou            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../incl/loop.h"

int	tokenize_util3(t_token *tokens, int *i, char *input)
{
	char	quote;
	char	*quoted;
	int		type;

	quote = input[*i++];
	quoted = extract_quoted(input, i, quote);
	type = 0;
	if (!quoted)
		return (1);
	if (quote == '\'')
		type = TOKEN_SINGLE_QUOTE;
	else
		type = TOKEN_DOUBLE_QUOTE;
	add_token(&tokens, create_token(quoted, type));
	free(quoted);
	return (0);
}