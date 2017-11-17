#include "j1WalkingEnemy.h"
#include "p2Log.h"
#include "j1Textures.h"
#include "j1Render.h"
#include "j1Collision.h"
#include "j1Input.h"
#include "j1App.h"
#include "j1Player.h"
#include "j1Map.h"
#include "j1Window.h"
#include "j1PathFinding.h"
#include "j1Scene.h"
#include "j1FadetoBlack.h"

j1WalkingEnemy::j1WalkingEnemy()
{
	name.create("enemies");
}

j1WalkingEnemy::~j1WalkingEnemy()
{
}

bool j1WalkingEnemy::Awake(pugi::xml_node& config)
{

	LOG("Loading animations");

	bool ret = true;

	animation_file.load_file(config.child("file").attribute("folder").as_string());
	animations = animation_file.child("animations").child("enemies").child("walking_enemy");

	if (animations == NULL)
	{
		LOG("Could not load animations");
		ret = false;
	}

	attributes = animations.child("attributes");
	rect = animations.first_child();

	load_anim = &fly;

	int i = rect.attribute("id").as_int();
	int j = attributes.attribute("size").as_int();

	while (i < j)
	{
		SDL_Rect r;
		r.x = rect.attribute("x").as_int();
		r.y = rect.attribute("y").as_int();
		r.w = rect.attribute("w").as_int();
		r.h = rect.attribute("h").as_int();

		load_anim->PushBack({ r.x,r.y,r.w,r.h });

		rect = rect.next_sibling();
		i = rect.attribute("id").as_int();
		load_anim->loop = attributes.attribute("loop").as_bool();
		load_anim->speed = attributes.attribute("speed").as_float();
		radius = attributes.attribute("radius").as_int();
		big_radius = attributes.attribute("big_radius").as_int();

	}
	animations = animation_file.child("animations").child("enemies");

	return ret;
}


bool j1WalkingEnemy::Start()
{
	bool ret = true;
	LOG("Loading flying enemy");
	speed.x = 2; speed.y = 8;
	r.x = spawn.x; r.y = spawn.y - 36;
	r.w = 63; r.h = 64;
	flip = false;
	found = false;
	back = false;
	agro = false;
	texture = App->tex->Load(animations.child("texture").child("folder").attribute("file").as_string());
	return ret;
}

bool j1WalkingEnemy::CleanUp()
{
	bool ret = true;
	LOG("Unloading enemy");

	App->tex->UnLoad(texture);
	return ret;
}

bool j1WalkingEnemy::Update(float dt)
{
	bool ret = true;

	canmove = CanStartMovement();

	if (canmove)
	{
		enemy_position = App->map->WorldToMap(r.x, r.y);
		player_position = App->map->WorldToMap(App->player->p.x, App->player->p.y);
		path_index = 0;
		found = true;
		canmove = false;
	}

	if (r.y > App->win->height) {
		dead = true;
		this->CleanUp();
	}


	if (!dead) {
		Movement();
	}

	Draw();



	return ret;
}


void j1WalkingEnemy::Draw()
{

	if ((App->scene->level == 0 || App->scene->level == 1) && App->fadetoblack->IsFading() == false)
	{
		if (App->input->GetKey(SDL_SCANCODE_F11) == KEY_DOWN)
		{
			agro = !agro;
		}
		if (agro)
		{
			App->render->DrawQuad({ r.x - 256,r.y - 256,512,512 }, 255, 72, 0, 155);
			App->render->DrawQuad({ r.x - 320,r.y - 320,640,640 }, 255, 150, 108, 155);
		}
		if ((App->scene->level == 0 || App->scene->level == 1) && App->fadetoblack->IsFading() == false)
		{
			iPoint offset;
			offset = GetOffset(offset.x, offset.y);

			flip = GetFlip();
			App->render->Blit(texture, r.x - offset.x, r.y - offset.y, &(fly.GetCurrentFrame()), flip);
		}
	}
}

void j1WalkingEnemy::Movement()
{

	if ((App->scene->level == 0 || App->scene->level == 1) && App->fadetoblack->IsFading() == false)
	{
		if (found == true && dead == false) {
			enemy_position = App->map->WorldToMap(r.x, r.y);
			player_position = App->map->WorldToMap(App->player->p.x, r.y);
			App->pathfinding->CreatePath(enemy_position, player_position, fly_path);

			if (path_index < fly_path.Count())
			{
				iPoint nextTile = App->map->MapToWorld(fly_path[path_index].x, fly_path[path_index].y);

				if (enemy_position.x <= fly_path[path_index].x && r.x < nextTile.x)
				{
					r.x += speed.x;
					omw = true;
				}
				else if (enemy_position.x >= fly_path[path_index].x && r.x > nextTile.x)
				{
					r.x -= speed.x;
					omw = true;
				}
				else
				{
					omw = false;
				}

				if (!omw) {
					path_index += 1;
				}

				if (fly_path[path_index].x == player_position.x && fly_path[path_index].y == player_position.y) {
					path_index = 0;
					dead = true;
				}
			}
		}
	}
	if (App->collision->CheckCollisionDown(r,speed))
	{
		r.y += speed.y;
	}
}

bool j1WalkingEnemy::CanStartMovement()
{
	bool ret = false;
	enemy_position = App->map->WorldToMap(r.x, r.y);
	player_position = App->map->WorldToMap(App->player->p.x, App->player->p.y);

	int i = player_position.x - enemy_position.x;


	if (player_position.x - enemy_position.x >= -radius && player_position.x - enemy_position.x <= radius)
	{
		if (!found)
		{
			ret = true;
		}
	}
	if ((player_position.x - enemy_position.x <= -big_radius || player_position.x - enemy_position.x >= big_radius) && found == true)
	{
		found = false;
		path_index = 0;
		ret = false;
		back = true;
	}
	return ret;
}

iPoint j1WalkingEnemy::GetOffset(int x, int y)
{
	iPoint offset;


	x = animations.child("flying_enemy").child("attributes").attribute("offset_x").as_int(0);
	y = animations.child("flying_enemy").child("attributes").attribute("offset_y").as_int(0);

	offset.x = x;
	offset.y = y;

	return iPoint(offset);
}

bool j1WalkingEnemy::GetFlip()
{
	bool ret = true;
	if (App->player->p.x - r.x > 0)
	{
		ret = false;
	}
	else
	{
		ret = true;
	}
	return ret;
}