#include <stdio.h>
#include <stdlib.h>

static void free_map(char **map)
{
	if (map)
	{
		for (int i = 0; map[i]; i++)
		{
			if (map[i])
				free(map[i]);
		}
		free(map);
	}
}

static size_t verify_map(char **map, char empty, char obs)
{
	
	size_t len1 = 0;
	for (int i = 0; map[0][i]; i++)
		len1++;
	// 2, not 1, due to newline
	if (len1 < 2)
		return (0);
	for (int i = 0; map[i]; i++)
	{
		size_t len2 = 0;
		for (int j = 0; map[i][j]; j++)
		{
			if (map[i][j] != empty && map[i][j] != obs && map[i][j] != '\n')
				return (0);
			len2++;
		}
		// 2, not 1, due to newline
		if (len2 < 2)
			return (0);
		if (len2 != len1)
			return (0);
	}
	// lazy, but: returns line length (including newline)
	return (len1);
}

static int min(int a, int b, int c)
{
	if (a < b)
	{
		if (a < c)
			return (a);
		else
			return (c);
	}
	else
	{
		if (b < c)
			return (b);
		else
			return (c);
	}
}


static int process_map(FILE *map_stream)
{	
	int read;
	int num_lines;
	char empty;
	char obs;
	char full;

	// newline included so it gets processed and is not part of the getline loop later
	read = fscanf(map_stream, "%d %c %c %c\n", &num_lines, &empty, &obs, &full);
	if (read == EOF || read != 4 || empty == full || empty == obs || full == obs)
		return (0);

	char *line = NULL;
	size_t len = 0;
	int lines_read = 0;

	char **map = malloc((num_lines + 1) * sizeof(char*));
	while ((read = getline(&line, &len, map_stream)) != -1)
	{
		map[lines_read] = malloc((read + 1) * sizeof(char));
		for (int i = 0; line[i]; i++)
			map[lines_read][i] = line[i];
		map[lines_read][read] = '\0';
		lines_read++;
	}
	// fopenf manages its own memory, freeing and allocation on call
	// line needs to be freed once after loop because final call (=final free) doesn't happen
	if (line)
		free(line);

	map[num_lines] = NULL;

	len = verify_map(map, empty, obs);
	if (lines_read != num_lines || num_lines < 1 || len == 0)
	{
		free_map(map);
		return (0);
	}	

	// DP table
	// len -1 to substract newline
	len -= 1;
	int **table = malloc(num_lines * sizeof(int*));
	for (int i = 0; i < num_lines; i++)
		table[i] = calloc(len, sizeof(int));
	
	// scanning
	for (int i = 0; map[i]; i++)
	{
		int j = 0;
		// this assigns each cell a value corresponding their square size potential
		for (; map[i][j]; j++)
		{
			// obstacles ruin square, so the potential here is 0
			if (map[i][j] = obs)
				table[i][j] = 0;
			// borders: one cell = one square
			else if (i == 1 || j == 1)
				table[i][j] = 1;
			else
				// the min arguments are constrains
				// each one means"the square can be at most this big from my direction."
				table[i][j] = min(table[i - 1][j], table[i][j - 1], table[i - 1][j - 1]) + 1;
		}
	}

	// DEBUG
	fprintf(stdout, "DEBUG: map:\n");
	for (int i = 0; map[i]; i++)
		fprintf(stdout, "%s", map[i]);

	free_map(map);
	return (1);
}

int main (int argc, char *argv[])
{
	int res;
	
	if (argc == 1)
	{
		res = process_map(stdin);
		if (!res)
			fprintf(stderr, "map error\n");
	}
	else
	{
		for (int i = 1; i < argc; i++)
		{
			FILE *map_stream = fopen(argv[i], "r");
			if (!map_stream)
				fprintf(stderr, "map error\n");
			else
			{
				res = process_map(map_stream);
				fclose(map_stream);
				if (!res)
					fprintf(stderr, "map error\n");
				else if (i + 1 != argc)
					fprintf(stdout, "\n");
			}
		}
	}
	return (0);
}
