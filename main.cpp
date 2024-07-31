#include <SFML/Graphics.hpp>
//#include <SFML/Audio.hpp>
#include <string>
#include <vector>
//#include <cmath>
//#include <ctime>

//#include <iostream>

//These could be changed to variables to alter the game by the user
#define bodySize 100.0f //Pixels (default is 100.0f) (in this case "size" is meant as a lenght of a side of the player head)
#define updateSpeed 200.0f //Miliseconds (default is 200.0f)

//Window dimensions
const int windowWidth = 1200; //(default is 1200)
const int windowHeight = 700; //(default is 700)

//Score is decided by the time it took 
unsigned int score = 0;
//Total foodEaten
unsigned int foodEaten = 0;

//game Stadium variable 0 is Main menu, 1 is gameplay, 2 is game over screen and 3 is Victory 
unsigned short gameStadium = 0;
//semi-transparent overlay for gameStadium: 0, 2 and 3
sf::RectangleShape overlay;

//Player Position
double p_xPos;
double p_yPos;

//Player Velocity
float p_xVel = 0;
float p_yVel = 0;

//Player snake Head Direction
unsigned short tempDirection = 360;
unsigned short direction = 360;

//Positions (snake body)
std::vector<sf::Vector2f> positions;

bool gameOver = false;

bool didEat = true;

//hasStartedMoving boolean for the foodTimer to start counting after the snake starts moving
bool hasStartedMoving = false;

//For bypassing tempDirection and direction checking for the first move when a new game starts
bool bypassDirectionCheck = true;

//Button text had to be a global variable for the restart function
sf::Text button_text;

class Body
{
private:

public:
	sf::RectangleShape rect;

	Body()
	{
		rect.setFillColor(sf::Color(0, 150, 0, 255)); //Default is sf::Color(0, 150, 0, 255)
		rect.setSize(sf::Vector2f(bodySize, bodySize));
		rect.setPosition(positions[positions.size() - 1]);

	}
};

//snake head
sf::RectangleShape head;

//std::vector for bodies
std::vector<Body> bodies;

//Food that will change positions everytime it gets eaten
sf::RectangleShape food;

//std::vector of all possible positions that the food could spawn on (without head and all body positions)
std::vector<sf::Vector2f> possibleFoodPositions;

//if called when no space is left to respawn the food on, then the program will probably crash
void respawnFood()
{
	//new technique, that puts all available possibleFoodPositions on a std::vector and then chooses some position from possibleFoodPositions with a random number
	//Adding all possibleFoodPositions to the std::vector
	for (int y = 0; y < (windowHeight / bodySize); y++)
	{
		for (int x = 0; x < (windowWidth / bodySize); x++)
		{
			possibleFoodPositions.push_back(sf::Vector2f(x * bodySize, y * bodySize));
		}
	}

	//Erasing the snake head from the std::vector
	for (int i = 0; i < possibleFoodPositions.size(); i++)
	{
		if (possibleFoodPositions[i] == sf::Vector2f(head.getPosition()))
		{
			possibleFoodPositions.erase(possibleFoodPositions.begin() + i);
			break;
		}
	}
	
	//Erasing all (body) positions from possibleFoodPositions
	for (int i = 0; i < positions.size(); i++)
	{
		for (int j = 0; j < possibleFoodPositions.size(); j++)
		{
			if (positions[i] == possibleFoodPositions[j])
			{
				possibleFoodPositions.erase(possibleFoodPositions.begin() + j);
			}
		}
	}
	
	//set food position to the random sf::Vector2f chosen from the possibleFoodPositions std::vector
	food.setPosition(possibleFoodPositions[rand() % possibleFoodPositions.size()]);

	//Erase old possibleFoodPositions that will not be ever used again
	possibleFoodPositions.clear();

	/*
	//Old technique that relies on luck, it tries to find a random x and y that the snake head nor any of the bodies are not on... if the random coordinates are in some of the positions of the head or the bodies, the function calls itself with recursion to repeat and find a new coordinate
	int x = (rand() % int(windowWidth / bodySize)) * bodySize;
	int y = (rand() % int(windowHeight / bodySize)) * bodySize;


	for (int i = 0; i < bodies.size(); i++)
	{
		if ((x == bodies[i].rect.getPosition().x && y == bodies[i].rect.getPosition().y) || (x == head.getPosition().x && y == head.getPosition().y))
		{
			respawnFood();
			break;
		}
		else
		{
			food.setPosition(x, y);
		}
	}
	*/
}

