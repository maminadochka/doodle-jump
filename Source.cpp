#include <SFML\Graphics.hpp>
#include <SFML\Audio.hpp>
#include <random>
#include <ctime>
#include <iostream>
#include <chrono>
#include "Source.h"

using namespace std;

const int WINDOW_HEIGHT = 650;
const int WINDOW_WIDTH = 500;
const int PLAYER_LEFT_BOUNDING_BOX = 20;
const int PLAYER_RIGHT_BOUNDING_BOX = 60;
const int PLAYER_BOTTOM_BOUNDING_BOX = 70;
FILE* SCORE_HISTORY_FILE;
char sch[255];
int best_score = 0;
sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Doodle Jump", sf::Style::Close);
sf::RectangleShape gameoverBackground(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
sf::Text gameoverText;
sf::Text scoreText;
sf::Text winText;

int getScoreFromHistory() {
	FILE* fp;
	fp = fopen("score_history.txt", "r");
	char line[1024];
	while (fgets(line, 1024, fp)) {
		return stoi(line);
	};
}

void gameover(int score, int best_score) {
	while (window.isOpen())
	{
		sf::Event event;

		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		if (score > best_score)
		{
			best_score = score;
		}

		window.draw(gameoverBackground);
		window.draw(gameoverText);
		window.draw(winText);
		winText.setPosition(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + WINDOW_HEIGHT / 3);
		window.draw(scoreText);
		window.display();
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
		{
			startGame();
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
		{
			exit(0);
		}
	}
}

void startGame() {
	auto start = chrono::system_clock::now();
	window.setFramerateLimit(70);
	sf::Texture backgroundTexture;
	sf::Texture playerTexture;
	sf::Texture platformTexture;
	backgroundTexture.loadFromFile("C:\\Users\\ksyut\\Desktop\\images\\background2.png");
	playerTexture.loadFromFile("C:\\Users\\ksyut\\Desktop\\images\\doodle2.png");
	platformTexture.loadFromFile("C:\\Users\\ksyut\\Desktop\\images\\platform2.png");
	sf::Sprite background(backgroundTexture);
	sf::Sprite player(playerTexture);
	sf::Sprite platform(platformTexture);

	gameoverBackground.setFillColor(sf::Color::White);

	// game over logic
	sf::Font font;
	font.loadFromFile("C:\\Users\\ksyut\\Desktop\\font\\arial.ttf");
	winText.setFont(font);
	winText.setCharacterSize(20);
	winText.setFillColor(sf::Color::Black);
	scoreText.setFont(font);
	scoreText.setCharacterSize(20);
	scoreText.setPosition(WINDOW_WIDTH - 150, WINDOW_HEIGHT - 630);
	scoreText.setFillColor(sf::Color::Magenta);
	gameoverText.setFont(font);
	gameoverText.setString("\t\t\t\tGame Over!\n\n\nPress R to start new game\nPress E to exit");
	gameoverText.setCharacterSize(30);
	gameoverText.setFillColor(sf::Color::Black);

	// jump sound 
	sf::SoundBuffer buffer;
	buffer.loadFromFile("C:\\Users\\ksyut\\Desktop\\sound\\jump.wav");
	sf::Sound sound;
	sound.setBuffer(buffer);

	// fall sound 
	sf::SoundBuffer buffer1;
	buffer1.loadFromFile("C:\\Users\\ksyut\\Desktop\\sound\\fall.wav");
	sf::Sound sound1;
	sound1.setBuffer(buffer1);

	// initialize platforms
	sf::Vector2u platformPosition[10];
	uniform_int_distribution<unsigned> x(0, WINDOW_WIDTH - platformTexture.getSize().x);
	uniform_int_distribution<unsigned> y(10, WINDOW_HEIGHT);
	default_random_engine e(time(0));

	for (size_t i = 0; i < 10; ++i)
	{
		platformPosition[i].x = x(e);
		platformPosition[i].y = y(e);
	}

	// player's positon and down velocity
	int playerX = 250;
	int playerY = 151;
	float dy = 0;
	int height = 150;
	int score = 0;

	// player's bounding box. It should modify according to the image size

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}
		scoreText.setString("Score: " + to_string(score) + "\nBest score: " + to_string(best_score));

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
			playerX -= 4;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
			playerX += 4;

		// player out of boundary and change the side
		if (playerX > WINDOW_WIDTH)
			playerX = 0;
		if (playerX < 0)
			playerX = WINDOW_WIDTH;

		// score
		if (playerY == height && dy < (-3))
		{
			score += 1;
			if (score > best_score) {
				best_score = score;
			}

			scoreText.setString("Score: " + to_string(score) + "\nBest score: " + to_string(best_score));
		}

		// player's jump mechanism
		dy += 0.2;
		playerY += dy;

		if (playerY < height)
		{
			for (size_t i = 0; i < 10; ++i)
			{
				playerY = height;
				platformPosition[i].y -= dy;  // vertical translation

				if (platformPosition[i].y > WINDOW_HEIGHT) // set new platform on the top
				{
					platformPosition[i].y = 0;
					platformPosition[i].x = x(e);
				}
			}
		}

		// detect jump on the platform
		for (size_t i = 0; i < 10; ++i)
		{
			if ((playerX + PLAYER_RIGHT_BOUNDING_BOX > platformPosition[i].x) &&
				(playerX + PLAYER_LEFT_BOUNDING_BOX < platformPosition[i].x + platformTexture.getSize().x)        // player's horizontal range can touch the platform
				&& (playerY + PLAYER_BOTTOM_BOUNDING_BOX > platformPosition[i].y) &&
				(playerY + PLAYER_BOTTOM_BOUNDING_BOX < platformPosition[i].y + platformTexture.getSize().y)  // player's vertical   range can touch the platform
				&& (dy > 0)) // player is falling
			{
				sound.play();
				dy = -10;
			}
		}
		player.setPosition(playerX, playerY);

		window.draw(background);
		window.draw(player);

		// set and draw platforms
		for (size_t i = 0; i < 10; ++i)
		{
			platform.setPosition(platformPosition[i].x, platformPosition[i].y);
			window.draw(platform);
		}

		// game over
		if (playerY > WINDOW_HEIGHT)
		{
			sound1.play();
			gameoverText.setPosition(30, 200);
			scoreText.setPosition(150, 400);
			if (getScoreFromHistory() < score) {
				winText.setString("you have new best score");
				//	window.draw(winText);
			}
			else {
				winText.setString("previous score was better!");
			};
			SCORE_HISTORY_FILE = fopen("score_history.txt", "wb+");
			if (SCORE_HISTORY_FILE == NULL) {
				cout << "Error opening score history file";
				exit(2);
			}
			fprintf(SCORE_HISTORY_FILE, "%d\n", score);
			gameover(score, best_score);
		}
		window.draw(scoreText);
		window.display();
	}
}


int main()
{
	// bg sound
	sf::SoundBuffer buffer2;
	buffer2.loadFromFile("C:\\Users\\ksyut\\Desktop\\sound\\background.wav");
	sf::Sound sound2;
	sound2.setBuffer(buffer2);
	sound2.play();

	// Game Over
	startGame();
	return 0;
}


// change path