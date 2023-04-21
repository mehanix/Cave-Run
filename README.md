<div align=center> <img src="./assets/logo.png" style="width:75%"></div>

<br/><br/>
<br/>

<div><img src="./assets/intro.png"></div>
  <br/>

This game was created as part of the Introduction to Robotics course I took during my 3rd year of studying Computer Science @ University of Bucharest, Faculty of Mathematics and Computer Science. It is written in C++ and the Arduino libraries, and was created during the span of a month.
  
I chose this game because I believed that it would best fit the restrictions of the hardware (8x8 single-color led matrices don't allow lots of expression 😛). I also wanted to implement a game with a panning camera, I thought it would be an interesting technical challenge. 

  I'm not aware of any other game that plays similarly - I'd call it a bit similar to Minesweeper, but that's about it.
  
<div><img src="./assets/game-description.png"></div>
  <br/>
  
🔥**Cave Run**🔥 is a game where you must _escape as many rooms as you can_ in 60 seconds ⌚.

To exit a room, you must collect all the keys 🔑 while avoiding the bombs 💣. 

The bombs are buried underground, but thankfully you are equipped with your trusty radar 📡 that warns you if you're approaching a bomb. 

_**Don't get blown up! 💥**_

<img src="./assets/features.png">

😎 Complete menu

🗺 Procedural generation

🏋 Multiple difficulty settings

🔲 Multple room sizes

🏆 Scoreboard

🛠 Customizable settings

🎹 Theme song and sound effects!!!! as well as a muting option 

⬆ Power-up: on click, see bombs in room

🎉 Endless fun!


<div ><img src="./assets/how-to-play.png"></div>
  <br/>
    <br/>
  <div align=center><img src="./assets/instructions.png" style="width:75%"></div>
  <br/>

<div><img src="./assets/components.png"></div>
  <br/>
  
 💥 1 Arduino Uno
 
 💥 1 8x8 LED Matrix
 
 💥 1 16x2 LCD 
 
 💥 1 joystick
 
 💥 1 passive buzzer
 
 💥 1 10k Ohm potentiometer
 
 💥 1 matrix driver
 
 💥 1 red LED
 
 💥 lots and lots of wires
 

  <br/>
  <br/>

<div><img src="./assets/media.png"></div>


<div align=center>
  <img src="./assets/build.png" style="width:60%">
<a href="https://www.youtube.com/watch?v=kI80q4XZILo"><img src="https://img.youtube.com/vi/kI80q4XZILo/0.jpg" alt="Cave Run Video"></a>

</div>

<br />

![Cave Run Docs](https://github.com/mehanix/Cave-Run/blob/main/assets/docs-header.png)

Below is the Cave Run technical overview!

## Architecture

I've said it before, I'll say it again: Cave Run, at its core, is nothing more than a huge state machine.

Here are the main states of the system:

* SPLASH: The greeting screen
* MENU: The main menu, with its options:
* MENU_SETTINGS: The settings submenu
* MENU_HIGHSCORES: Highscores submenu
* MENU_ABOUT: About screen
* GAME: The game itself
* GAME_END: The game end screen, which waits for user interaction before switching back to MENU, thus completing the game interaction circle.



All the main arduino loop() function does is check the current global state and run its corresponding loop.

## Loop design
All the main state loops do the following:

* Get user input
* If input, parse it and do action
* If action results in having to redraw things, redraw what is needed depending on the state (lcd, matrix, etc)

As an example, the `menuLoop()`:

* Reads joystick Y axis input
* If there is input, compute action logic
  * Determine if the action is legal; in this case, if moving the joystick up/down through the menu items, update the selected item index, if not going out of bounds
  * If the internal state changed, signal that we need to redraw data, by toggling `shouldRedrawMenu` to true
* If the state did something that toggled `shouldRedrawMenu`, we update the components so that the user gets to see the effects of their action.

All the submenus function similarly.

As for the main `gameLoop()`, while obviously more complicated internally, it boils down to:

* Holding an internal game state:
  * when first entering the game screen, the GAME_SETUP state is put into place, which sets up the game by resetting values and drawing LCD labels
  * afterwards, we switch to the GAME_LOOP state which loops through checking if there is any update through the various game elements:
  * `updateTime()`: Computes remaining time and displays it on the LCD when necessary
  * `updateScore()`: Redraws score if shouldRedrawScore was toggled by another event
  * `updatePlayer()`: Handles player movement by keeping track of the player's current and previous move. Computes when the player goes out of the current region bound and toggles map redrawing when necessary. Keeps track of player blinking animation.
  * `updateBombRadar()`: Computes distance between player and bombs and finds the nearest bomb. If it is near enough, light up the radar. If player has stepped on bomb, make the radar LED flash intermittently for a few seconds.
  * `updateKeys()`: Checks if player has stepped on any of the keys. If they have, remove the key from the game, and toggle redrawing of the map and updating of the key counter.
  * `updateBomb()`: Checks if player has stepped on any bomb. If they have, remove the bomb from the game, and toggle bomb specific actions (lose a life, radar goes off)
  * `updateDoor()`: Computes whether to unlock the door, when no keys are left, unlock the door to the next level.
  * `updatePowerup()`: Shows bombs visible on the current map region.

Additionally, there are the following draw functions:
* `drawMap()`: if `shouldRedrawMap` is toggled, clear and redraw the current map segment, otherwise do nothing
* `drawPlayer()`: if `shouldRedrawPlayer` is toggled, redraw player and clear player from previous position, otherwise do nothing

Finally, we also `checkEndConditions()` to see if any of the end conditions have been satisfied (timeout, no lives), and if so, we end the game, by changing the overall system state to `GAME_END`.

The score is computed on-demand , that is, whenever there is an action taken that triggers a scoreable event, we trigger score calculation and update the display accordingly.

## State changing

State changing is handled with interrupts in the `StateChanger.h` module. It's a big switch statement (you will notice that I love using switch statements - i feel they work well in this environment) which depending on the current global state (system state + other variables depending on the state) switches the global state and marks redraws to be done where necessary.

! Please note: i tried to keep the interrupt function as simple logically as possible, as per Arduino design guidelines, you're only supposed to do fast operations in interrupts, so I tried to only change variable states and other operations that are done in linear time.

### Game design
The rooms are procedurally generated using a customizable algorithm which renders a number of rooms of certain sizes based on the current difficulty.
The default configuration is as follows:

* for Easy, 3 small rooms, 3 medium rooms, then large rooms until the time runs out
* for Medium, 1 small room, 2 medium rooms, then large rooms until the time runs out
* for Hard, 2 medium rooms, then large rooms until time runs out

The difficulty setting also increases the amount of items that you have to collect/avoid per room.

## Collision detection

When generating objects for the level, I make sure they don't generate on top of one another or block the door through a brute-force algorithm. I simply check if they overlap anything that already exists, and if not, the newly-generated item is good to go. Otherwise, I generate a new pair of coordinates and try again.

You may call it rudimentary, I call it good enough for the job, due to the small map and item count. It's fast enough to be insesisable when playing, so i deem it ✨appropiate✨ for this purpose.

## The game map, player movement and panning camera

The map is sized a multiple of 4 - as the matrix is 8x8, i figured this would flow most naturally. I keep the map in memory as a simple bool 2d array, with 1 representing the walls and 0 the floor through which the player can move.

The items are kept separately in arrays of `Coordinates` - one array for the keys, one for the bombs. Ah, yes, the `Coordinate` is a simple struct I created because I don't like the C++ pair. I wrote this entire game using C-style code, it'd be weird to add C++ elements like that anyway.

One of the most interesting aspects to program in this project was the panning camera - or at least I thought so, until I figured out i could use a bit of arithmetics to get the job done.

If you imagine a map as a bunch of 8x8 quadrants, you can take your current position x/y coordinates, divide them by 8, and get the current region segment.

```
For example:
My coordinates are x=2,y=5 so i am in region {0,0}
My coordinates are x=10,y=5 so i am in region {1,0} (divide x and y by 8)
```
When changing region (when stepping from x=7 to x=8 for instance), I should redraw both the map and player. By doing the opposite operation (multiply region segment data by 8) you get the coordinates of the upper leftmost corner of the region, and you can use that index to draw the map section where the player is located.

I also draw the player as (x%8, y%8) -> once you step out of the current region you wrap around on the other side. This works out nicely as the map has no extra walls on the inside.

Cool trick.

As I'm writing this I just figured out a possible optimisation: I think you can get away without the 2d map entirely, as I only use it for drawing sections of it on the LED matrix and for collision detection. 

It would complicate the code significantly, but it'd also free up a huge chunk of the memory which... I'm not sure what I'd do with. This approach would also sacrifice extensibility: With a 2d map, I can add obstacles such as extra walls in the future, whereas using formulas would force me to keep the map as a square. 

Overall, I think i made the right call going for the 2d bitmap route.

## Conclusions

* I enjoyed making this game - the most challenging aspect was actually being flexible and adapting to the restrictions of the hardware.
* I believe that _polish_ is subjective - this is my vision of how the final version of the game should look. I found it enjoyable to see other people's versions through these projects.
* Don't write your technical docs in the github wiki panel - i just spent an hour applying forencsics techniques to my Firefox cache in order to recover this page. I succeeded, but still. 

