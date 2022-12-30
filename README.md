# Risen FPS patch

This repo contains fixes for some of the commonly encountered FPS-related bugs in the 2009 video game Risen (Available on [GOG.com](https://www.gog.com/en/game/risen) or [Steam](https://store.steampowered.com/app/40300/Risen/?l=czech)).

## Techical explanation

This section includes technical explanations for how each of the individual fixes function.

### Climbing patch

Whenever the player initiates a climb by standing next to a ledge and pressing space, the player controller switches to a "climbing" state. While the player is in this state, the game calculates the target position by grudually interpolating between the player's position and the ledge.

Once this is done, the game then "corrects" the player position to the target position by setting the player's velocity and letting the physics system do the rest of the work.

Sadly, on high FPS this mechanism breaks and the game overcorrects by a small amount each frame, which causes the player to jump up and down super fast, eventually sending him either into the stratosphere or the depths of the earth.

The "fix" bypasses the physics system entirely and instead sets the target position of the player directly.

### Airwalk patch

