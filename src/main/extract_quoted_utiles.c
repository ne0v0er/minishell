/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   extract_quoted_utiles.c                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: auzou <auzou@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 18:23:14 by auzou             #+#    #+#             */
/*   Updated: 2025/03/19 18:37:56 by auzou            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../incl/loop.h"

static int	printf_error(char *quoted)
{
	printf("quote need be close\n");
	free(quoted);
	return (1);
}

int	check_format_quoted(char *quoted, char *input, int *i, char quote)
{
	int	start;

	if (is_quote(input[*i]))
	{
		while (input[*i] && is_quote(input[*i]))
		{
			quote = input[*i];
			start = ++(*i);
			if (input[*i] && quote != input[*i])
			{
				while (input[*i] && quote != input[*i])
					(*i)++;
				if (!input[*i])
					printf_error(quoted);
				quoted = ft_strjoin(quoted, strndup(&input[start], *i - start));
				(*i)++;
			}
			else if (!input[*i])
				printf_error(quoted);
		}
	}
	return (0);
}