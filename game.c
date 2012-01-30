#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <assert.h>

int maxx =	94;
int maxy =	35;

int minx =	0;
int miny =	0;

#define SLEEPT	120000

int	end = 0;

#define ARRAYSIZE(a)	(sizeof(a)/sizeof(a[0]))

//XXX 18+ text mode gfx, you were warned!

char *ship[] = {
	"  /.\\ ",
	"  |0|  ",
	" /V V\\",
};

char *mothership[] = {
    "  \\    +    / ",
    "   \\  /w\\  / ",
    "    \\/www\\/  ",
    "    /wwwww\\   ",
    "   /WWWWWWW\\  ",
    "  |OOOOOOOOO|  ",
    "   \\MMMMMMM/  ",
    "    \\mmmmm/   ",
    "    /\\mmm/\\  ",
    "   /  \\m/  \\ ",
    "  /   /+\\   \\"

};

char *mothership_dead[] = {
    "  \\        _ ",
    "    /w@\\  / ",
    "    \\/www@\\/  ",
    "   @ /wwwww\\   ",
    "   /  @W  WWW\\  ",
    "  |OOO  OOO@ OOO|  ",
    "   \\MMMMM/ @ ",
    "    \\mm@mmm/ @  ",
    "    /@\\ @ mmm/\\  ",
    "   /  \\m/  \\ ",
    "    \\\\   \\"
};

#define MOTHERSHIP_SPEED	20
#define MOTHERSHIP_LIVE		20

char *block[] = {

	"+-----------+",
	"|           |",
	"|   \\o/    |",
	"|     0     |",
	"|    / \\   |",
	"|           |",
	"+-----------+"
};

struct unit {
	int x, y;
	int shots;
	int step;
	struct shot {
		int x, y;
		int type;
	} shot[10];
	char	*name;
	char	**gfxdat;
	char	**gfxdat_dead;
	int	layers;
	int	width;
	int	health;
};

struct unit		user;
#define USERHEAT	10
struct unit		ai[10];

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

void ai_move()
{
	int j = 0;
	if (!--ai[j].step)
		ai[j].step = MOTHERSHIP_SPEED;
	else
		return;

	if (ai[j].y++ > maxy)
		ai[j].y = 0;
}

void draw_units()
{
	erase();

	int i;
	for (i = 0; i < user.layers; i++)
		mvprintw(user.y + i, user.x, user.gfxdat[i]);

	int j = 0;
	for (i = 0; i < ai[j].layers; i++)
		mvprintw(ai[j].y + i, ai[j].x, ai[j].gfxdat[i]);

	ai_move();
}

void moveshot(int n)
{
	assert(n >= 0 && n < user.shots);

	int i;
	if (user.shot[n].y > 0)
		user.shot[n].y--;
	else {
		for (i = n; i < user.shots; i++) {
			user.shot[i] = user.shot[i+1];
		}
		user.shots--;
	}
}

void check_collision(struct shot shot, struct unit target)
{
	if (shot.y == 0)
	{
		
		if (--target.health == 0) {
			target.gfxdat = target.gfxdat_dead;
		}
	}
}

void update_shots()
{
	mvprintw(0, 0, "mothership:%d\n", ai[0].health);
	int i;
	for (i = 0; i < user.shots; i++) {
		moveshot(i);
		mvaddch(user.shot[i].y, user.shot[i].x, user.shot[i].type);
		check_collision(user.shot[i], ai[0]);
	}
	refresh();

}

void fire(int type)
{
	int i;
	if (type == laser) {
		for (i = user.y; i > 0; i--)
			mvaddch(i - 1, user.x + user.width / 2, '|');
	}
	else {
		// make space for new shot
		int allhot = 1;
		for (i = 0; i < user.shots; i++) {
			if (user.shot[i].y <= 0) {
				allhot = 0;
				break;
			}
		}
		if (allhot && user.shots < USERHEAT - 1) {
			allhot = 0;
			user.shots++;
		}
		assert(user.shots < USERHEAT);
		if (allhot) {
			// cool down...
			return;
		}
		// we good, fire...
		user.shot[i].x = user.x + user.width / 2;
		user.shot[i].y = user.y + 1;
		user.shot[i].type = type;
		mvaddch(user.shot[i].y, user.shot[i].x, user.shot[i].type);
	}

	refresh();
}

bool quit()
{
	int c = 0;
	while ((c = getch()) != 'y' || c != 'n')
		mvprintw(35, 0, "Quit (y/n): \n");
	if (c == 'y')
		return true;
	return false;
}

void userctl(int key)
{
	switch (key) 
	{
	case 'q':
		//if (quit())
			end = 1;
		break;
	case 'w':
		user.y -= user.y > miny ? user.step : 0;
		break;
	case 's':
		user.y += user.y < maxy ? user.step : 0;
		break;
	case 'a':
		user.x -= user.x > minx ? user.step : 0;
		break;
	case 'd':
		user.x += user.x < maxx ? user.step : 0;
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
	user.x		= 50;
	user.y		= 35;
	user.shots	= 0;
	user.name	= "real people";
	user.gfxdat	= ship;
	user.layers	= ARRAYSIZE(ship);
	user.width	= strlen(user.gfxdat[0]);
	user.step	= 2;
	user.health	= 100;

	int j = 0;
	ai[j].x		= 15;
	ai[j].y		= -5;
	ai[j].name		= "mother ship";
	ai[j].gfxdat	= mothership;
	ai[j].gfxdat_dead	= mothership_dead;
	ai[j].layers	= ARRAYSIZE(mothership);
	ai[j].width	= strlen(user.gfxdat[0]);
	ai[j].step	= 1;
	ai[j].health	= MOTHERSHIP_LIVE;


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

