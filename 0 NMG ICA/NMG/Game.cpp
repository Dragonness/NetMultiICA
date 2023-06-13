#include "Game.h"

void Car::Update()
{	
	// if within bounds
	pos.x += sin(angle) * speed;
	pos.y += cos(angle) * speed;
	//else out of bounds
	/// xy += sincos(angle) * speed * 0.5;
}

void Car::FindTarget(std::vector<sf::Vector2f> points) // The AI
{
	float tx = points[c].x;
	float ty = points[c].y;
	float beta = angle - atan2(tx - pos.x, -ty + pos.y);
	if (sin(beta) < 0) angle += 0.005 * speed; else angle -= 0.005 * speed;
	// Check if passed a checkpoint
	if (pow(pos.x - tx, 2) + pow(pos.y - ty, 2) < 625)
	{ c = (c + 1) % points.size(); }
}

void Car::Move()
{
	bool up = false, left = false, down = false, right = false;

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) { up = true; }
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) { left = true; }
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) { down = true; }
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) { right = true; }

	if (up && speed < maxSpeed)
	{
		if (speed < 0) { speed += deccel; }
		else { speed += accel; }
	}
	if (down && speed > maxSpeed)
	{
		if (speed > 0) { speed -= deccel; }
		else { speed -= accel; }
	}

	if (!up && !down)
	{
		if (speed - deccel > 0) { speed -= deccel; }
		else if (speed + deccel < 0) { speed += deccel; }
		else { speed = 0; }
	}

	if (right && speed != 0) { angle += turnSpeed * speed / maxSpeed; }
	if (left && speed != 0) { angle -= turnSpeed * speed / maxSpeed; }
}