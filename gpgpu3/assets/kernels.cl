#define RESTITUTION 0.9f

/* Updating the particles positions. */

__kernel void update_positions
    (
        float dt, float2 bounds,
        __global float2* positions, __global float2* velocities,
        __global float* radii
    )
{
    int idx = get_global_id( 0 );
    positions[idx] += velocities[idx] * dt;

    if (positions[idx].x - radii[idx] < 0.0f) positions[idx].x = radii[idx], velocities[idx].x *= -1.0f;
	if (positions[idx].y - radii[idx] < 0.0f) positions[idx].y = radii[idx], velocities[idx].y *= -1.0f;
	if (positions[idx].x + radii[idx] >= bounds.x) positions[idx].x = bounds.x - radii[idx] - 1.0f, velocities[idx].x *= -1.0f;
	if (positions[idx].y + radii[idx] >= bounds.y) positions[idx].y = bounds.y - radii[idx] - 1.0f, velocities[idx].y *= -1.0f;
}


/* Building the grid kernels. */

__kernel void reset_grid(int resolution, int capacity, __global unsigned int* grid)
{
    int x = get_global_id( 0 );
    int y = get_global_id( 1 );
    
    grid[(x + y * resolution) * capacity] = 0;
}

__kernel void build_grid
    (
        int resolution, int capacity, int cellwidth, int cellheight,
        __global float2* positions, __global volatile unsigned int* grid
    )
{
    int idx = get_global_id( 0 );

    // Convert particle position to cell coordinates.
    int gx = positions[idx].x / cellwidth;
    int gy = positions[idx].y / cellheight;

    // Compute cell index.
    int cell = (gx + gy * resolution) * capacity;

    // Check if there is still any space left in the cell to place the particle.
    // Here we must use atomic operations to avoid racing conditions.
    int count = atomic_inc(&grid[cell]);
    if (count < capacity - 1) grid[cell + count + 1] = idx; 

    // NOTE! The counters can now exceed the capacity. We should fix this in another kernel!
}

__kernel void fix_counters(int resolution, int capacity, __global unsigned int* grid)
{
    int x = get_global_id( 0 );
    int y = get_global_id( 1 );
    
    int idx = (x + y * resolution) * capacity;
    grid[idx] = min(grid[idx], (unsigned int)capacity - 1);
}


/* User Input */

__kernel void user_input(float dt, float2 cursor, float maxSpeed, __global float2* positions, __global float2* velocities)
{

    int idx = get_global_id( 0 );

    float2 diff = positions[idx] - cursor;
    float sqrdLength = diff.x * diff.x + diff.y * diff.y;

    // If we happen to exactly click on a particle, ignore it.
    if (sqrdLength == 0.0f|| sqrdLength > 128.0f * 128.0f) return;

    // Apply forces based on reciprocal distance.
    float force = 25.0f * 128.0f * 128.0f / sqrdLength;
    velocities[idx] += force * diff * dt;

    float speed = length(velocities[idx]);
    if (speed > maxSpeed) velocities[idx] = (velocities[idx] / speed) * maxSpeed;

}


/* Handle collisions */

bool CheckCollisions(unsigned int p1, unsigned int p2, float dt,
    __global float2* positions, __global float2* velocities, __global float* radii)
{
    float2 p1NextPos = positions[p1] + velocities[p1] * dt;
    float2 p2NextPos = positions[p2] + velocities[p2] * dt;
    float2 diff = p2NextPos - p1NextPos;

    float distSquared = diff.x * diff.x + diff.y * diff.y;

    return distSquared <= (radii[p1] + radii[p2]) * (radii[p1] + radii[p2]);
}

void ResolveCollisions(unsigned int p1, unsigned int p2,
    __global float2* positions, __global float2* velocities, 
    __global float* radii, __global float* mass)
{
    
    // Normal
	float2 normal = normalize(positions[p2] - positions[p1]);

	// Relative velocity
	float2 rv = velocities[p2] - velocities[p1];

	// Velocity along the normal
	float velAlongNormal = dot(rv, normal);

	// Do not resolve if velocities are separating
	if (velAlongNormal > 0) return;

	// Calculate impulse scalar
	float j = -(1.0f + RESTITUTION) * velAlongNormal;
	j /= 1 / mass[p1] + 1 / mass[p2];

	// Apply impulse
	float2 impulse = j * normal;
	velocities[p1] -= (1.0f / mass[p1]) * impulse;
	velocities[p2] += (1.0f / mass[p2]) * impulse;

	// Calculate overlap
	float overlap = (radii[p1] + radii[p2]) - length(positions[p1] - positions[p2]);

	// Correct positions.
	positions[p1] -= overlap * 0.5f * normal;
	positions[p2] += overlap * 0.5f * normal;

}

