#include <stdio.h>
#include <stdlib.h>

static int verify_input(int argc, char *argv[])
{
	if (argc != 4)
	{
		char *msg = "Invalid number of arguments!\n";
		for (int i = 0; msg[i]; i++)
			putchar(msg[i]);
		return (0);
	}
	if (atoi(argv[1]) == 0 && !(argv[1][0] == '0' && argv[1][1] == '\0'))
	{
		char *msg = "Invalid width!\n";
			for (int i = 0; msg[i]; i++)
				putchar(msg[i]);
			return (0);
	}
	if (atoi(argv[2]) == 0 && !(argv[2][0] == '0' && argv[2][1] == '\0'))
	{
		char *msg = "Invalid height!\n";
		for (int i = 0; msg[i]; i++)
			putchar(msg[i]);
		return (0);
	}
	if (atoi(argv[3]) == 0 && !(argv[3][0] == '0' && argv[3][1] == '\0'))
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
			grid[i][j] = 'x';
		grid[i][width] = '\0';
	}
	grid[height] = NULL;
	return (grid);
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
static void free_grid(char **grid, int height, int width)
{
	for (int i = 0; i < height; i++)
		free(grid[i]);
	free(grid);
}
int main (int argc, char *argv[])
{
	if (!verify_input(argc, argv))
		return (1);
	
		int width = atoi(argv[1]);
	int height = atoi(argv[2]);
	int iterations = atoi(argv[3]);

	char **grid = create_grid(width, height);
	
	// DEBUG
	print_grid(grid, height, width);


	// FREE
	free_grid(grid, height, width);
	return (0);
}