void spawnFood()
{
	int x = (rand() % int(windowWidth / bodySize) * bodySize);
	int y = (rand() % int(windowHeight / bodySize) * bodySize);


	if (x == head.getPosition().x && y == head.getPosition().y)
	{
		spawnFood();
	}
	else
	{
		food.setPosition(x, y);
	}
}

void restartGame()
{
	gameStadium = 0;

	//Player snake Head Direction
	tempDirection = 360;
	direction = 360;

	//Player snake Head Position
	p_xPos = int(windowWidth / bodySize * 0.5f) * bodySize;
	p_yPos = int(windowHeight / bodySize * 0.5f) * bodySize;

	//Set Main menu Button text to say "Play" again
	button_text.setString("Play");

	//Erase all bodies
	bodies.clear();
	positions.clear();

	//Reset Scores
	score = 0;
	foodEaten = 0;

	gameOver = false;

	didEat = true;

	hasStartedMoving = false;

	bypassDirectionCheck = true;


	//snake Head Velocity
	p_xVel = 0;
	p_yVel = 0;


	//Push back 1 body along with the snake Head when the game starts
	positions.push_back(sf::Vector2f(p_xPos, p_yPos));


	//snake Head Sprite
	head.setSize(sf::Vector2f(bodySize, bodySize));
	head.setPosition(p_xPos, p_yPos);
	head.setFillColor(sf::Color(50, 200, 0, 255)); //Default is sf::Color(50, 200, 0, 255)

	//Food Sprite
	food.setSize(sf::Vector2f(bodySize, bodySize));
	food.setFillColor(sf::Color(255, 0, 0, 255)); //Default is sf::Color(255, 0, 0, 255)

	//Spawn the first food
	spawnFood();

}

