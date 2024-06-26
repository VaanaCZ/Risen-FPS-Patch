
# Risen FPS patch

This repo contains fixes for some of the commonly encountered FPS-related bugs in the 2009 video game Risen (Available on [GOG.com](https://www.gog.com/en/game/risen) or [Steam](https://store.steampowered.com/app/40300/Risen/?l=czech)).

Special thanks to Auronen for helping to diagnose and fix these issues.

## Installation

**Caution: This patch is intended for the original version of the game. It will not work on the updated version from 2023!**

1. Download the zip file.
2. Extract the included DLL file into the `scripts` folder in your game directory (`Risen\bin\scripts`).
3. Launch the game and enjoy.

## Techical explanation

This section includes technical explanations for how each of the individual fixes function.

### Climbing patch

Whenever the player initiates a climb by standing next to a ledge and pressing space, the game calculates the target position by gradually interpolating between the player's position and the ledge. After doing so, the game then "corrects" the player position to the target position by setting the player's velocity and letting the physics system do the rest of the work.

Sadly, on high FPS this mechanism breaks and the game overcorrects by a small amount each frame, which causes the player to jump up and down super fast, eventually sending him either into the stratosphere or to the depths of the earth.

The "fix" bypasses the physics system entirely and instead sets the target position of the player directly.

[![Risen - Climbing FPS fix](https://img.youtube.com/vi/1tOcu34vBKM/maxresdefault.jpg)](https://www.youtube.com/watch?v=1tOcu34vBKM)

### Airwalk patch

While in freefall, the game performs checks to see if the player has landed on the ground so that it can return to a walking state and give back control. This check works by comparing the player's current Y coordinate to the one from the previous frame and seeing if the difference between them is below a certain threshold (less than 5cm). 

When the game is running at high FPS (100+), the difference in position between individual frames becomes so small, that the game falsely detects the player has landed.

The patch fixes this by altering the check according to the current fps.

[![Risen - Airwalk FPS fix](https://img.youtube.com/vi/Hl06FP98I6I/maxresdefault.jpg)](https://www.youtube.com/watch?v=Hl06FP98I6I)

### Thunder patch

Every frame there is check which determines whether a new thunder effect should be generated. If this check passes (1% chance for a sound effect and 0.75% for a visual effect), then a new thunder effect is created.

The patch simply limits this check to 1/30 of an FPS. So that the thunderstorm effects are generated at the correct frequency.
