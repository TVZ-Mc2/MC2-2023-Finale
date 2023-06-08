#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <time.h>
#include <cmath>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include "tinyxml2.h"
using namespace std;
using namespace tinyxml2;

bool isSpace(unsigned char c)
{
	return (c == ' ' || c == '\n' || c == '\r' ||
		c == '\t' || c == '\v' || c == '\f');
}

class Brick
{
public:
	char Id;
	int HitPoints;
	int BreakScore;
	int x1, x2, y1, y2;
	string Texture;
	string HitSound;
	string BreakSound;

	//Default constructor
	Brick()
	{
		Id = 0;
		HitPoints = 0;
		BreakScore = 0;
		Texture = "";
		HitSound = "";
		BreakSound = "";
	}

	//Constructor with id
	Brick(char id)
	{
		switch (id)
		{
		case 'S':
			Id = id;
			HitPoints = 1;
			BreakScore = 50;
			Texture = "Textures/Bricks/Soft.dds";
			HitSound = "Sounds/Hit_01.mp3";
			BreakSound = "Sounds/Break_01.mp3";
			break;
		case 'M':
			Id = id;
			HitPoints = 2;
			BreakScore = 100;
			Texture = "Textures/Bricks/Medium.dds";
			HitSound = "Sounds/Hit_02.mp3";
			BreakSound = "Sounds/Break_02.mp3";
			break;
		case 'H':
			Id = id;
			HitPoints = 3;
			BreakScore = 150;
			Texture = "Textures/Bricks/Hard.dds";
			HitSound = "Sounds/Hit_03.mp3";
			BreakSound = "Sounds/Break_03.mp3";
			break;
		case 'I':
			Id = id;
			HitPoints = 10;
			BreakScore = 0;
			Texture = "Textures/Bricks/Impenetrable.dds";
			HitSound = "Sounds/Hit_04.mp3";
			BreakSound = "Sounds/Break_04.mp3";
			break;
		default:
			Id = *"";
			HitPoints = 0;
			BreakScore = 0;
			x1 = x2 = y1 = y2 = -1;
			Texture = "";
			HitSound = "";
			BreakSound = "";
			break;
		}
	}
};

class Ball
{
public:
	float x1, x2, y1, y2, oldx1, oldx2, oldy1, oldy2;
	float fBallDx, fBallDy;
	int fBallSize;
	float fBallSpeedX, fBallSpeedY;
	bool isAlive;

	Ball()
	{
		x1 = x2 = y1 = y2 = oldx1 = oldx2 = oldy1 = oldy2 = 300;
		fBallSize = 7;
		fBallSpeedX = fBallSpeedY = 0;
		fBallDx = fBallDy = 0;
		isAlive = false;
	}

	Ball(float x, float y, int s, int speed, float fAng)
	{
		fBallSize = s;
		x1 = x - (fBallSize) / 1.5;
		x2 = x + (fBallSize) / 1.5;
		y1 = y - (fBallSize) / 1.5;
		y2 = y + (fBallSize) / 1.5;
		oldx1 = x - (fBallSize) / 1.5;
		oldx2 = x + (fBallSize) / 1.5;
		oldy1 = y - (fBallSize) / 1.5;
		oldy2 = y + (fBallSize) / 1.5;

		fBallDx = cos(fAng);
		fBallSpeedX = speed * (1 / abs(fBallDx));
		fBallDy = sin(fAng);
		fBallSpeedY = speed * (1 / abs(fBallDy));
		isAlive = true;
	}
};

class Level
{
public:
	string Name;
	string BackgroundTexture;
	int RowCount;
	int ColumnCount;
	int RowSpacing;
	int ColumnSpacing;
	int bricksLeft;
	vector<Brick> bricks;
	bool isFinished, extraBall, extraBallPicked;

	Level()
	{
		Name = "";
		BackgroundTexture = "";
		RowCount = 0;
		ColumnCount = 0;
		RowSpacing = 0;
		ColumnSpacing = 0;
		bricks = {};
		bricksLeft = 0;
		isFinished = extraBall = extraBallPicked = false;
	}