int main()
{
	//RenderWindow
	sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Hadicek game", sf::Style::Close);
	window.setFramerateLimit(60);

	//Delta Time
	sf::Clock snakeTimer;

	//Food timer to determine the score
	sf::Clock foodTimer;

	srand(time(0));

	//Event
	sf::Event evnt;

	//Font loaded from C:\Windows\Fonts\arialbd.ttf (Will not work on other operating systems, unless you create this directory and put a font in a ttf format and with the same name.)
	sf::Font Arial;
	Arial.loadFromFile("C:\\Windows\\Fonts\\arialbd.ttf");
	
	//Play Button
	sf::RectangleShape button_rect;
	button_rect.setFillColor(sf::Color(60, 60, 60));
	button_rect.setOutlineColor(sf::Color(30, 30, 30));
	button_rect.setOutlineThickness(6);
	button_rect.setSize(sf::Vector2f(windowWidth * 0.15f, windowHeight * 0.15f));
	button_rect.setPosition(sf::Vector2f((windowWidth * 0.5f) - (button_rect.getLocalBounds().width * 0.5f), windowHeight * 0.8f));

	//Text for the button
	button_text.setFont(Arial);
	button_text.setString("Play");
	button_text.setCharacterSize(45);
	button_text.setFillColor(sf::Color(255, 255, 255));
	button_text.setPosition(sf::Vector2f(button_rect.getPosition().x + (button_rect.getLocalBounds().width - button_text.getLocalBounds().width - (button_rect.getOutlineThickness() - button_text.getOutlineThickness()) * 2) * 0.5f, windowHeight * 0.8f + button_rect.getLocalBounds().height * 0.16f));
	button_text.setOutlineThickness(5);

	//Texts
	sf::Text mainMenu_text;
	mainMenu_text.setFont(Arial);
	mainMenu_text.setCharacterSize(windowWidth * 0.05f);
	mainMenu_text.setFillColor(sf::Color(255, 255, 255, 255));
	mainMenu_text.setOutlineThickness(5);
	mainMenu_text.setOutlineColor(sf::Color(0, 0, 0, 255));
	mainMenu_text.setString("Welcome to this game!");
	mainMenu_text.setPosition(sf::Vector2f((windowWidth * 0.5f) - (mainMenu_text.getLocalBounds().width * 0.5f), windowHeight * 0.2f));

	sf::Text gameOver_text;
	gameOver_text.setFont(Arial);
	gameOver_text.setCharacterSize(100);
	gameOver_text.setFillColor(sf::Color(255, 255, 255, 255));
	gameOver_text.setOutlineThickness(5);
	gameOver_text.setOutlineColor(sf::Color(0, 0, 0, 255));
	gameOver_text.setString("Game over!");
	gameOver_text.setPosition(sf::Vector2f((windowWidth * 0.5f) - (gameOver_text.getLocalBounds().width * 0.5f), windowHeight * 0.2f));

	sf::Text foodEaten_text;
	foodEaten_text.setPosition(10, 5);
	foodEaten_text.setFont(Arial);
	foodEaten_text.setCharacterSize(25);
	foodEaten_text.setFillColor(sf::Color(255, 255, 255, 255));
	foodEaten_text.setOutlineThickness(5);
	foodEaten_text.setOutlineColor(sf::Color(0, 0, 0, 255));
	foodEaten_text.setString("Food Eaten: " + foodEaten);

	sf::Text score_text;
	score_text.setPosition(10, 35);
	score_text.setFont(Arial);
	score_text.setCharacterSize(25);
	score_text.setFillColor(sf::Color(255, 255, 255, 255));
	score_text.setOutlineThickness(5);
	score_text.setOutlineColor(sf::Color(0, 0, 0, 255));
	score_text.setString("Score: " + std::to_string(score));

	overlay.setPosition(0, 0);
	overlay.setSize(sf::Vector2f(windowWidth, windowHeight));
	overlay.setFillColor(sf::Color(0, 0, 0, 128));


	//Player snake Head Position
	p_xPos = int(windowWidth / bodySize * 0.5f) * bodySize;
	p_yPos = int(windowHeight / bodySize * 0.5f) * bodySize;

	//Push back 1 body along with the snake Head when the game starts
	positions.push_back(sf::Vector2f(p_xPos, p_yPos));


	//snake Head Sprite
	head.setSize(sf::Vector2f(bodySize, bodySize));
	head.setPosition(p_xPos, p_yPos);
	head.setFillColor(sf::Color(50, 200, 0, 255)); //Default is sf::Color(50, 200, 0, 255)

	//snake Head Velocity
	p_xVel = 0;
	p_yVel = 0;


	food.setSize(sf::Vector2f(bodySize, bodySize));
	food.setFillColor(sf::Color(255, 0, 0, 255));

	//Spawn the first food
	spawnFood();

	bool canClickButtonAgain = true;

	//Gameloop
	while (window.isOpen())
	{
		//Poll Events
		while (window.pollEvent(evnt))
		{
			if (evnt.type == evnt.Closed)
			{
				window.close();
			}
		}


		//Events
		if (gameStadium == 1)
		{
			gameOver_text.setString("Game over!");
			button_text.setString("Restart");

			//Controls
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
			{
				tempDirection = 90;
				p_xVel = 0.0f;
				p_yVel = -1 * head.getSize().y;

				hasStartedMoving = true;
			}

			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
			{
				tempDirection = 270;
				p_xVel = 0.0f;
				p_yVel = head.getSize().y;
				
				hasStartedMoving = true;
			}

			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
			{
				tempDirection = 0;
				p_xVel = -1 * head.getSize().x;
				p_yVel = 0.0f;

				hasStartedMoving = true;
			}

			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
			{
				tempDirection = 180;
				p_xVel = head.getSize().x;
				p_yVel = 0.0f;

				hasStartedMoving = true;
			}


			if (snakeTimer.getElapsedTime().asMilliseconds() >= updateSpeed)
			{
				if (!bypassDirectionCheck)
				{
					if (direction == 90 && tempDirection == 270)
					{
						tempDirection = 90;
						p_yVel = -1 * head.getSize().y;
					}

					if (direction == 270 && tempDirection == 90)
					{
						tempDirection = 270;
						p_yVel = head.getSize().y;
					}

					if (direction == 0 && tempDirection == 180)
					{
						tempDirection = 0;
						p_xVel = -1 * head.getSize().x;
					}

					if (direction == 180 && tempDirection == 0)
					{
						tempDirection = 180;
						p_xVel = head.getSize().x;
					}
				}

				bypassDirectionCheck = false;


				direction = tempDirection;

				p_xPos = p_xPos + p_xVel;
				p_yPos = p_yPos + p_yVel;


				
				//GameOver checking before actually setting a real position
				for (int i = 1; i < bodies.size(); i++)
				{
					//If the Player Head bumps to any of the bodies 
					if (p_xPos == (positions[i].x) && p_yPos == (positions[i].y))
					{
						gameOver = true;
						gameStadium = 2;
					}
				}

				//If bumps to any of the sides of the window
				if ((p_xPos < 0 || p_xPos >= windowWidth) || (p_yPos < 0 || p_yPos >= windowHeight))
				{
					gameOver = true;
					gameStadium = 2;
				}
				


				if (!gameOver)
				{

					if (!didEat)
					{
						positions.erase(positions.begin());
						bodies.erase(bodies.begin());
						
					}

					Body body;

					bodies.push_back(body);

					didEat = false;


					head.setPosition(p_xPos, p_yPos);
	
					positions.push_back(sf::Vector2f(p_xPos, p_yPos));



					if (head.getPosition() == food.getPosition())
					{
						didEat = true;
						foodEaten = foodEaten + 1;

						//Set current score
						score = score + (10 / foodTimer.getElapsedTime().asSeconds());

						//Checking if the entire board is filled, if yes, then set to victory gameStadium (I used std::ceil to round UP the number and I subtracted 2, because the snake player head is not in the "bodies" std::vector and the second position on the board is taken by the food)
						if ((std::ceil(windowWidth / bodySize) * std::ceil(windowHeight / bodySize) - 2) == bodies.size())
						{
							gameStadium = 3;
							
							gameOver_text.setString("Victory!");
						}
						//respawnFood() if the gameStadium is not 3 to avoid crashing
						if (gameStadium == 1)
						{
							respawnFood();
						}


						foodTimer.restart();
					}

					if (!hasStartedMoving)
					{
						foodTimer.restart();
					}

					snakeTimer.restart();
				}
			}
		}


		score_text.setString("Score: " + std::to_string(score));
		foodEaten_text.setString("Food Eaten: " + std::to_string(foodEaten));


		if (!sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			canClickButtonAgain = true;
		}

		if (gameStadium != 1)
		{

			if (((sf::Mouse::getPosition().x >= (window.getPosition().x + button_rect.getPosition().x)) && (sf::Mouse::getPosition().x <= (window.getPosition().x + button_rect.getPosition().x + button_rect.getLocalBounds().width))) && ((sf::Mouse::getPosition().y >= (window.getPosition().y + button_rect.getPosition().y + 25)) && ((sf::Mouse::getPosition().y) <= (window.getPosition().y + button_rect.getPosition().y + 25 + button_rect.getLocalBounds().height))))
			{
				button_rect.setFillColor(sf::Color(80, 80, 80));

				if (canClickButtonAgain)
				{
					if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
					{
						canClickButtonAgain = false;

						if (gameStadium == 0)
						{
							gameStadium = 1;
						}
						else
						{
							restartGame();
						}
					}
				}

			}
			else
			{
				button_rect.setFillColor(sf::Color(60, 60, 60));
			}
		}

		//std::cout for debugging
		//std::cout << "p_xVel: " << p_xVel << "	" << "p_yVel: " << p_yVel << "		" << "direction: " << direction << "	" << "tempDirection: " << tempDirection << "\n";
		//std::cout << "bodies std::vector size: " << bodies.size() << "	" << "positions std::vector size: " << positions.size() << "	" << "snakeTimer: " << snakeTimer.getElapsedTime().asMilliseconds() << "	" << "foodTimer: " << foodTimer.getElapsedTime().asSeconds() << "	" << "gameStadium: " << gameStadium << "\n";

		//Renderer
		window.clear(sf::Color(70, 70, 70, 255));

		if (gameStadium == 3)
		{
			window.clear(sf::Color(0, 150, 0, 255));
		}

		//If gameStadium is not in the main menu, then draw the head, bodies and food
		if (gameStadium != 0)
		{
			window.draw(food);

			for (int i = 0; i < bodies.size(); i++)
			{
				window.draw(bodies[i].rect);
			}


			window.draw(head);
		}

		//main Menu/game over/victory semi-transparent overlay
		if (!(gameStadium == 1))
		{
			window.draw(overlay);

			button_text.setPosition(sf::Vector2f(button_rect.getPosition().x + (button_rect.getLocalBounds().width - button_text.getLocalBounds().width - (button_rect.getOutlineThickness() - button_text.getOutlineThickness()) * 2) * 0.5f, windowHeight * 0.8f + button_rect.getLocalBounds().height * 0.16f));
			window.draw(button_rect);
			window.draw(button_text);

			if (gameStadium != 0)
			{
				gameOver_text.setPosition(sf::Vector2f((windowWidth * 0.5f) - (gameOver_text.getLocalBounds().width * 0.5f), windowHeight * 0.2f));

				window.draw(gameOver_text);
			}
		}


		//Main menu
		if (gameStadium == 0)
		{
			window.draw(mainMenu_text);
		}
		
		//Food eaten text
		if (!(gameStadium == 0))
		{
			window.draw(foodEaten_text);
			window.draw(score_text);
		}


		window.display();

		//Just for reseting sf::Clock
		if (gameStadium != 1)
		{
			foodTimer.restart();
			snakeTimer.restart();
		}

	}


	return 0;
}
