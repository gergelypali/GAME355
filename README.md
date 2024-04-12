# Introduction
This is my Capstone project for Humber College’s Game programming advanced diploma program. In this final class our goal was to create a comprehensive piece of software. I chose to create a custom game engine from scratch without any previous code, just a blank VisualStudioCode project and the internet. My preferred language is C++ as it is the industry standard for game development and I wanted to increase my expertise in it.
# Rationale
My goal with the demo level is to showcase what the engine is currently capable of. It is a purely technical demo, without any real game experience and artistic depth. The main feature is to draw around 1600-1700 shapes on the screen with minimal GPU interaction.
# Game Loop
Starting in the Main Menu the player can start the one level or exit the game. The demo level is a maze that is randomly generated during scene loading and the player’s goal is to reach the opposite corner with the least amount of help. Because the maze is perfect, the player needs to find the one and only path to the target node. During the level, the player can ask for help and the engine can show the path for a short amount of time. After reaching the goal, the game will load the Main Menu again to restart the game loop.
# Game Mechanics
The player can move around in the maze to find the right path. Collision detection does not permit the player to move through walls and go out of the level. Help from the engine will show the right path as a blue trail of arrows.
# Controls
The following keys can be used
+ W Move up
+ S Move down
+ A Move left
+ D Move right
+ Q Exit game
+ F Create new maze, reset player to start position
+ R Find and display path
