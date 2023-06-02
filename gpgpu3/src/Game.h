#pragma once
#include "Template/Application.h"

#define N_PARTICLES					1024 * 50		// Number of particles in simulation.
#define GRID_RESOLUTION				128				// Divide the particle area in 128 * 128 cells.
#define CELL_CAPACITY				1024			// Maximum number of particles that we can store per cell.


struct Particle
{

	glm::vec2 pos;
	glm::vec2 velocity;

	float mass;
	float radius;

	uint color;
};
>>>>>>> master


/*
* Implement your game logic in this class. The order of function calls is:
* 1) Tick
* 2) Draw
* 3) RenderGUI
*/
class Game
{

private:

	/*
	* OpenCL context used for any gpgpu operations.
	*/
	clContext* m_clContext = nullptr;
	/*
	* Contains opencl code from which we can create our kernels.
	*/
	clProgram* m_clProgram = nullptr;
	/*
	* Queue for executing kernels.
	*/
	clCommandQueue* m_clQueue = nullptr;

	/*
	* Kernel for updating positions.
	*/
	clKernel* m_clPosKernel = nullptr;

	clBuffer* m_clPositionBuffer = nullptr;
	clBuffer* m_clVelocityBuffer = nullptr;


	/*
	* Average time it takes to process a frame.
	* For debugging purpose only.
	*/
	float m_AvgFrameTime = 0.0f;

	/*
	* Accelleration structure for particle intersection.
	*/
	uint* m_Grid = new uint[GRID_RESOLUTION * GRID_RESOLUTION * CELL_CAPACITY];

	/* Convert Array of Structure to Structure of Arrays. */
	glm::vec2 m_Positions[N_PARTICLES];
	glm::vec2 m_Velocities[N_PARTICLES];
	float m_Radii[N_PARTICLES];
	float m_Masses[N_PARTICLES];
	uint m_Colors[N_PARTICLES];

	/*
	* Fill the particle grid.
	*/
	void UpdateParticleGrid();
	/*
	* Checks for particle collisions and updates particles accordingly.
	*/
	void UpdateParticleCollisions(float dt);

	/*
	* Apply forces to the particles based on user input.
	*/
	void HandleUserInput(float dt);

	/*
	* Checks if two particles collide.
	*/
	bool CheckCollision(uint p1, uint p2, float dt);
	/*
	* Resolve collision between two particles.
	*/
	void ResolveCollision(uint p1, uint p2);

	/*
	* Draws a particle on the screen.
	*/
	void DrawParticle(uint p);

public:
	/*
	* Call your initializers from the constructor.
	*/
	Game();
	/*
	* Perform any saving operations and free allocated memory.
	*/
	~Game();

	/*
	* Use the Tick function to implement your game logic.
	* @param[in] dt				Time since previous Tick call in seconds.
	*/
	void Tick(float dt);
	/*
	* Use the draw function to implement any non-gui related rendering.
	* @param[in] dt				Time since previous call in seconds.
	*/
	void Draw(float dt);
	/*
	* Use the render gui function to implement any gui related rendering using ImGui.
	* @param[in] dt				Time since previous call in seconds.
	*/
	void RenderGUI(float dt);
};

