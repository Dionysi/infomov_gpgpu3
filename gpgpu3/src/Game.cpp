#include "stdfax.h"
#include "Game.h"

#include <glm/gtx/norm.hpp> // glm::length2(...)

#define RESTITUTION 0.9f
#define SPEED_MOD 100.0f
#define MAX_SPEED 256.0f

void Game::UpdateParticleGrid()
{
	// Reset counters to zero.
	for (int i = 0; i < GRID_RESOLUTION * GRID_RESOLUTION; i++) m_Grid[i * CELL_CAPACITY] = 0;

	// Compute cell width in pixels.
	uint cellWidth = Application::RenderWidth() / GRID_RESOLUTION;
	uint cellHeight = Application::RenderHeight() / GRID_RESOLUTION;

	// Insert particles in cells.
	for (uint i = 0; i < N_PARTICLES; i++)
	{
		// Compute particle cell index.
		uint gx = m_Positions[i].x / cellWidth;
		uint gy = m_Positions[i].y / cellHeight;

		// Compute cell index.
		uint cell = (gx + gy * GRID_RESOLUTION) * CELL_CAPACITY;

		// Add particle to cell if there is any space.
		if (m_Grid[cell] < CELL_CAPACITY - 1)
		{
			// Index within the cell.
			uint cellidx = m_Grid[cell];
			// Add to cell.
			m_Grid[cell + cellidx + 1] = i;
			// Increment the counter.
			m_Grid[cell]++;
		}
	}
}

void Game::UpdateParticleCollisions(float dt)
{
	// Loop over all cells in the grid (except last row and last column).
	for (int y = 0; y < GRID_RESOLUTION - 1; y++)
		for (int x = 0; x < GRID_RESOLUTION - 1; x++)
		{
			int cell = (x + y * GRID_RESOLUTION) * CELL_CAPACITY;
			// Number of particles in the cell.
			int nParticles = m_Grid[cell];

			for (int i = 0; i < nParticles; i++)
			{
				uint p1 = m_Grid[cell + i + 1];

				/* Check for collisions within the cell. */
				for (int j = i + 1; j < nParticles; j++)
				{
					uint p2 = m_Grid[cell + j + 1];
					if (CheckCollision(p1, p2, dt)) ResolveCollision(p1, p2);
				}

				/*  Check for collision with neighbouring cells. */

				// Cell to the right.
				int nextCell = (x + 1 + y * GRID_RESOLUTION) * CELL_CAPACITY;
				for (int j = 0; j < m_Grid[nextCell]; j++)
				{
					uint p2 = m_Grid[nextCell + j + 1];
					if (CheckCollision(p1, p2, dt)) ResolveCollision(p1, p2);
				}

				// Cell below.
				nextCell = (x + (y + 1) * GRID_RESOLUTION) * CELL_CAPACITY;
				for (int j = 0; j < m_Grid[nextCell]; j++)
				{
					uint p2 = m_Grid[nextCell + j + 1];
					if (CheckCollision(p1, p2, dt)) ResolveCollision(p1, p2);
				}

				// Cell below to the right.
				nextCell = (x + 1 + (y + 1) * GRID_RESOLUTION) * CELL_CAPACITY;
				for (int j = 0; j < m_Grid[nextCell]; j++)
				{
					uint p2 = m_Grid[nextCell + j + 1];
					if (CheckCollision(p1, p2, dt)) ResolveCollision(p1, p2);
				}
			}
		}

	// Handle collisions in the last column.
	for (int y = 0; y < GRID_RESOLUTION - 1; y++)
	{
		int cell = (0 + y * GRID_RESOLUTION) * CELL_CAPACITY;
		// Number of particles in the cell.
		int nParticles = m_Grid[cell];
		// Check for collision cell below.
		for (int i = 0; i < nParticles; i++)
		{
			uint p1 = m_Grid[cell + i + 1];

			/* Check for collisions within the cell. */
			for (int j = i + 1; j < nParticles; j++)
			{
				uint p2 = m_Grid[cell + j + 1];
				if (CheckCollision(p1, p2, dt)) ResolveCollision(p1, p2);
			}

			/* Check for collisions with cell below. */
			int nextCell = (0 + (y + 1) * GRID_RESOLUTION) * CELL_CAPACITY;
			for (int j = 1; j < m_Grid[nextCell]; j++)
			{
				uint p2 = m_Grid[nextCell + j + 1];
				if (CheckCollision(p1, p2, dt)) ResolveCollision(p1, p2);
			}
		}
	}

	// Handle collisions for the bottom row.
	for (int x = 0; x < GRID_RESOLUTION - 1; x++)
	{
		int cell = (x + (GRID_RESOLUTION - 1) * GRID_RESOLUTION) * CELL_CAPACITY;
		// Number of particles in the cell.
		int nParticles = m_Grid[cell];
		// Check for collision cell below.
		for (int i = 0; i < nParticles; i++)
		{
			uint p1 = m_Grid[cell + i + 1];
			/* Check for collisions within the cell. */
			for (int j = i + 1; j < nParticles; j++)
			{
				uint p2 = m_Grid[cell + j + 1];
				if (CheckCollision(p1, p2, dt)) ResolveCollision(p1, p2);
			}
			/* Check for collisions with cell to the right. */
			int nextCell = (x + 1 + (GRID_RESOLUTION - 1) * GRID_RESOLUTION) * CELL_CAPACITY;
			for (int j = 1; j < m_Grid[nextCell]; j++)
			{
				uint p2 = m_Grid[nextCell + j + 1];
				if (CheckCollision(p1, p2, dt)) ResolveCollision(p1, p2);
			}
		}
	}
}