	Level(XMLDocument* d)
	{
		XMLElement* e = d->FirstChildElement("Level");
		string name = "Name";
		Name = e->Attribute(&name[0]);
		BackgroundTexture = e->Attribute("BackgroundTexture");
		RowCount = e->FindAttribute("RowCount")->IntValue();
		ColumnCount = e->FindAttribute("ColumnCount")->IntValue();
		RowSpacing = e->FindAttribute("RowSpacing")->IntValue();
		ColumnSpacing = e->FindAttribute("ColumnSpacing")->IntValue();
		isFinished = extraBall = extraBallPicked = false;

		XMLElement* brickElement = e->FirstChildElement("Bricks");

		//Get bricks and remove spaces from the string
		string bricksString = (string)brickElement->FirstChild()->ToText()->Value();
		bricksString.erase(remove_if(bricksString.begin(), bricksString.end(), isSpace), bricksString.end());

		bricks = {};
		bricksLeft = 0;

		for (int i = 0; i < ColumnCount * RowCount; i++)
		{
			bricks.push_back(Brick(bricksString[i]));
			if (bricksString[i] != *"I" && bricksString[i] != *"_")
			{
				bricksLeft += 1;
			}
		}
	}
};

class Player
{
public:
	int lives, score;
	bool won;

	Player()
	{
		lives = 4;
		score = 0;
		won = false;
	}
};

