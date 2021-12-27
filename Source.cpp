#include <SFML\Graphics.hpp>
#include <SFML\Audio.hpp>
#include <random>
#include <ctime>
#include <iostream>
#include <chrono>
#include "Source.h"
#include "Graphics.h"

using namespace std;

FILE* SCORE_HISTORY_FILE;
char sch[255];
int best_score = 0;
sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Doodle Jump", sf::Style::Close);
sf::RectangleShape gameoverBackground(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
sf::RectangleShape startScreenBackground(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
sf::Text gameoverText;
sf::Text scoreText;
sf::Text winText;
sf::Text lvlText;

// texture load from file + texture set to object
sf::RectangleShape initObject(string textureFilePath, sf::Vector2f obj_size) {
	sf::RectangleShape game_object;
	sf::Texture game_texture;
	game_texture.loadFromFile(textureFilePath);
	game_object.setSize(obj_size);
	game_object.setTexture(&game_texture);
	return game_object;
}

int getScoreFromHistory() {
	FILE* fp;
	fp = fopen("score_history.txt", "r");
	char line[1024];
	while (fgets(line, 1024, fp)) {
		return stoi(line);
	};
	fclose(fp);
}

sf::Text cookTextObject(sf::Font font, string message, sf::Color color) {
	sf::Text text;
	text.setFont(font);
	text.setCharacterSize(20);
	text.setFillColor(color);
	text.setString(message);
	return text;
}

bool detectJump(int playerX, int playerY, sf::Vector2u platformPosition, sf::Texture platformTexture, int dy) {
	if ((playerX + PLAYER_RIGHT_BOUNDING_BOX > platformPosition.x) &&
		(playerX + PLAYER_LEFT_BOUNDING_BOX < platformPosition.x
			+ platformTexture.getSize().x)        // player's horizontal range can touch the platform
		&& (playerY + PLAYER_BOTTOM_BOUNDING_BOX > platformPosition.y) &&
		(playerY + PLAYER_BOTTOM_BOUNDING_BOX < platformPosition.y
			+ platformTexture.getSize().y)  // player's vertical   range can touch the platform
		&& (dy > 0)) {
		return true;
	}
	else {
		return false;
	}
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

		gameoverBackground.setFillColor(sf::Color::White);
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

void startScreen() {
	sf::Font font;
	font.loadFromFile("font\\arial.ttf");
	sf::Text startText;
	startText.setFont(font);
	startText.setCharacterSize(40);
	startText.setFillColor(sf::Color::Black);
	startText.setPosition(WINDOW_WIDTH - 450, WINDOW_HEIGHT - 100);
	sf::Texture startScreenTexture;
	startScreenTexture.loadFromFile("images\\startScreen.png");
	sf::Sprite background(startScreenTexture);
	startText.setString("Press enter to start");
	window.draw(background);
	window.draw(startText);
	window.display();
	while (window.isOpen()) {
		sf::Event event;

		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) {
			startGame();
		}
	}
}

void startGame() {
	auto start = chrono::system_clock::now();
	window.setFramerateLimit(70);
	sf::CircleShape blackHole;
	sf::Texture backgroundTexture;
	sf::Texture playerTexture;
	sf::Texture platformTexture;
	//sf::Texture blackHoleTexture;
	backgroundTexture.loadFromFile("images\\background.png");
	playerTexture.loadFromFile("images\\doodle.png");
	platformTexture.loadFromFile("images\\platform.png");
	//blackHoleTexture.loadFromFile("images\\hole.png");
	//blackHole.setScale(2, 1.50);
	//blackHole.setTexture(&blackHoleTexture);
	//sf::RectangleShape blackHole = initObject("images\\hole.png", sf::Vector2f(80, 82));
	blackHole.setRadius(30);
	blackHole.setFillColor(sf::Color::Black);
	sf::Sprite background(backgroundTexture);
	sf::Sprite player(playerTexture);
	sf::Sprite platform(platformTexture);

	// text
	sf::Font font;
	font.loadFromFile("font\\arial.ttf");
	//winText = cookTextObject(font, "", sf::Color::Black);
	winText.setFont(font);
	winText.setCharacterSize(20);
	winText.setFillColor(sf::Color::Black);
	//scoreText = cookTextObject(font, "", sf::Color::Magenta);
	scoreText.setFont(font);
	scoreText.setCharacterSize(20);
	scoreText.setPosition(WINDOW_WIDTH - 170, WINDOW_HEIGHT - 630);
	scoreText.setFillColor(sf::Color::Magenta);
	gameoverText.setFont(font);
	gameoverText.setString("\t\t\t\tGame Over!\n\n\nPress R to start new game\nPress E to exit");
	gameoverText.setCharacterSize(30);
	gameoverText.setFillColor(sf::Color::Black);
	lvlText.setFont(font);
	lvlText.setCharacterSize(40);
	lvlText.setFillColor(sf::Color::Magenta);

	// jump sound 
	sf::SoundBuffer buffer;
	buffer.loadFromFile("sound\\jump.wav");
	sf::Sound sound;
	sound.setBuffer(buffer);

	// fall sound 
	sf::SoundBuffer buffer1;
	buffer1.loadFromFile("sound\\fall.wav");
	sf::Sound sound1;
	sound1.setBuffer(buffer1);

	// initialize platforms
	sf::Vector2u platformPosition[10];
	sf::Vector2u blackHolePosition[10];
	uniform_int_distribution<unsigned> x(0, WINDOW_WIDTH - platformTexture.getSize().x);
	uniform_int_distribution<unsigned> y(10, WINDOW_HEIGHT);
	default_random_engine e(time(0));
 
	for (size_t i = 0; i < WINDOW_WIDTH / platformTexture.getSize().x + 1; ++i)
	{
		platformPosition[i].x = x(e);
		platformPosition[i].y = y(e);
	}
	for (size_t i = 0; i < WINDOW_WIDTH / blackHole.getGlobalBounds().width + 1; ++i) {
		blackHolePosition[i].x = x(e);
		blackHolePosition[i].y = y(e);
	}

	// player's positon and down velocity
	int playerX = 250;
	int playerY = 151;
	float dy = 0;
	int height = 150;
	int score = 0;

	//levels
	bool hasLoadedFile = false;
	bool hasLoadedFile2 = false;
	int lvl = 1;
	lvlText.setString("Level " + to_string(lvl));
	lvlText.setPosition(5, 3);
	
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}
		scoreText.setString("Score: " + to_string(score) + "\nBest score: " 
			+ to_string(best_score));

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)
			|| sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
			playerX -= 4;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)
			|| sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
		playerX += 4;

		// player out of boundary and change the side
		if (playerX > WINDOW_WIDTH)
			playerX = 0;
		if (playerX < 0)
			playerX = WINDOW_WIDTH;

		// score and lvls
		if (playerY == height && dy < (-3))
		{
			score += 1;
			if (score > 200 && lvl == 1) {
				if (hasLoadedFile == false) {
					backgroundTexture.loadFromFile("images\\background1.png");
					playerTexture.loadFromFile("images\\doodle1.png");
					platformTexture.loadFromFile("images\\platform1.png");
					hasLoadedFile = true;
					lvl++;
					lvlText.setString("Level " + to_string(lvl));
				}
			}

			if (score >= 400 && lvl == 2) {
				if (hasLoadedFile2 == false) {
					backgroundTexture.loadFromFile("images\\background2.png");
					playerTexture.loadFromFile("images\\doodle2.png");
					platformTexture.loadFromFile("images\\platform2.png");
					hasLoadedFile = true;
					lvl++;
					lvlText.setString("Level " + to_string(lvl));
				}
			}

			if (score > best_score) {
				best_score = score;
			}

			scoreText.setString("Score: " + to_string(score) + "\nBest score: " + to_string(best_score));
		}

		// player's jump mechanism
		dy += 0.16;
		playerY += dy;

		if (playerY < height)
		{
			for (size_t i = 0; i < WINDOW_WIDTH / platformTexture.getSize().x + 1; ++i)
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
		for (size_t i = 0; i < WINDOW_WIDTH/platformTexture.getSize().x+1; ++i)
		{
			if (detectJump(playerX, playerY, platformPosition[i], platformTexture, dy)) // player is falling
			{
				sound.play();
				dy = -10;
			}
		}
		player.setPosition(playerX, playerY);

		window.draw(background);
		window.draw(player);

		//black hole
		for (size_t i = 0; i < WINDOW_WIDTH / platformTexture.getSize().x + 1; ++i)
		{
			if ((sf::FloatRect(playerX + PLAYER_LEFT_BOUNDING_BOX, playerY + 5, 6, 6).intersects(blackHole.getGlobalBounds()))) {
				gameoverText.setPosition(30, 200);
				scoreText.setPosition(150, 400);
				if (getScoreFromHistory() < best_score) {
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
				fprintf(SCORE_HISTORY_FILE, "%d\n", best_score); 
				fclose(SCORE_HISTORY_FILE);
				sf::sleep(sf::milliseconds(300));
				sound1.play();
				best_score = getScoreFromHistory();
				gameover(score, best_score);
			}
		}
		player.setPosition(playerX, playerY);
		 
		// set and draw platforms
		for (size_t i = 0; i < WINDOW_WIDTH / platformTexture.getSize().x + 1; ++i)
		{
			platform.setPosition(platformPosition[i].x, platformPosition[i].y);
			window.draw(platform);
		}
		//for (size_t i = 0; i < WINDOW_WIDTH / blackHole.getGlobalBounds().width; ++i) {
		blackHole.setPosition(platformPosition[5].x + 10, platformPosition[5].y + 10);
		//}
		window.draw(blackHole);

		// game over
		if (playerY > WINDOW_HEIGHT)
		{
			sound1.play();
			gameoverText.setPosition(30, 200);
			scoreText.setPosition(150, 400);
			if (getScoreFromHistory() < best_score) {
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
			fprintf(SCORE_HISTORY_FILE, "%d\n", best_score);
			fclose(SCORE_HISTORY_FILE);
			best_score = getScoreFromHistory();
			gameover(score, best_score);
		}
		window.draw(scoreText);
		window.draw(lvlText);
		window.display();
	}
}

int main()
{
	// bg sound
	sf::SoundBuffer buffer2;
	buffer2.loadFromFile("sound\\background.wav");
	sf::Sound sound2;
	sound2.setBuffer(buffer2);
	sound2.play();
	sound2.setLoop(true);
	startScreen();
	return 0;
}