void Game::HandleUserInput(float dt)
{
	// Check if mouse is held down.
	if (Input::MouseLeftButtonDown())
	{
		glm::ivec2 cpos = Input::CursorPosition();
		if (cpos.x < 0 || cpos.y < 0 || cpos.x >= Application::WindowWidth() || cpos.y >= Application::WindowHeight()) return;

		// Convert mouse position to texture position.
		float xscale = Application::RenderWidth() / Application::WindowWidth();
		float yscale = Application::RenderHeight() / Application::WindowHeight();

		glm::vec2 cursorPos = glm::vec2(Input::CursorPosition().x, Input::CursorPosition().y) * glm::vec2(xscale, yscale);

		// Convert cursor pos to grid coordinates.
		int gx = GRID_RESOLUTION * cursorPos.x / Application::RenderWidth();
		int gy = GRID_RESOLUTION * cursorPos.y / Application::RenderHeight();

		int xmin = glm::max(0, gx - 5);
		int xmax = glm::min(GRID_RESOLUTION - 1, gx + 6);
		int ymin = glm::max(0, gy - 2);
		int ymax = glm::min(GRID_RESOLUTION - 1, gy + 3);

		// Apply forces to particles based on the cursor position.
		for (int y = ymin; y < ymax; y++)
			for (int x = xmin; x < xmax; x++)
			{
				// Apply forces to particles in cell.
				int cell = (x + y * GRID_RESOLUTION) * CELL_CAPACITY;
				uint nParticles = m_Grid[cell];

				for (uint i = 0; i < nParticles; i++)
				{
					uint pidx = m_Grid[cell + i + 1];

					glm::vec2 diff = m_Positions[pidx] - cursorPos;
					float sqrdlength = glm::length2(diff);

					// If we happen to exactly click on a particle, ignore it.
					if (sqrdlength == 0.0f || sqrdlength > 128.0f * 128.0f) continue;

					// Apply forces based on reciprocal distance.
					float force = 25.0f * 128.0f * 128.0f / sqrdlength;
					m_Velocities[pidx] += force * diff * dt;

					float speed = glm::length(m_Velocities[pidx]);
					if (speed > MAX_SPEED) m_Velocities[pidx] = (m_Velocities[pidx] / speed) * MAX_SPEED;
				}
			}

	}
}

