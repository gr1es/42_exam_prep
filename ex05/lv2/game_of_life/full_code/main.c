#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int verify_input(int argc, char *argv[])
{
	if (argc != 4)
	{
		char *msg = "Invalid number of arguments!\n";
		for (int i = 0; msg[i]; i++)
			putchar(msg[i]);
		return (0);
	}
	if (atoi(argv[1]) < 1)
	{
		char *msg = "Invalid width!\n";
			for (int i = 0; msg[i]; i++)
				putchar(msg[i]);
			return (0);
	}
	if (atoi(argv[2]) < 1)
	{
		char *msg = "Invalid height!\n";
		for (int i = 0; msg[i]; i++)
			putchar(msg[i]);
		return (0);
	}
	if (atoi(argv[3]) < 0 || (atoi(argv[3]) == 0 && !(argv[3][0] == '0' && argv[3][1] == '\0')))
	{
		char *msg = "Invalid iteration info!\n";
		for (int i = 0; msg[i]; i++)
			putchar(msg[i]);
		return (0);
	}
	return (1);
}

static char **create_grid(int width, int height)
{
	char **grid = malloc((height + 1) * sizeof(char*));
	for (int i = 0; i < height; i++)
	{
		grid[i] = malloc((width + 1) * sizeof(char));
		for (int j = 0; j < width; j++)
			grid[i][j] = ' ';
		grid[i][width] = '\0';
	}
	grid[height] = NULL;
	return (grid);
}

static void copy_grid(char **copy, char **grid)
{
	int y = 0;
	for (; grid[y]; y++)
	{
		int x = 0;
		for (; grid[y][x] != '\0'; x++)
			copy[y][x] = grid[y][x];
		copy[y][x] = '\0';
	}
	copy[y] = NULL;
}

static void print_grid(char **grid, int height, int width)
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
			putchar(grid[i][j]);
		putchar('\n');
	}
}
static void free_grid(char **grid, int height)
{
	for (int i = 0; i < height; i++)
		free(grid[i]);
	free(grid);
}

static void process_input(char **grid, int *y, int *x, int *writing_flag, int height, int width, char input)
{
	if (input == 'x')
	{
		if (*writing_flag == 0)
		{			
			*writing_flag = 1;
			grid[(*y)][(*x)] = '0';
		}
		else
			*writing_flag = 0;
	}
	else if (input == 'w' && (*y) != 0)
	{		
		(*y)--;
		if (*writing_flag == 1)
			grid[(*y)][(*x)] = '0';
	}
	else if (input == 'a' && (*x) != 0)
	{
		(*x)--;
		if (*writing_flag == 1)
			grid[(*y)][(*x)] = '0';
	}
	else if (input == 's' && (*y) < height - 1)
	{
		(*y)++;
		if (*writing_flag == 1)
			grid[(*y)][(*x)] = '0';
	}
	else if (input == 'd' && (*x) < width - 1)
	{
		(*x)++;
		if (*writing_flag == 1)
			grid[(*y)][(*x)] = '0';
	}
}

static void play_game(char **grid, int height, int width, char *inputs)
{
	int y = 0;
	int x = 0;
	int writing_flag = 0;
	for (int i = 0; inputs[i]; i++)
		process_input(grid, &y, &x, &writing_flag, height, width, inputs[i]);
}

static size_t count_neighbors(char **grid, int y, int x, int height, int width)
{
	size_t c = 0;
	if (y != 0 && x != 0)
		if (grid[y - 1][x - 1] == '0')
			c++;
	if (y != 0)
	{
		if (grid[y - 1][x] == '0')
			c++;
		if (x + 1 != width && grid[y - 1][x + 1] == '0')
			c++;
	}
	if (x != 0 && grid[y][x - 1] == '0')
		c++;
	if (x + 1 != width && grid[y][x + 1] == '0')
		c++;
	if (y + 1 != height)
	{
		if (x != 0 && grid[y + 1][x - 1] == '0')
			c++;
		if (grid[y + 1][x] == '0')
			c++;
		if (x + 1 != width && grid[y + 1][x + 1] == '0')
			c++;
	}
	return (c);
}

static void process_states(char ***grid, int height, int width)
{
	char **copy = create_grid(width, height);
	copy_grid(copy, *grid);

	for (int y = 0; y < height; y++)
	{
		int x = 0;
		for (; x < width; x++)
		{
			size_t neighbors = count_neighbors(*grid, y, x, height, width);
			// living cell
			if ((*grid)[y][x] == '0')
			{
				// underpopulation and overpopulation
				if (neighbors < 2 || neighbors > 3)
					copy[y][x] = ' ';
			}
			// dead cell
			if ((*grid)[y][x] == ' ')
			// reproduction
				if (neighbors == 3)
					copy[y][x] = '0';
		}
	}
	copy_grid(*grid, copy);
	free_grid(copy, height);
}

static char *get_inputs()
{
	char input;
	char *inputs = NULL;
	int bytes_read;
	size_t len = 0;

	while ((bytes_read = read(0, &input, sizeof(char))) > 0)
	{
		if (!inputs)
			inputs = malloc (sizeof(char) + 2);
		else
			inputs = realloc(inputs, len + 2);
		inputs[len] = input;
		len++;
	}
	if (bytes_read < 0)
	{
		if (inputs)
			free(inputs);
		return (NULL);
	}
	else if (bytes_read == 0)
	{		
		if (inputs)
			inputs[len] = '\0';
	}	
	return (inputs);
	}

int main (int argc, char *argv[])
{
	if (!verify_input(argc, argv))
		return (1);
	
	int width = atoi(argv[1]);
	int height = atoi(argv[2]);
	int iterations = atoi(argv[3]);

	char **grid = create_grid(width, height);
	
	char *inputs = get_inputs();
	if (!inputs)
	{
		print_grid(grid, height, width);
		free_grid(grid, height);
		return (0);
	}

	play_game(grid, height, width, inputs);


	for (int i = 0; i < iterations; i++)
		process_states(&grid, height, width);

	print_grid(grid, height, width);

	free(inputs);
	free_grid(grid, height);
	return (0);
}
