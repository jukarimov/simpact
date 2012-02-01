#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>

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
	" /V V\\"
};

char *ship_dd[] = {
	"  %.| ",
	"  /@-- ",
	" /# @\\"
};

char *star[] = {
	"    |   ",
	" -- * --",
	"    |   "
};

char *mothership[] = {
    "  \\    +    / ",
    "   \\  /w\\  / ",
    "    \\/www\\/  ",
    "    /wwwww\\   ",
    "   /WWWWWWW\\  ",
    "--|OOOOOOOOO|--",
    "   \\MMMMMMM/  ",
    "    \\mmmmm/   ",
    "    /\\mmm/\\  ",
    "   /  \\m/  \\ ",
    "  /   /+\\   \\"
};

char *ufo_body[] = {
	"   VV   ",
	"-@-MM-@-"
};
char *ufo_body_dd[] = {
	"_V@_",
	"/@-"
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

struct unit {
	int x, y;
	int shots;
	int step, step_static;
	struct shot {
		int active;
		int x, y;
		int type;
	} shot[10];
	char	*name;
	char	**gfxdat;
	char	**gfxdat_dead;
	int	length;
	int	width;
	int	health;
};

struct unit		user;
#define USERHEAT	10
struct unit		ai[100];

int arms[] = { '.', ':', '|', '!', '^', '#', '*', 'o', 'i', 'A', 'x' };

enum {
	laser,
	arm0,
	arm1,
	arm2,
	arm3,
	arm4,
	arm5,
	arm6,
	arm7,
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

void ai_move(struct unit *u)
{
	if (!--u->step) // to slow down
		u->step = u->step_static;
	else
		return;

	if (u->y++ > maxy && u->health > 0) {
		u->y = -15 * rand() % maxy;
		u->x = rand() % maxx;
	}

	else if (u->health < 0 && !strcmp(u->name,"obi_van")) {
		printf("You were shot\n");
		closedsp();
		exit(0);
	}
}

void draw_units()
{
	erase();

	int i, j;

	for (j = 0; j < 20; j++) {

		for (i = 0; i < ai[j].length; i++)
			mvprintw(ai[j].y + i, ai[j].x, ai[j].gfxdat[i]);

		ai_move(&ai[j]);
	}

	for (i = 0; i < user.length; i++)
		mvprintw(user.y + i, user.x, user.gfxdat[i]);

}

void moveshot(int n)
{
	assert(n >= 0 && n < user.shots);

	int i;
	if (user.shot[n].y > 0)
		user.shot[n].y--;
	else {
		user.shot[n].active = 0;
		for (i = n; i < user.shots; i++) {
			user.shot[i] = user.shot[i+1];
		}
		user.shots--;
	}
}

void check_collision(struct shot *shot, struct unit *target)
{
	if (shot->y > target->y && shot->y < target->y + target->length &&
	    shot->x > target->x && shot->x < target->x + target->width && shot->active == 1)
	{
		shot->active = 0; // already damaged, can't hurt anymore
		shot->type = ' ';
		if (--target->health < 0)
			target->gfxdat = target->gfxdat_dead;
	}
}

void update_shots()
{
	int i, j;
	for (i = 0; i < user.shots; i++) {
		moveshot(i);
		mvaddch(user.shot[i].y, user.shot[i].x, user.shot[i].type);
		for (j = 0; j < 10; j++)
			check_collision(&user.shot[i], &ai[j]);
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
		user.shot[i].active = 1;
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
	user.x			= 50;
	user.y			= 35;
	user.shots		= 0;
	user.name		= "obi van";
	user.gfxdat		= ship;
	user.gfxdat_dead	= ship_dd;
	user.length		= ARRAYSIZE(ship);
	user.width		= strlen(user.gfxdat[0]);
	user.step		= 2;
	user.step_static	= 1;
	user.health		= 100;

	int j = 0;
	ai[j].x			= 15;
	ai[j].y			= -15;
	ai[j].name		= "mother ship";
	ai[j].gfxdat		= mothership;
	ai[j].gfxdat_dead	= mothership_dead;
	ai[j].length		= ARRAYSIZE(mothership);
	ai[j].width		= strlen(mothership[0]);
	ai[j].step		= 1;
	ai[j].step_static	= 1;
	ai[j].health		= 5;

	for (j = 1; j < 10; j++) {
		ai[j].x			= 15;
		ai[j].y			= -5;
		ai[j].name		= "ufo-enemy";
		ai[j].gfxdat		= ufo_body;
		ai[j].gfxdat_dead	= ufo_body_dd;
		ai[j].length		= ARRAYSIZE(ufo_body);
		ai[j].width		= strlen(ufo_body[0]);
		ai[j].step		= 1;
		ai[j].step_static	= 1;
		ai[j].health		= 1;
	}

	for (j = 10; j < 20; j++) {
		ai[j].x			= 15;
		ai[j].y			= -5;
		ai[j].name		= "star";
		ai[j].gfxdat		= star;
		ai[j].length		= ARRAYSIZE(star);
		ai[j].width		= strlen(star[0]);
		ai[j].step		= 1;
		ai[j].step_static	= 1;
		ai[j].health		= 999;
	}

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