bool Game::CheckCollision(uint p1, uint p2, float dt)
{
	glm::vec2 p1NextPos = m_Positions[p1] + m_Velocities[p1] * dt;
	glm::vec2 p2NextPos = m_Positions[p2] + m_Velocities[p2] * dt;

	float distSquared = glm::length2(p1NextPos - p2NextPos);

	return distSquared <= (m_Radii[p1] + m_Radii[p2]) * (m_Radii[p1] + m_Radii[p2]);
}

void Game::ResolveCollision(uint p1, uint p2)
{
	// Normal
	glm::vec2 normal = glm::normalize(m_Positions[p2] - m_Positions[p1]);

	// Relative velocity
	glm::vec2 rv = m_Velocities[p2] - m_Velocities[p1];

	// Velocity along the normal
	float velAlongNormal = glm::dot(rv, normal);

	// Do not resolve if velocities are separating
	if (velAlongNormal > 0) return;

	// Calculate impulse scalar
	float j = -(1.0f + RESTITUTION) * velAlongNormal;
	j /= 1 / m_Masses[p1] + 1 / m_Masses[p2];

	// Apply impulse
	glm::vec2 impulse = j * normal;
	m_Velocities[p1] -= (1.0f / m_Masses[p1]) * impulse;
	m_Velocities[p2] += (1.0f / m_Masses[p2]) * impulse;

	// Cap velocity at a maximum speed.
	float p1speed = glm::length(m_Velocities[p1]);
	float p2speed = glm::length(m_Velocities[p2]);

	// Calculate overlap
	float overlap = (m_Radii[p1] + m_Radii[p2]) - glm::length(m_Positions[p1]  - m_Positions[p2]);

	// Correct positions.
	m_Positions[p1] -= overlap * 0.5f * normal;
	m_Positions[p2]  += overlap * 0.5f * normal;
}

void Game::DrawParticle(uint p)
{
	int radius = (int)m_Radii[p];
	int radSquared = radius * radius;
	int cx = (int)m_Positions[p].x, cy = (int)m_Positions[p].y;

	int yStart = glm::max(cy - radius, 0);
	int yEnd = glm::min((int)Application::RenderHeight() - 1, cy + radius);
	int xStart = glm::max(cx - radius, 0);
	int xEnd = glm::min((int)Application::RenderWidth() - 1, cx + radius);

	// Particle speed.
	glm::vec2 direction = glm::normalize(m_Velocities[p]);
	uint color = 0x000000FF | ((uint)(direction.x * 127.0f + 128.0f) << 24) | ((uint)(direction.y * 127.0f + 128.0f) << 16);

	// Iterate over pixels and draw the circle.
	for (int y = yStart; y < yEnd; y++)
		for (int x = xStart; x < xEnd; x++)
		{
			// Check if the current pixel is within the circle.
			int dx = x - cx, dy = y - cy;

			if (dx * dx + dy * dy < radSquared) Application::Screen()->PlotPixel(color, x, y);
		}
}


