City Collectathon

A 3D city exploration game built in C++ and OpenGL (FreeGLUT) for a Computer Graphics group project. Drive a hovercraft, collect golden crystals, and observe a dynamic day/night cycle while demonstrating core 3D mathematical transformations.

🎓 Graphics Topics Showcased

Translation (glTranslatef): Positioning objects and player movement.

Rotation (glRotatef): Player steering, camera orbit, and spinning collectibles.

Scaling (glScalef): Object sizing and pulsing animations.

Shearing (Custom Matrix): A custom 4x4 matrix skewing the central Transformation Monument.

🎮 Controls

W / A / S / D: Drive Hovercraft

Left Click + Drag: Orbit Camera

🚀 Build and Run

Ensure you have a C++ compiler and the freeglut library installed (e.g., sudo apt-get install freeglut3-dev on Linux).

Compile the game from your terminal:

g++ main.cpp camera.cpp collision.cpp environment.cpp globals.cpp lighting.cpp -lglut -lGLU -lGL -o CityGame


Run the game:

./CityGame



[Add name here] - [Role/Topic]

[Add name here] - [Role/Topic]
