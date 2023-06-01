
// Update the particle positions.
__kernel void update_positions(float dt, __global float2* positions, __global float2* velocities)
{
    int idx = get_global_id( 0 );
    positions[idx] += velocities[idx] * dt;
}