/*
* Handle particle collisions within a cell.
*/
__kernel void handle_collisions_within
    (
        // Update every call.
        float dt,
        // Set once parameters.
        int resolution, int capacity,
        __global float2* positions, __global float2* velocities,
        __global float* mass, __global float* radii,
        __global unsigned int* grid
    )
{

    // Get id's.
    int x = get_global_id( 0 );
    int y = get_global_id( 1 );

    // Cell id.
    int cell = (x + y * resolution) * capacity;

    // Number of particles in the cell.
    int n = grid[cell];
    // No particles.
    if (n == 0) return;

    // Check for collisions.
    for (int i = 0; i < n - 1; i++)
    {
        // Current particle. 
        unsigned int p1 = grid[cell + i + 1];

        // Remaining particles.
        for (int j = i + 1; j < n; j++)
        {
            unsigned int p2 = grid[cell + j + 1];
            if (CheckCollisions(p1, p2, dt, positions, velocities, radii))
                ResolveCollisions(p1, p2, positions, velocities, radii, mass);
        }
    }
}

__kernel void intersect_horizontal_neighbors
    (
        float dt, int even,
        // Set once parameters.
        int resolution, int capacity,
        __global float2* positions, __global float2* velocities,
        __global float* mass, __global float* radii,
        __global unsigned int* grid
    )
{
    // Get id's.
    int x = get_global_id( 0 ) * 2 + select(0, 1, even == 0 );
    int y = get_global_id( 1 );

    if (x > resolution - 2) return;

    // Cell id.
    int cell = (x + y * resolution) * capacity;

    // Number of particles in the cell.
    int n = grid[cell];
    // No particles.
    if (n == 0) return;

    // Check for collisions.
    for (int i = 0; i < n - 1; i++)
    {
        // Current particle. 
        unsigned int p1 = grid[cell + i + 1];

        // Find neighbor cell index.
        int neighbor = (x + 1 + y * resolution) * capacity;
        int nNeighbor = grid[neighbor];

        for (int j = 0; j < nNeighbor; j++)
        {
            // find the particle index.
            unsigned int p2 = grid[neighbor + j + 1];

            // Check for collisions.
            if (CheckCollisions(p1, p2, dt, positions, velocities, radii))
                ResolveCollisions(p1, p2, positions, velocities, radii, mass);
            
        }
    }
}

__kernel void intersect_vertical_neighbors
    (
        float dt, int even,
        // Set once parameters.
        int resolution, int capacity,
        __global float2* positions, __global float2* velocities,
        __global float* mass, __global float* radii,
        __global unsigned int* grid
    )
{
    // Get id's.
    int x = get_global_id( 0 );
    int y = get_global_id( 1 ) * 2 + select(0, 1, even == 0 );

    if (y > resolution - 2) return;

    // Cell id.
    int cell = (x + y * resolution) * capacity;

    // Number of particles in the cell.
    int n = grid[cell];
    // No particles.
    if (n == 0) return;

    // Check for collisions.
    for (int i = 0; i < n - 1; i++)
    {
        // Current particle. 
        unsigned int p1 = grid[cell + i + 1];

        // Find neighbor cell index.
        int neighbor = (x + (y + 1) * resolution) * capacity;
        int nNeighbor = grid[neighbor];

        for (int j = 0; j < nNeighbor; j++)
        {
            // find the particle index.
            unsigned int p2 = grid[neighbor + j + 1];
            
            // Check for collisions.
            if (CheckCollisions(p1, p2, dt, positions, velocities, radii))
                ResolveCollisions(p1, p2, positions, velocities, radii, mass);
        }
    }
}

__kernel void intersect_diagonal_neighbors
    (
        float dt, int even,
        // Set once parameters.
        int resolution, int capacity,
        __global float2* positions, __global float2* velocities,
        __global float* mass, __global float* radii,
        __global unsigned int* grid
    )
{
        // Get id's.
    int x = get_global_id( 0 );
    int y = get_global_id( 1 ) * 2 + select(0, 1, even == 0 );

    if (y > resolution - 2) return;

    // Cell id.
    int cell = (x + y * resolution) * capacity;

    // Number of particles in the cell.
    int n = grid[cell];
    // No particles.
    if (n == 0) return;

    // Check for collisions.
    for (int i = 0; i < n - 1; i++)
    {
        // Current particle. 
        unsigned int p1 = grid[cell + i + 1];

        // Right diagonal.
        if (x < resolution - 1)
        {
            int neighbor = ((x + 1) + (y + 1) * resolution) * capacity;
            int nNeighbor = grid[neighbor];

            for (int j = 0; j < nNeighbor; j++)
            {
                // find the particle index.
                unsigned int p2 = grid[neighbor + j + 1];
                
                // Check for collisions.
                if (CheckCollisions(p1, p2, dt, positions, velocities, radii))
                    ResolveCollisions(p1, p2, positions, velocities, radii, mass);
            }
        }

        // Left diagonal.
        if (x > 0)
        {
            int neighbor = ((x - 1) + (y + 1) * resolution) * capacity;
            int nNeighbor = grid[neighbor];

            for (int j = 0; j < nNeighbor; j++)
            {
                // find the particle index.
                unsigned int p2 = grid[neighbor + j + 1];
                
                // Check for collisions.
                if (CheckCollisions(p1, p2, dt, positions, velocities, radii))
                    ResolveCollisions(p1, p2, positions, velocities, radii, mass);
            }
        }
       
    }
}