int main()
{
	//initialisation of addons
	al_init();
	al_init_font_addon();
	al_init_ttf_addon();
	al_init_primitives_addon();
	srand(time(0));

	//Initialise level
	const char* levels[3] = { "Level1.xml", "Level2.xml", "Level3.xml" };
	int curLevel = 0;
	XMLDocument level1;
	level1.LoadFile(levels[curLevel]);
	Level lvl = Level(&level1);

	//Initialisation of display and other important elements
	ALLEGRO_DISPLAY* display;
	ALLEGRO_EVENT_QUEUE* queue;
	ALLEGRO_FONT* font;
	ALLEGRO_FONT* fontSmall;
	ALLEGRO_TIMER* timer;
	ALLEGRO_TIMER* timer2;
	ALLEGRO_KEYBOARD_STATE keyState;
	ALLEGRO_EVENT event;
	ALLEGRO_SAMPLE* sample = NULL;
	ALLEGRO_SAMPLE* backgroundMusic = NULL;
	ALLEGRO_SAMPLE* winMusic = NULL;
	ALLEGRO_SAMPLE_INSTANCE* sampleInstance = NULL;

	queue = al_create_event_queue();
	timer = al_create_timer(1.0 / 120);
	timer2 = al_create_timer(1.0 / 5);
	display = al_create_display(640, 640);
	font = al_load_ttf_font("VT323.ttf", 64, 0);
	fontSmall = al_load_ttf_font("VT323.ttf", 20, 0);

	al_create_event_queue();
	al_install_keyboard();
	al_install_mouse();
	al_install_audio();
	al_init_acodec_addon();
	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_mouse_event_source());
	al_register_event_source(queue, al_get_display_event_source(display));
	al_register_event_source(queue, al_get_timer_event_source(timer));
	al_register_event_source(queue, al_get_timer_event_source(timer2));

	//Sounds
	al_reserve_samples(10);
	sample = al_load_sample("Sounds/Dead.wav");
	backgroundMusic = al_load_sample("Sounds/Background.mp3");
	winMusic = al_load_sample("Sounds/Cheer.mp3");
	float soundGain = 0.7;
	al_play_sample(backgroundMusic, soundGain, 0, 1, ALLEGRO_PLAYMODE_LOOP, NULL);
	//Width and height of the display
	float width = al_get_display_width(display);
	float height = al_get_display_height(display);

	//Player
	Player player = Player();

	//Bat
	float fBatX = width / 2;
	float fBatY = height - 15;
	float fBatSpeed = 3;
	float fBatWidth = 75;

	//Ball 
	float defaultBallSpeed = 2.5;
	int defaultBallSize = 10;
	vector<Ball> balls;

	float r = rand();
	float fAng = (float)(r / RAND_MAX) * 3.141592;
	balls.push_back(Ball(fBatX + fBatWidth / 2, fBatY, defaultBallSize, defaultBallSpeed, fAng));

	//Extra Ball
	int exBallPickupX = 0;
	int exBallPickupY = 0;
	int exBallPickupSize = 30;

	//Brick size
	int brickSizeX = 25;
	int brickSizeY = 15;

	//Start timer and game loop
	al_start_timer(timer);
	al_start_timer(timer2);
	float oldTime = 0;
	float fps = 0;
	bool running = true;

	while (running)
	{
		al_wait_for_event(queue, &event);

		//Close 
		if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
			running = false;

		al_get_keyboard_state(&keyState);

		//Left-Right movement
		if (al_key_down(&keyState, ALLEGRO_KEY_RIGHT))
		{
			fBatX += fBatSpeed;
		}

		if (al_key_down(&keyState, ALLEGRO_KEY_LEFT))
		{
			fBatX -= fBatSpeed;
		}

		if (al_key_down(&keyState, ALLEGRO_KEY_ESCAPE))
		{
			if (al_get_timer_started(timer))
			{
				al_stop_timer(timer);
			}
			else
			{
				al_resume_timer(timer);
			}
		}

		//Executes each tick
		if (event.type == ALLEGRO_EVENT_TIMER && event.timer.source == timer)
		{
			//Check all balls
			for (int ballNum = 0; ballNum < balls.size(); ballNum++)
			{
				//Old positions are used for direction and collision detection
				if (balls[ballNum].isAlive)
				{
					balls[ballNum].oldx1 = balls[ballNum].x1;
					balls[ballNum].oldx2 = balls[ballNum].x2;
					balls[ballNum].oldy1 = balls[ballNum].y1;
					balls[ballNum].oldy2 = balls[ballNum].y2;
					balls[ballNum].x1 += balls[ballNum].fBallDx * balls[ballNum].fBallSpeedX;
					balls[ballNum].x2 += balls[ballNum].fBallDx * balls[ballNum].fBallSpeedX;
					balls[ballNum].y1 += balls[ballNum].fBallDy * balls[ballNum].fBallSpeedY;
					balls[ballNum].y2 += balls[ballNum].fBallDy * balls[ballNum].fBallSpeedY;
					balls[ballNum].fBallSpeedX = defaultBallSpeed;

					//Edge bounce
					if (balls[ballNum].x2 >= width)
						balls[ballNum].fBallDx *= -1;

					if (balls[ballNum].x1 <= 0)
						balls[ballNum].fBallDx *= -1;

					if (balls[ballNum].y1 <= 0)
						balls[ballNum].fBallDy *= -1;

					if (balls[ballNum].y2 >= height)
					{
						if (player.lives > 0)
						{
							float r = rand();
							float fAng = (float)(r / RAND_MAX) * 3.141592;

							if (ballNum == 0)
							{
								player.lives -= 1;

								if (player.lives != 0)
									balls[ballNum] = Ball(fBatX + fBatWidth / 2, fBatY, defaultBallSize, defaultBallSpeed, fAng);
								else
								{
									sample = al_load_sample("Sounds/Dead.wav");
									al_play_sample(sample, soundGain, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
								}
							}
							else
								balls[ballNum].isAlive = false;
						}
						else
						{
							balls[ballNum] = Ball(fBatX + fBatWidth / 2, fBatY, defaultBallSize, 0, 0);
							al_stop_timer(timer);
						}
					}

					//Brick collision
					for (int i = 0; i < lvl.ColumnCount * lvl.RowCount; i++)
					{
						if (
							(balls[ballNum].x2 >= lvl.bricks[i].x1 && balls[ballNum].x2 <= lvl.bricks[i].x2 &&
								balls[ballNum].y2 >= lvl.bricks[i].y1 && balls[ballNum].y2 <= lvl.bricks[i].y2) ||
								(balls[ballNum].x1 >= lvl.bricks[i].x1 && balls[ballNum].x1 <= lvl.bricks[i].x2 &&
									balls[ballNum].y1 >= lvl.bricks[i].y1 && balls[ballNum].y1 <= lvl.bricks[i].y2))
						{
							if (lvl.bricks[i].HitPoints > 0)
							{
								if (balls[ballNum].oldy2 < lvl.bricks[i].y1) balls[ballNum].fBallDy *= -1;
								else if (balls[ballNum].oldy1 > lvl.bricks[i].y2) balls[ballNum].fBallDy *= -1;
								else if (balls[ballNum].oldx2 < lvl.bricks[i].x1) balls[ballNum].fBallDx *= -1;
								else if (balls[ballNum].oldx1 > lvl.bricks[i].x2) balls[ballNum].fBallDx *= -1;
								else balls[ballNum].fBallDy *= -1;

								if (lvl.bricks[i].Id != *"I")
								{
									lvl.bricks[i].HitPoints -= 1;
									if (lvl.bricks[i].HitPoints == 0)
									{
										sample = al_load_sample(&lvl.bricks[i].BreakSound[0]);
										al_play_sample(sample, soundGain, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
										lvl.bricksLeft -= 1;
										player.score += lvl.bricks[i].BreakScore;
										lvl.bricks[i].Id = *"_";
										lvl.bricks[i].y1 = -1;
										lvl.bricks[i].y2 = -1;
										lvl.bricks[i].x1 = -1;
										lvl.bricks[i].x2 = -1;
									}
									else
									{
										sample = al_load_sample(&lvl.bricks[i].HitSound[0]);
										al_play_sample(sample, soundGain, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
									}
								}
								else
								{
									sample = al_load_sample(&lvl.bricks[i].HitSound[0]);
									al_play_sample(sample, soundGain, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
								}
							}
						}
					}

					//Extra ball with 0.1% chance
					if (!lvl.extraBall && !lvl.extraBallPicked && rand()%1000==1) {
						exBallPickupX = rand() % ((int)(width - 30) - 30 + 1) + 30;
						exBallPickupY = rand() % ((int)(height - 30) - ((int)(height * 3 / 5) + 1) + ((int)(height * 3 / 5)));
						lvl.extraBall = true;
					}
					//Extra ball pickup collision
					if ( !lvl.extraBallPicked &&
						((balls[ballNum].x2 >= exBallPickupX && balls[ballNum].x2 <= exBallPickupX + exBallPickupSize &&
							balls[ballNum].y2 >= exBallPickupY && balls[ballNum].y2 <= exBallPickupY + exBallPickupSize) ||
						(balls[ballNum].x1 >= exBallPickupX && balls[ballNum].x1 <= exBallPickupX + exBallPickupSize &&
							balls[ballNum].y1 >= exBallPickupY && balls[ballNum].y1 <= exBallPickupY + exBallPickupSize)))
					{
						cout << " x:"<< balls[ballNum].x1 << " y:" << balls[ballNum].y1 << "\n";
						cout << " x2:" << balls[ballNum].x2 << " y2:" << balls[ballNum].y2 << "\n";
						cout << " xKocka:" << exBallPickupX << " yKocka:" << exBallPickupY << "\n";
						balls.push_back(Ball(fBatX + fBatWidth / 2, fBatY, defaultBallSize, defaultBallSpeed, fAng));
						lvl.extraBallPicked = true;
					}

					//Bat collision and restrictions
					if (((balls[ballNum].x2 >= fBatX && balls[ballNum].x2 <= fBatX + fBatWidth) ||
						(balls[ballNum].x1 >= fBatX && balls[ballNum].x1 <= fBatX + fBatWidth)) &&
						balls[ballNum].y2 >= fBatY)
					{
						if (balls[ballNum].oldy2 < balls[ballNum].y2)
						{
							balls[ballNum].fBallDx = cos((((balls[ballNum].x2 - (balls[ballNum].fBallSize - (balls[ballNum].fBallSize / 10)) / 2) - fBatX + fBatWidth) / fBatWidth) * 3.141592);
							balls[ballNum].fBallSpeedX *= (1 / abs(balls[ballNum].fBallDx));
							balls[ballNum].fBallDy *= -1;
						}
					}
				}
			}

			if (fBatX + fBatWidth >= width) fBatX = width - fBatWidth;
			if (fBatX <= 0) fBatX = 0;

			//Drawing
			al_clear_to_color(al_map_rgba(1, 1, 1, 1));

			//Background
			int timerCount = al_get_timer_count(timer);
			//al_draw_filled_circle(width / 2, height / 2, timerCount % 400 + 20, al_map_rgb(155, 155, 155));

			al_draw_text(font, al_map_rgb(255, 255, 255), width / 2 - ((strlen(&lvl.Name[0]) + sqrt(strlen(&lvl.Name[0]))) * 64 / 4 / 2), 0, 0, &lvl.Name[0]);

			//Draw ExtraBall pickup square
			if (lvl.extraBall && !lvl.extraBallPicked)
			{
				al_draw_rectangle(exBallPickupX, exBallPickupY, exBallPickupX + exBallPickupSize, exBallPickupY + exBallPickupSize, al_map_rgb(222, 80, 40), 2);
				al_draw_filled_circle(exBallPickupX + exBallPickupSize / 2, exBallPickupY + exBallPickupSize / 2, 7, al_map_rgb(255, 255, 255));
			}

			if (al_get_timer_count(timer) % 10 == 0)
			{
				fps = 10 / (al_get_time() - oldTime);
				oldTime = al_get_time();
				al_draw_text(fontSmall, al_map_rgb(255, 255, 255), 20, 0, 0, &to_string(fps)[0]);

				string txt = "Score:";
				txt += to_string(player.score);
				al_draw_text(fontSmall, al_map_rgb(255, 255, 255), width - 100, 10, 0, &txt[0]);

				string txt2 = "Lives:";
				txt2 += to_string(player.lives);
				al_draw_text(fontSmall, al_map_rgb(255, 255, 255), width - 100, 30, 0, &txt2[0]);
			}
			else
			{
				al_draw_text(fontSmall, al_map_rgb(255, 255, 255), 20, 0, 0, &to_string(fps)[0]);

				string txt = "Score:";
				txt += to_string(player.score);
				al_draw_text(fontSmall, al_map_rgb(255, 255, 255), width - 100, 10, 0, &txt[0]);

				string txt2 = "Lives:";
				txt2 += to_string(player.lives);
				al_draw_text(fontSmall, al_map_rgb(255, 255, 255), width - 100, 30, 0, &txt2[0]);
			}

			for (int ballNum = 0; ballNum < balls.size(); ballNum++)
			{
				if (balls[ballNum].isAlive)
				{
					if (ballNum == 0)
						al_draw_filled_circle(balls[ballNum].x2 - (balls[ballNum].fBallSize) / 1.5, balls[ballNum].y2 - (balls[ballNum].fBallSize) / 1.5, 7, al_map_rgb(255, 255, 0));
					else
						al_draw_filled_circle(balls[ballNum].x2 - (balls[ballNum].fBallSize) / 1.5, balls[ballNum].y2 - (balls[ballNum].fBallSize) / 1.5, 7, al_map_rgb(100, 100, 100));
					
					//Hitbox
					//al_draw_filled_rectangle(balls[ballNum].x1, balls[ballNum].y1, balls[ballNum].x2, balls[ballNum].y2, al_map_rgb(255, 0, 0));	
				}
			}
			
			//Draw Bat
			al_draw_filled_rectangle(fBatX, fBatY, fBatX + fBatWidth, fBatY + 5, al_map_rgb(255, 255, 255));

			//Draw bricks
			int tempBy = height / 4;
			for (int bY = 0; bY < lvl.RowCount; bY++)
			{
				tempBy += lvl.RowSpacing;
				int tempBx = 35;
				for (int bX = 0; bX < lvl.ColumnCount; bX++)
				{
					tempBx += lvl.ColumnSpacing;
					Brick* tempBrick = &lvl.bricks[static_cast<unsigned __int64>(bY) * static_cast<unsigned __int64>(lvl.ColumnCount) + static_cast<unsigned __int64>(bX)];
					tempBrick->x1 = bX * brickSizeX + tempBx;
					tempBrick->x2 = (bX + 1) * brickSizeX + tempBx;
					tempBrick->y1 = bY * brickSizeY + tempBy;
					tempBrick->y2 = (bY + 1) * brickSizeY + tempBy;

					switch (tempBrick->Id)
					{
					case 'S':
						al_draw_filled_rectangle(bX * brickSizeX + tempBx, bY * brickSizeY + tempBy, (bX + 1) * brickSizeX + tempBx,
							(bY + 1) * brickSizeY + tempBy, al_map_rgb(0, 255, 0));
						break;
					case 'M':
						al_draw_filled_rectangle(bX * brickSizeX + tempBx, bY * brickSizeY + tempBy, (bX + 1) * brickSizeX + tempBx,
							(bY + 1) * brickSizeY + tempBy, al_map_rgba(0, 0, 255, tempBrick->HitPoints));
						break;
					case 'H':
						al_draw_filled_rectangle(bX * brickSizeX + tempBx, bY * brickSizeY + tempBy, (bX + 1) * brickSizeX + tempBx,
							(bY + 1) * brickSizeY + tempBy, al_map_rgba(255, 0, 0, 0.5));
						break;
					case 'I':
						al_draw_filled_rectangle(bX * brickSizeX + tempBx, bY * brickSizeY + tempBy, (bX + 1) * brickSizeX + tempBx,
							(bY + 1) * brickSizeY + tempBy, al_map_rgb(122, 122, 122));
						break;
					default:
						break;
					}
				}
			}
			al_flip_display();
		}

		if (lvl.bricksLeft == 0)
		{
			if (curLevel + 1 < 3)
			{
				XMLDocument nextLvl;
				curLevel++;
				balls = {};
				defaultBallSpeed += 0.25;
				balls.push_back(Ball(fBatX + fBatWidth / 2, fBatY, defaultBallSize, defaultBallSpeed, fAng));
				nextLvl.LoadFile(levels[curLevel]);
				lvl = Level(&nextLvl);
			}
			else
			{
				lvl.isFinished = true;
			}
		}

		//Game over
		if (event.type == ALLEGRO_EVENT_TIMER && event.timer.source == timer2)
		{
			if (player.lives == 0)
			{
				al_draw_text(font, al_map_rgb(255, 255, 255), width / 2 - (15 * 64 / 4 / 2), height / 3, 0, "Game Over");
				al_flip_display();
			}

			if (lvl.isFinished)
			{
				al_draw_text(font, al_map_rgb(255, 255, 255), width / 2 - (25 * 64 / 4 / 2), height / 3, 0, "Congratulations!");
				string txt = "Score:";
				txt += to_string(player.score);
				al_draw_text(font, al_map_rgb(255, 255, 255), width / 2 - (15 * 64 / 4 / 2), height / 3 + 55, 0, &txt[0]);
				al_flip_display();
				if (!player.won) {
					al_play_sample(winMusic, soundGain, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
					player.won = true;
				}
				al_stop_timer(timer);
			}
		}
	}

	//Destructor calls
	al_destroy_display(display);
	al_destroy_font(font);
	al_uninstall_keyboard();
	al_uninstall_mouse();
	al_destroy_timer(timer);
	al_destroy_sample(sample);
	al_destroy_sample_instance(sampleInstance);
	al_uninstall_audio();
	return 0;
}