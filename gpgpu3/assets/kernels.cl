
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

