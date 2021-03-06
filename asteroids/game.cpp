/*********************************************************************
 * File: game.cpp
 * Description: Contains the implementaiton of the game class
 *  methods.
 *
 *********************************************************************/

#include "game.h"
#include "uiInteract.h"
#include "flyingObject.h"
#define OFF_SCREEN_BORDER_AMOUNT 5


// These are needed for the getClosestDistance function...
#include <limits>
#include <algorithm>
using namespace std;

/**********************************************************
 * Initial constructor
 ***********************************************************/
Game::Game(Point topLeft, Point bottomRight)
{
   this->topLeft = topLeft;
   this->bottomRight = bottomRight;
   createStartingRocks(5);

   // set random star points
   for (int i = 0; i < 100; i++)
   {
      pointStar[i].setX(random(-200, 200));
      pointStar[i].setY(random(-200, 200));
   }
	
}

/**********************************************************
 * Add stars to the background
 ***********************************************************/
void Game::addBackGround()
{
   for (int i = 0; i < 100; i++)
      drawDot(pointStar[i]);
}

/**********************************************************
 * Set initial rocks
 ***********************************************************/
void Game::createStartingRocks(int count)
{
   Velocity startVelocity;
   for (int i = 0; i < count; i++)
   {
      startVelocity.setX(random(topLeft.getX(), bottomRight.getX())); 
      startVelocity.setY(random(bottomRight.getY(), topLeft.getY())); 
      startVelocity.setDx(random(-1.0, 1.0));
      startVelocity.setDy(random(-1.0, 1.0));
      Rock* theRock = new BigRock(startVelocity);
      _rocks.push_back(theRock);
   }

}

/**********************************************************
 * Add inital Rocks to the game
 ***********************************************************/
void Game::addRock(Rock * rock)
{
   Velocity startVelocity;
   startVelocity.setX(random(topLeft.getX(), bottomRight.getX())); 
   startVelocity.setY(random(bottomRight.getY(), topLeft.getY())); 
   startVelocity.setDx(random(-1.0, 1.0));
   startVelocity.setDy(random(-1.0, 1.0));
   Rock* theRock = new SmallRock(startVelocity);
   _rocks.push_back(theRock);
}

/**********************************************************
 * Advance through the frames
 ***********************************************************/
void Game::advance()
{
   advanceRock();
   advanceBullets();
   handleCollisions();
   ship.advance(); 
   cleanUpZombies();
   addBackGround();
}

/**********************************************************
 * Move the rocks
 ***********************************************************/
void Game::advanceRock()
{
   for (int i = 0; i < _rocks.size(); i++)
      _rocks[i]->advance();
}

/**********************************************************
 * Move the Bullets
 ***********************************************************/
void Game::advanceBullets()
{
   for (int i = 0; i < bullets.size(); i++)
   {
      if (bullets[i].isAlive())
         bullets[i].advance();
   }
}

/**********************************************************
 * Handle User Input
 ***********************************************************/
void Game::handleInput(const Interface & ui)
{
   if (ui.isUp()) 	  { ship.applyThrust(); }

   if (ui.isLeft())  { ship.rotateLeft();  }

   if (ui.isRight()) { ship.rotateRight();	}

   if (ui.isSpace())
   { 
      bool value = ship._alive;
      if (ship._alive == 0)
         value = false;
      Bullet newBullet;
      newBullet.fire(ship.getPoint(), ship.getAngle(), value);
      bullets.push_back(newBullet);
   }
}

/**********************************************************
 * Draw objects
 ***********************************************************/
void Game::draw(const Interface & ui)
{
   ship.draw();
   for (int i = 0; i < _rocks.size(); i++)
   {
      _rocks[i]->draw();
   }
	
   if (!ui.isUp())
   {
      bool value = false;
      ship.thrust(value);
   }
   if (ui.isUp())
   {
      bool value = true;
      ship.thrust(value);
   }
   for (int i = 0; i < bullets.size(); i++)
   {
      if (bullets[i].isAlive())
      {
         bullets[i].draw();
      }
   }
   if (ship._alive == false)
   {
      drawText((0,0), "GAME OVER");
   }
   Point pointLocation;
   pointLocation.setX(topLeft.getX() + 15);
   pointLocation.setY(topLeft.getY() - 5);
   drawNumber(pointLocation, score);

}

/**********************************************************
 * Make objects collide and give score
 ***********************************************************/
void Game::handleCollisions()
{
   for (int i = 0; i < bullets.size(); i++)
      for (int j = 0; j < _rocks.size(); j++)
      {
         if (bullets[i].isAlive() && _rocks[j] != NULL && _rocks[j]->isAlive())
         {

            if (fabs(bullets[i].getPoint().getX() - _rocks[j]->getPoint().getX()) < CLOSE_ENOUGH
                && fabs(bullets[i].getPoint().getY() - _rocks[j]->getPoint().getY()) < CLOSE_ENOUGH)
            {
               (_rocks[j])->kill();
               if (_rocks[j]->getRadius() == BIG_ROCK_SIZE)
               {
                  splitBigRock((BigRock*)_rocks[j]);
                  points = 5;
               }
               else if (_rocks[j]->getRadius() == MEDIUM_ROCK_SIZE)
               {
                  splitMediumRock((MediumRock*)_rocks[j]);
                  points = 10;
               }
  				
               score += points;
					
               bullets[i].kill();
            }
         }
      }
   for (int k = 0; k < _rocks.size(); k++)
		
   {
      if (ship.isAlive() && _rocks[k] != NULL && _rocks[k]->isAlive())
      {

         if (fabs(ship.getPoint().getX() - _rocks[k]->getPoint().getX()) < CLOSE_ENOUGH
             && fabs(ship.getPoint().getY() - _rocks[k]->getPoint().getY()) < CLOSE_ENOUGH)
         {
            (_rocks[k])->kill();
            ship.kill();
            if (_rocks[k]->getRadius() == BIG_ROCK_SIZE && !_rocks[k]->isAlive())
            {
               splitBigRock((BigRock*)_rocks[k]);
					
            }
            else if (_rocks[k]->getRadius() == MEDIUM_ROCK_SIZE)
            {
               splitMediumRock((MediumRock*)_rocks[k]);
            }
         }
      }
   }
}

