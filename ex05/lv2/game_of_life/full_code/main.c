#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

static int validate_input(int argc, char *argv[])
{
	if (argc != 4)
		return (0);
	int width = atoi(argv[1]);
	int height = atoi(argv[2]);
	int iter = atoi(argv[3]);
	if (width < 1)
		return (0);
	if (height < 1)
		return (0);
	if (iter < 0)
		return (0);
	if (iter == 0 && !(argv[3][0] == '0' && argv[3][1] == '\0'))
		return (0);
	return (1);
}

static char **create_grid(int width, int height)
{
	char **grid = malloc((height + 1) * sizeof(char*));
	for (int i = 0; i < height; i++)
	{
		grid[i] = malloc((width + 1)* sizeof(char));
		for (int j = 0; j < width; j++)
			grid[i][j] = ' ';
		grid[i][width] = '\0';
	}	
	grid[height] = NULL;
	return (grid);
}

static void print_grid(char **grid)
{
	for (int i = 0; grid[i]; i++)
	{
		for (int j = 0; grid[i][j]; j++)
			putchar(grid[i][j]);
		putchar('\n');
	}
}

static void free_grid(char **grid)
{
	if (grid)
	{
		for (int i = 0; grid[i]; i++)
			free(grid[i]);
		free(grid);
	}
}

static char *get_inputs()
{
	char input;
	char *inputs = NULL;
	size_t len = 0;
	int bytes_read = 0;
	while ((bytes_read = read(0, &input, sizeof(char))) > 0)
	{
		if (input == 'w' || input == 'a' || input == 's' || input == 'd' || input == 'x')
		{
			if (!inputs)
				inputs = malloc(sizeof(char) * 2);
			else
				inputs = realloc(inputs, sizeof(char) * (len + 2));
			inputs[len] = input;
			len++;
		}
	}
	if (bytes_read < 0)
	{
		if (inputs)
			free(inputs);
		return (NULL);
	}
	if (bytes_read == 0)
	{
		if (inputs)
			inputs[len] = '\0';
	}
	return (inputs);
}

static void setup_game(char **grid, char *inputs)
{
	int y = 0;
	int x = 0;
	int write_flag = 0;

	for (int i = 0; inputs[i]; i++)
	{
		if (inputs[i] == 'x')
		{
			if (!write_flag)
				write_flag = 1;
			else
				write_flag = 0;
		}
		else if (inputs[i] == 'w' && y != 0)
			y--;
		else if (inputs[i] == 'a' && x != 0)
			x--;
		else if (inputs[i] == 's'&& grid[y + 1])
			y++;
		else if (inputs[i] == 'd' && grid[y][x + 1])
			x++;
		if (write_flag)
			grid[y][x] = '0';
	}
}

static int count_neighbors(char **grid, int y, int x)
{
	int c = 0;

	// top left
	if (y != 0 && x != 0)
		if (grid[y - 1][x - 1] == '0')
			c++;
	// top
	if (y != 0)
		if (grid[y - 1][x] == '0')
			c++;
	// top right
	if (y != 0)
		if (grid[y - 1][x + 1] == '0')
			c++;
	// left
	if (x != 0)
		if (grid[y][x - 1] == '0')
			c++;
	// right
	if (grid[y][x + 1] == '0')
		c++;
	// bottom left
	if (grid[y + 1] && x != 0)
		if (grid[y + 1][x - 1] == '0')
			c++;
	// bottom
	if (grid[y + 1])
		if (grid[y + 1][x] == '0')
			c++;
	// bottom right
	if (grid[y + 1])
		if (grid[y + 1][x + 1] == '0')
			c++;
	return (c);
}

static void process_game(char ***grid, int width, int height)
{
	int i = 0;
	char **copy = create_grid(width, height);
	for (; (*grid)[i]; i++)
	{
		int j = 0;
		for (; (*grid)[i][j]; j++)
			copy[i][j] = (*grid)[i][j];
	}
	for (int y = 0; (*grid)[y]; y++)
	{
		for (int x = 0; (*grid)[y][x]; x++)
		{
			int neighbors = count_neighbors((*grid), y, x);
			// underpopulation
			if (neighbors < 2 && (*grid)[y][x] == '0')
				copy[y][x] = ' ';
			// overpopulation
			if (neighbors > 3 && (*grid)[y][x] == '0')
				copy[y][x] = ' ';
			// generation
			if (neighbors == 3 && (*grid)[y][x] == ' ')
				copy[y][x] = '0';
		}	
	}
	free_grid(*grid);
	*grid = copy;
}

int main (int argc, char *argv[])
{
	if (validate_input(argc, argv) != 1)
		return (1);
	int width = atoi(argv[1]);
	int height = atoi(argv[2]);
	int iter = atoi(argv[3]);

	char **grid = create_grid(width, height);
	char *inputs = get_inputs();

	if (inputs)
	{
		setup_game(grid, inputs);
		for (int i = 0; i < iter; i++)
			process_game(&grid, width, height);
	}

	print_grid(grid);

	if (inputs)
		free(inputs);
	free_grid(grid);
	return (0);
}
