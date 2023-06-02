
// Update the particle positions.
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

__kernel void build_grid
    (
        int resolution, int capacity,
        __global float2* positions, __global unsigned int* grid
    )
{

}