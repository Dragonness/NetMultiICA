#pragma once
#include "util.h"

class Game
{
public:
	// TODO: use checkpoint to make sure we are on the track.
	// Slow speed when not on the track.
	const int num = 8;
	std::vector<sf::Vector2f> checkpoints //Checkpoint locations
	{
		sf::Vector2f(300, 610),
		sf::Vector2f(1270, 430),
		sf::Vector2f(1380, 2380),
		sf::Vector2f(1900, 2460),
		sf::Vector2f(1970, 1700),
		sf::Vector2f(2550, 1680),
		sf::Vector2f(2560, 3150),
		sf::Vector2f(500, 3300)
	};

	std::vector<sf::Color> carColours
	{
		sf::Color::Red,
		sf::Color::Blue,
		sf::Color::Green,
		sf::Color::Yellow,
		sf::Color::White
	};
};



class Car
{
	float accel,
		deccel,
		turnSpeed,
		maxSpeed;

	sf::Color col;
	bool ai;

public:
	sf::Vector2f pos;
	int c; //Current checkpoint
	float speed, 
		angle;

	Car() { 
		speed = 0;
		angle = 0;

		c = 0;
		ai = false;

		maxSpeed = 10.0f;
		accel = 0.3f;
		deccel = 0.3f;
		turnSpeed = 0.07f;
	}

	void Update(); //Updating car positions
	void FindTarget(std::vector<sf::Vector2f> points);
	void Move(); //Player controlls and movement
};