Game::Game()
{
	// Initialize opencl.
	m_clContext = new clContext(true);
	m_clProgram = new clProgram(m_clContext, "kernels.cl");
	m_clQueue = new clCommandQueue(m_clContext, false, true);

	m_clPosKernel = new clKernel(m_clProgram, "update_positions");

	m_clPositionBuffer = new clBuffer(m_clContext, sizeof(float) * 2 * N_PARTICLES, BufferFlags::READ_WRITE);
	m_clVelocityBuffer = new clBuffer(m_clContext, sizeof(float) * 2 * N_PARTICLES, BufferFlags::READ_WRITE);
	m_clPosKernel->SetArgument(1, m_clPositionBuffer);
	m_clPosKernel->SetArgument(2, m_clVelocityBuffer);


	// Resize the window.
	Application::SetWindowSize(1024, 1024, true);

	// Initialize seed for deterministic sim.
	srand(0);

	// Assign particles in the simulation random positions.
	for (size_t i = 0; i < N_PARTICLES; i++)
	{
		float rrandmax = 1.0f / (float)RAND_MAX;

		m_Positions[i] = glm::vec2(rand() % Application::RenderWidth(), rand() % Application::RenderHeight());
		m_Velocities[i] = glm::vec2((float)rand() * rrandmax - 0.5f, (float)rand() * rrandmax - 0.5f) * SPEED_MOD * 2.0f;
		m_Radii[i] = 6.0f + 3.0f * (float)rand() * rrandmax;
		m_Masses[i] = m_Radii[i] * 4.0f;
		m_Colors[i] = (rand() % 255 << 24) | (rand() % 255 << 16) | (rand() % 255 << 8) | 255u;
	}

}

Game::~Game()
{

	delete m_clContext;
}

void Game::Tick(float dt)
{
	// Update average frametime.
	m_AvgFrameTime = 0.99f * m_AvgFrameTime + 0.01 * dt;

	// Build the particle grid.
	UpdateParticleGrid();
	// Handle collisions using the grid.
	UpdateParticleCollisions(dt);

	// Apply forces based on user input.
	HandleUserInput(dt);

	// Copy velocities and positions to the device.
	m_clPositionBuffer->CopyToDevice(m_clQueue, m_Positions, 0, sizeof(float) * 2 * N_PARTICLES, false);
	m_clVelocityBuffer->CopyToDevice(m_clQueue, m_Velocities, 0, sizeof(float) * 2 * N_PARTICLES, false);
	// Set the delta time argument for the kernel.
	m_clPosKernel->SetArgument(0, &dt, sizeof(float));
	// Enqueue the kernel for execution (no need to synchronize because of the copy.
	m_clPosKernel->Enqueue(m_clQueue, N_PARTICLES, 1024);
	// Copy result back to host.
	m_clPositionBuffer->CopyToHost(m_clQueue, m_Positions, 0, sizeof(float) * 2 * N_PARTICLES, true);

	// Update positions and heck collision with screen boundaries.
	for (int i = 0; i < N_PARTICLES; i++)
	{
		// Check if outside of boundary.
		if (m_Positions[i].x - m_Radii[i] < 0.0f) m_Positions[i].x = m_Radii[i], m_Velocities[i].x *= -1.0f;
		if (m_Positions[i].y - m_Radii[i] < 0.0f) m_Positions[i].y = m_Radii[i], m_Velocities[i].y *= -1.0f;
		if (m_Positions[i].x + m_Radii[i] >= Application::RenderWidth()) m_Positions[i].x = Application::RenderWidth() - m_Radii[i] - 1.0f, m_Velocities[i].x *= -1.0f;
		if (m_Positions[i].y + m_Radii[i] >= Application::RenderHeight()) m_Positions[i].y = Application::RenderHeight() - m_Radii[i] - 1.0f, m_Velocities[i].y *= -1.0f;
	}
}

void Game::Draw(float dt)
{
	// Clear the screen.
	Application::Screen()->Clear();
	// Render the particles.

	for (uint i = 0; i < N_PARTICLES; i++) DrawParticle(i);

	Application::Screen()->SyncPixels();
}

void Game::RenderGUI(float dt)
{
	// GUI code goes here. 

	// feed inputs to dear imgui, start new frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	const static char* windowTitle = "Debug";
	static bool display = true;
	ImGui::Begin(windowTitle, &display, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::SetWindowFontScale(1.25f);
	ImGui::Text("Frame-time: %.1f", dt * 1000.0f);
	ImGui::End();

	// Render dear imgui into screen
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
