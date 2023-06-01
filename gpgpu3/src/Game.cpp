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
		uint gx = m_Particles[i].pos.x / cellWidth;
		uint gy = m_Particles[i].pos.y / cellHeight;

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
				Particle& p1 = m_Particles[m_Grid[cell + i + 1]];

				/* Check for collisions within the cell. */
				for (int j = i + 1; j < nParticles; j++)
				{
					Particle& p2 = m_Particles[m_Grid[cell + j + 1]];
					if (CheckCollision(p1, p2, dt)) ResolveCollision(p1, p2);
				}

				/*  Check for collision with neighbouring cells. */

				// Cell to the right.
				int nextCell = (x + 1 + y * GRID_RESOLUTION) * CELL_CAPACITY;
				for (int j = 0; j < m_Grid[nextCell]; j++)
				{
					Particle& p2 = m_Particles[m_Grid[nextCell + j + 1]];
					if (CheckCollision(p1, p2, dt)) ResolveCollision(p1, p2);
				}

				// Cell below.
				nextCell = (x + (y + 1) * GRID_RESOLUTION) * CELL_CAPACITY;
				for (int j = 0; j < m_Grid[nextCell]; j++)
				{
					Particle& p2 = m_Particles[m_Grid[nextCell + j + 1]];
					if (CheckCollision(p1, p2, dt)) ResolveCollision(p1, p2);
				}

				// Cell below to the right.
				nextCell = (x + 1 + (y + 1) * GRID_RESOLUTION) * CELL_CAPACITY;
				for (int j = 0; j < m_Grid[nextCell]; j++)
				{
					Particle& p2 = m_Particles[m_Grid[nextCell + j + 1]];
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
			Particle& p1 = m_Particles[m_Grid[cell + i + 1]];

			/* Check for collisions within the cell. */
			for (int j = i + 1; j < nParticles; j++)
			{
				Particle& p2 = m_Particles[m_Grid[cell + j + 1]];
				if (CheckCollision(p1, p2, dt)) ResolveCollision(p1, p2);
			}

			/* Check for collisions with cell below. */
			int nextCell = (0 + (y + 1) * GRID_RESOLUTION) * CELL_CAPACITY;
			for (int j = 1; j < m_Grid[nextCell]; j++)
			{
				Particle& p2 = m_Particles[m_Grid[nextCell + j + 1]];
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
			Particle& p1 = m_Particles[m_Grid[cell + i + 1]];
			/* Check for collisions within the cell. */
			for (int j = i + 1; j < nParticles; j++)
			{
				Particle& p2 = m_Particles[m_Grid[cell + j + 1]];
				if (CheckCollision(p1, p2, dt)) ResolveCollision(p1, p2);
			}
			/* Check for collisions with cell to the right. */
			int nextCell = (x + 1 + (GRID_RESOLUTION - 1) * GRID_RESOLUTION) * CELL_CAPACITY;
			for (int j = 1; j < m_Grid[nextCell]; j++)
			{
				Particle& p2 = m_Particles[m_Grid[nextCell + j + 1]];
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
					Particle& p = m_Particles[m_Grid[cell + i + 1]];
					glm::vec2 diff = p.pos - cursorPos;
					float sqrdlength = glm::length2(diff);

					// If we happen to exactly click on a particle, ignore it.
					if (sqrdlength == 0.0f) continue;

					// Apply forces based on reciprocal distance.
					float force = 25.0f * 128.0f * 128.0f / sqrdlength;
					p.velocity += force * diff * dt;

					float speed = glm::length(p.velocity);
					if (speed > MAX_SPEED) p.velocity = (p.velocity / speed) * MAX_SPEED;
				}
			}

	}
}

bool Game::CheckCollision(const Particle& p1, const Particle& p2, float dt)
{
	glm::vec2 p1NextPos = p1.pos + p1.velocity * dt;
	glm::vec2 p2NextPos = p2.pos + p2.velocity * dt;

	float distSquared = glm::length2(p1NextPos - p2NextPos);

	return distSquared <= (p1.radius + p2.radius) * (p1.radius + p2.radius);
}

