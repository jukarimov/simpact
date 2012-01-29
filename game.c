#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>

int maxx =	94;
int maxy =	35;

int minx =	0;
int miny =	0;

#define SLEEPT	120000

int	end = 0;
int	step = 2;

#define ARRAYSIZE(a)	(sizeof(a)/sizeof(a[0]))

char *ship[] = {
	" /.\\\n",
	" |0|\n",
	"/ V \\\n",
};

char *meteor[] = {
      "     .  \n"
      "    /o\\\n",
      "   /oo\\\n",
      "  /oooo\\\n",
      " /OOOOOO\\\n",
      "|OOOOOOOO|\n",
      " \\OOOOOO/\n",
      "  \\oooo/\n",
      "   \\oo/\n",
      "    \\o/\n",
      "      `\n"
};

struct unit {
	int x, y;
	int shots;
	struct {
		int x, y;
		int type;
	} shot[100];
	char	*name;
	char	**gfxdat;
	int	layers;
};

struct unit	user;
struct unit	ai;

int arms[] = { '.', ':', '|', '!', '^', '#', '*', 'o', 'i', 'A', 'x' };

enum {
	gun,
	gun2,
	laser,
	shell,
	bomb1,
	bomb2,
	bomb3,
	bomb4,
	bomb5,
	bomb6,
	bomb7,
};



/* ========================================================================= */

int opendsp()
{
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    scrollok(stdscr, FALSE);
    curs_set(0);
    return 1;
}

void closedsp()
{
    flushinp();
    erase();
    refresh();
    endwin();
}

void draw_units()
{
	erase();

	int i;
	for (i = 0; i < user.layers; i++)
		mvprintw(user.y + i, user.x, user.gfxdat[i]);

	for (i = 0; i < ai.layers; i++)
		mvprintw(ai.y + i, ai.x, ai.gfxdat[i]);
}

void moveshot(int n)
{
	if (n < 0 || n > user.shots) {
		erase();
		mvprintw(0, 0, "wtf? n:%d\n", n);
		return;
	}

	if (user.shot[n].y > 0)
		user.shot[n].y--;
	else
		user.shots--;
}

void update_shots()
{
	int i;
	for (i = 0; i < user.shots; i++) {
		moveshot(i);
		mvaddch(user.shot[i].y, user.shot[i].x, user.shot[i].type);
	}
	refresh();
}

void fire(int type)
{
	int i;
	if (type == laser) {
		for (i = user.y; i > 0; i--)
			mvaddch(i - 1, user.x + 2, '|');
	}
	else {
		i = user.shots++;
		user.shot[i].x = user.x + 2;
		user.shot[i].y = user.y + 1;
		user.shot[i].type = type;
		mvaddch(user.shot[i].y, user.shot[i].x, user.shot[i].type);
	}

	refresh();
}

void userctl(int key)
{
	switch (key) 
	{
	case 'q':
		end = 1;
		break;
	case 'w':
		user.y -= user.y > miny ? step : 0;
		break;
	case 's':
		user.y += user.y < maxy ? step : 0;
		break;
	case 'a':
		user.x -= user.x > minx ? step : 0;
		break;
	case 'd':
		user.x += user.x < maxx ? step : 0;
		break;
	case ' ':
		fire(laser);
		break;
	case 10:
		fire(arms[7]);
		break;
	}
}

int main()
{
	user.x = 50;
	user.y = 35;
	user.shots = 0;
	user.name = "real people";
	user.gfxdat = ship;
	user.layers = ARRAYSIZE(ship);

	ai.x = 15;
	ai.y = 5;
	ai.name = "food";
	ai.gfxdat = meteor;
	ai.layers = ARRAYSIZE(meteor);

	if(opendsp())
		fprintf(stderr, "curses init failed\n");

	while (!end)
	{
		draw_units();
		userctl(getch());
		update_shots();
		usleep(SLEEPT);
	}

	closedsp();
	return 0;
}

