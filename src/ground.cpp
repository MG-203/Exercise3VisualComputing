#include "ground.h"
#include "mygl/geometry.h"

// forward declaration so computeWaveHeight can be called before its definition
static float computeWaveHeight(const Ground &ground, const Vector3D &pos);

Ground groundCreate(const Vector3D &color) {
    Ground ground;
    ground.vertices.resize(grid::vertexPos.size());

    float min_height = std::numeric_limits<float>::infinity();
    float max_height = -std::numeric_limits<float>::infinity();

    /* Transform the grid's vertices and add them to the flag with the correct color */
    for (unsigned i = 0; i < ground.vertices.size(); i++) {
        
        Vector3D pos = grid::vertexPos[i];

        float height = computeWaveHeight(ground, pos);
        pos.y = height;

        ground.vertices[i] = {pos, color};

        min_height = std::min(min_height, height);
        max_height = std::max(max_height, height);
    }
    float heightRange = max_height - min_height;

    Vector3D lowColor = color;
    Vector3D highColor{
        std::min(color.x + 0.3f, 1.0f),
        std::min(color.y + 0.3f, 1.0f),
        std::min(color.z + 0.3f, 1.0f)
    };

    /* Update vertex colors based on height */
    for (unsigned i = 0; i < ground.vertices.size(); i++) {
        float height = ground.vertices[i].pos.y;
        float t = (height - min_height) / heightRange;

        ground.vertices[i].color = lowColor * (1.0f - t) + highColor * t;
    }

    ground.mesh = meshCreate(ground.vertices, grid::indices, GL_DYNAMIC_DRAW, GL_STATIC_DRAW);

    return ground;
}

// Computes the height of the wave at a given position by summing up the contributions of all waves
static float computeWaveHeight(const Ground &ground, const Vector3D &pos) {
    float height = 0.0f;

    for (const WaveParams &wave : ground.waveParamsVec) {
        float phase = wave.omega * (wave.direction.x * pos.x + wave.direction.y * pos.z);
        height += wave.amplitude * sin(phase);
    }

    return height;
}

// Returns the height of the ground at a specific position
float groundGetHeightAt(const Ground &ground, const Vector3D &pos) {
    return computeWaveHeight(ground, pos);
}



void groundDelete(Ground &ground) { 
    meshDelete(ground.mesh);
}