/**********************************************************
 * Split Rocks
 ***********************************************************/
void Game::splitBigRock(BigRock * rock)
{
   // First Medium Asteroid // plus 1 pixel/frame in the up direction.
   Velocity startVelocityMedium1;
   startVelocityMedium1.setX(rock->getPoint().getX() + 1);
   startVelocityMedium1.setY(rock->getPoint().getY() + 1);
   startVelocityMedium1.setDx(random(-1.0, 1.0));
   startVelocityMedium1.setDy(random(0.0, 1.0));
   Rock* theRock = new MediumRock(startVelocityMedium1);
   this->_rocks.push_back(theRock);

   // Second Medium Asteroid // plus 1 pixel/frame in the down direction.
   Velocity startVelocityMedium2;
   startVelocityMedium2.setX(rock->getPoint().getX() + 1);
   startVelocityMedium2.setY(rock->getPoint().getY() + 1);
   startVelocityMedium2.setDx(random(-1.0, 1.0));
   startVelocityMedium2.setDy(random(-1.0, 0.0));
   //startVelocity.setDy(random(-1.0, 1.0));
   Rock* theRock2 = new MediumRock(startVelocityMedium2);
   this->_rocks.push_back(theRock2);

   //Small Asteroid plus 2 pixels/frame to the right.
   Velocity startVelocitySmall;
   startVelocitySmall.setX(rock->getPoint().getX() + 2);
   startVelocitySmall.setY(rock->getPoint().getY() + 2);
   startVelocitySmall.setDx(random(0.0, 1.0));
   startVelocitySmall.setDy(random(-1.0, 1.0));
   Rock* theRock3 = new SmallRock(startVelocitySmall);
   _rocks.push_back(theRock3);
}

/**********************************************************
 * Split Medium Rocks
 ***********************************************************/
void Game::splitMediumRock(MediumRock * rock)
{
   // First asteroid going to the right 
   Velocity startVelocity1;
   startVelocity1.setX(rock->getPoint().getX());
   startVelocity1.setY(rock->getPoint().getY());
   startVelocity1.setDx(random(0.0, 1.0));
   startVelocity1.setDy(random(-1.0, 1.0));
   Rock* theRock = new SmallRock(startVelocity1);
   _rocks.push_back(theRock);

   // Second asteroid going to the left
   Velocity startVelocity2;
   startVelocity2.setX(rock->getPoint().getX() + 3);
   startVelocity2.setY(rock->getPoint().getY() + 3);
   startVelocity2.setDx(random(-1.0, 0.0));
   startVelocity2.setDy(random(-1.0, 1.0));
   Rock* theRock2 = new SmallRock(startVelocity2);
   _rocks.push_back(theRock2);
	
}

/**********************************************************
 * Erase dead objects
 ***********************************************************/
void Game :: cleanUpZombies()
{
   for (int j = 0; j < _rocks.size(); j++)
   {
      if (_rocks[j] != NULL && !_rocks[j]->isAlive())
      {
         _rocks.erase(_rocks.begin() + j);
      }
   }
   vector<Bullet>::iterator bulletIt = bullets.begin();
   while (bulletIt != bullets.end())
   {
      Bullet bullet = *bulletIt;
      if (!bullet.isAlive())
      {    
         bulletIt = bullets.erase(bulletIt);
      }
      else
      {
         bulletIt++;
      }
   } 
}

/**********************************************************
 * Get screen limits
 ***********************************************************/
bool Game::isOnScreen(const Point & point)
{
   return (point.getX() >= topLeft.getX() - OFF_SCREEN_BORDER_AMOUNT
           && point.getX() <= bottomRight.getX() + OFF_SCREEN_BORDER_AMOUNT
           && point.getY() >= bottomRight.getY() - OFF_SCREEN_BORDER_AMOUNT
           && point.getY() <= topLeft.getY() + OFF_SCREEN_BORDER_AMOUNT);
}


/**********************************************************
 * Function: getClosestDistance
 * Description: Determine how close these two objects will
 *   get in between the frames.
 **********************************************************/
/*
  float Game :: getClosestDistance(const FlyingObject &obj1, const FlyingObject &obj2) const
  {
// find the maximum distance traveled
float dMax = max(abs(obj1.getVelocity().getDx()), abs(obj1.getVelocity().getDy()));
dMax = max(dMax, abs(obj2.getVelocity().getDx()));
dMax = max(dMax, abs(obj2.getVelocity().getDy()));
dMax = max(dMax, 0.1f); // when dx and dy are 0.0. Go through the loop once.

float distMin = std::numeric_limits<float>::max();
for (float i = 0.0; i <= dMax; i++)
{
Point point1(obj1.getPoint().getX() + (obj1.getVelocity().getDx() * i / dMax),
obj1.getPoint().getY() + (obj1.getVelocity().getDy() * i / dMax));
Point point2(obj2.getPoint().getX() + (obj2.getVelocity().getDx() * i / dMax),
obj2.getPoint().getY() + (obj2.getVelocity().getDy() * i / dMax));

float xDiff = point1.getX() - point2.getX();
float yDiff = point1.getY() - point2.getY();

float distSquared = (xDiff * xDiff) +(yDiff * yDiff);

distMin = min(distMin, distSquared);
}

return sqrt(distMin);
}
*/
