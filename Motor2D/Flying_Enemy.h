#ifndef __FLYING_ENEMY_H__
#define __FLYING_ENEMY_H__

#include "Entity.h"
#include "p2DynArray.h"
#include "p2Point.h"
enum Direction;

class Flying_Enemy : public Entity
{
private:

	Animation fly;

public:
	iPoint originalpos;

	Flying_Enemy(int x, int y);
	void Update(float dt);
	void Movement();
	bool CanStartMovement();
	void GetOffset();
	void UpdateSpeed();

	iPoint spawn;

	bool found;
	bool canmove;
	bool agro;

	int radius;
	int big_radius;
	uint anim_speed; 


	p2DynArray<iPoint> fly_path;
	iPoint player_position;
	iPoint enemy_position;

	int path_index;
	bool omw = false;
	bool dead = false;
	float dt;

	Collider* collider = nullptr;


	//Animations
	Animation* load_anim = nullptr;


};

#endif