void Game::ResolveCollision(Particle& p1, Particle& p2)
{
	// Normal
	glm::vec2 normal = glm::normalize(p2.pos - p1.pos);

	// Relative velocity
	glm::vec2 rv = p2.velocity - p1.velocity;

	// Velocity along the normal
	float velAlongNormal = glm::dot(rv, normal);

	// Do not resolve if velocities are separating
	if (velAlongNormal > 0) return;

	// Calculate impulse scalar
	float j = -(1.0f + RESTITUTION) * velAlongNormal;
	j /= 1 / p1.mass + 1 / p2.mass;

	// Apply impulse
	glm::vec2 impulse = j * normal;
	p1.velocity -= (1.0f / p1.mass) * impulse;
	p2.velocity += (1.0f / p2.mass) * impulse;

	// Cap velocity at a maximum speed.
	float p1speed = glm::length(p1.velocity);
	float p2speed = glm::length(p2.velocity);

	// Calculate overlap
	float overlap = (p1.radius + p2.radius) - glm::length(p1.pos - p2.pos);

	// Correct positions.
	p1.pos -= overlap * 0.5f * normal;
	p2.pos += overlap * 0.5f * normal;
}

void Game::DrawParticle(Particle& p)
{
	int radius = (int)p.radius;
	int radSquared = radius * radius;
	int cx = (int)p.pos.x, cy = (int)p.pos.y;

	int yStart = glm::max(cy - radius, 0);
	int yEnd = glm::min((int)Application::RenderHeight() - 1, cy + radius);
	int xStart = glm::max(cx - radius, 0);
	int xEnd = glm::min((int)Application::RenderWidth() - 1, cx + radius);

	// Particle speed.
	glm::vec2 direction = glm::normalize(p.velocity);
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
	// Resize the window.
	Application::SetWindowSize(1024, 1024, true);

	// Initialize seed for deterministic sim.
	srand(0);

	// Assign particles in the simulation random positions.
	for (size_t i = 0; i < N_PARTICLES; i++)
	{
		float rrandmax = 1.0f / (float)RAND_MAX;

		glm::vec2 pos = glm::vec2(rand() % Application::RenderWidth(), rand() % Application::RenderHeight());
		glm::vec2 vel = glm::vec2((float)rand() * rrandmax - 0.5f, (float)rand() * rrandmax - 0.5f) * SPEED_MOD * 2.0f;
		float radius = 6.0f + 3.0f * (float)rand() * rrandmax;
		float mass = radius * 4.0f;
		uint color = (rand() % 255 << 24) | (rand() % 255 << 16) | (rand() % 255 << 8) | 255u;

		m_Particles[i] = { pos, vel, mass, radius, color };
	}

}

Game::~Game()
{
	// Perform any saving operations and free allocated memory.
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

	// Update positions and heck collision with screen boundaries.
	for (int i = 0; i < N_PARTICLES; i++)
	{
		Particle& p = m_Particles[i];

		// Update particle position.
		p.pos += p.velocity * dt;

		// Check if outside of boundary.
		if (p.pos.x - p.radius < 0.0f) p.pos.x = p.radius, p.velocity.x *= -1.0f;
		if (p.pos.y - p.radius < 0.0f) p.pos.y = p.radius, p.velocity.y *= -1.0f;
		if (p.pos.x + p.radius >= Application::RenderWidth()) p.pos.x = Application::RenderWidth() - p.radius - 1.0f, p.velocity.x *= -1.0f;
		if (p.pos.y + p.radius >= Application::RenderHeight()) p.pos.y = Application::RenderHeight() - p.radius - 1.0f, p.velocity.y *= -1.0f;
	}
}

void Game::Draw(float dt)
{
	// Clear the screen.
	Application::Screen()->Clear();
	// Render the particles.

	for (int i = 0; i < N_PARTICLES; i++) DrawParticle(m_Particles[i]);

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
