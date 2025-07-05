# MeshVista - Implementation Details 🧠

This document provides a comprehensive explanation of the implementation details for the MeshVista 3D mesh viewer. It covers the technical aspects of each feature, the architecture of the application, and the design decisions made during development.

## Table of Contents
- [Architecture Overview](#architecture-overview)
- [OFF File Format Parser](#off-file-format-parser)
- [Mesh Rendering Pipeline](#mesh-rendering-pipeline)
- [Shader Implementation](#shader-implementation)
- [Camera System](#camera-system)
- [Lighting System](#lighting-system)
- [Interactive Features](#interactive-features)
- [Performance Considerations](#performance-considerations)

## Architecture Overview

MeshVista follows a modular architecture consisting of the following components:

1. **Main Application** (`main.cpp`): Handles window creation, OpenGL context setup, and the main rendering loop.
2. **Mesh Loader** (`MeshLoader.cpp`/`.h`): Manages the loading and preparation of 3D mesh data.
3. **OFF Reader** (`OFFReader.cpp`/`.h`): Specialized parser for the OFF file format.
4. **Shader Manager**: Handles the compilation, linking, and usage of GLSL shaders.
5. **UI System**: ImGui-based interface for interactive controls.
6. **Camera System**: Manages viewpoints and navigation through the 3D scene.

The application follows an event-driven architecture where user inputs trigger various actions such as model transformations, camera movements, and rendering mode changes.

## OFF File Format Parser

The OFF (Object File Format) parser is implemented in the `OFFReader` class with the following functionality:

```cpp
// Key steps in the OFF parsing process:
1. Read the OFF header
2. Parse vertex count, face count, and edge count
3. Read vertex coordinates (x, y, z)
4. Read face definitions (typically triangles)
5. Compute bounding box for normalization
6. Center and scale the model
```

The parser handles potential file format issues and calculates a bounding box to normalize the model scale:

```cpp
// Computing the bounding box
void OFFReader::computeBoundingBox() {
    // Initialize min/max with first vertex
    minBounds = vertices[0];
    maxBounds = vertices[0];
    
    // Find min/max for each coordinate
    for (const auto& vertex : vertices) {
        minBounds.x = std::min(minBounds.x, vertex.x);
        minBounds.y = std::min(minBounds.y, vertex.y);
        minBounds.z = std::min(minBounds.z, vertex.z);
        
        maxBounds.x = std::max(maxBounds.x, vertex.x);
        maxBounds.y = std::max(maxBounds.y, vertex.y);
        maxBounds.z = std::max(maxBounds.z, vertex.z);
    }
}
```

## Mesh Rendering Pipeline

The rendering pipeline consists of several stages:

1. **Data Preparation**: Vertices, normals, and faces are loaded into OpenGL buffers.
2. **Shader Binding**: Appropriate shaders are bound for the current rendering mode.
3. **Transformation**: Model, view, and projection matrices are applied.
4. **Lighting Calculation**: Phong shading is applied if enabled.
5. **Fragment Processing**: Special effects like depth-based coloring are applied.
6. **Output Merging**: Final pixel colors are determined and written to the framebuffer.

The rendering pipeline is managed by setting up Vertex Array Objects (VAOs) and Vertex Buffer Objects (VBOs):

```cpp
// Setting up VAO and VBOs
GLuint VAO, VBO_vertices, VBO_normals, EBO;
glGenVertexArrays(1, &VAO);
glGenBuffers(1, &VBO_vertices);
glGenBuffers(1, &VBO_normals);
glGenBuffers(1, &EBO);

glBindVertexArray(VAO);

// Bind vertex data
glBindBuffer(GL_ARRAY_BUFFER, VBO_vertices);
glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
glEnableVertexAttribArray(0);

// Bind normal data
glBindBuffer(GL_ARRAY_BUFFER, VBO_normals);
glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
glEnableVertexAttribArray(1);

// Bind indices
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
```

## Shader Implementation

The application uses three main shader types:

### Vertex Shader

The vertex shader is responsible for transforming vertices and preparing data for the fragment shader:

```glsl
// Key portions of vertex.glsl
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
```

### Fragment Shader

The fragment shader implements the Phong (Blinn-Phong) lighting model and depth-based coloring:

```glsl
// Key portions of fragment.glsl
#version 330 core
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

// Light structure
struct Light {
    vec3 position;
    vec3 color;
    float intensity;
    bool enabled;
};

uniform Light lights[3];
uniform vec3 viewPos;
uniform bool usePhongShading;
uniform bool useDepthColoring;
uniform vec3 nearColor;
uniform vec3 farColor;

void main() {
    // Base material properties
    vec3 objectColor = vec3(0.8, 0.8, 0.8);
    vec3 normal = normalize(Normal);
    
    // Phong shading implementation
    if (usePhongShading) {
        vec3 result = vec3(0.0);
        
        // Ambient component (global)
        float ambientStrength = 0.1;
        vec3 ambient = ambientStrength * objectColor;
        result += ambient;
        
        for (int i = 0; i < 3; i++) {
            if (lights[i].enabled) {
                // Diffuse component
                vec3 lightDir = normalize(lights[i].position - FragPos);
                float diff = max(dot(normal, lightDir), 0.0);
                vec3 diffuse = diff * lights[i].color * lights[i].intensity;
                
                // Specular component (Blinn-Phong)
                float specularStrength = 0.5;
                vec3 viewDir = normalize(viewPos - FragPos);
                vec3 halfwayDir = normalize(lightDir + viewDir);
                float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
                vec3 specular = specularStrength * spec * lights[i].color * lights[i].intensity;
                
                result += diffuse + specular;
            }
        }
        
        FragColor = vec4(result, 1.0);
    }
    
    // Depth-based coloring
    if (useDepthColoring) {
        float depth = gl_FragCoord.z / gl_FragCoord.w;
        float normalizedDepth = clamp((depth - 0.1) / 3.0, 0.0, 1.0);
        vec3 depthColor = mix(nearColor, farColor, normalizedDepth);
        
        if (usePhongShading) {
            // Blend with lighting
            FragColor = vec4(FragColor.rgb * 0.7 + depthColor * 0.3, 1.0);
        } else {
            FragColor = vec4(depthColor, 1.0);
        }
    }
}
```

### Geometry Shader

The geometry shader is used for the exploded view feature, displacing faces along their normals:

```glsl
// Key portions of geometry.glsl
#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec3 gNormal[];
in vec3 gFragPos[];

out vec3 FragPos;
out vec3 Normal;

uniform bool explodeView;
uniform float explodeAmount;
uniform vec3 modelCenter;

void main() {
    vec3 faceNormal = normalize(gNormal[0] + gNormal[1] + gNormal[2]) / 3.0;
    vec3 faceCentroid = (gFragPos[0] + gFragPos[1] + gFragPos[2]) / 3.0;
    vec3 explodeDir = normalize(faceCentroid - modelCenter);
    
    // Process each vertex of the triangle
    for (int i = 0; i < 3; i++) {
        vec3 pos = gFragPos[i];
        
        // Apply explosion if enabled
        if (explodeView) {
            pos += explodeDir * explodeAmount;
        }
        
        gl_Position = projection * view * vec4(pos, 1.0);
        FragPos = pos;
        Normal = gNormal[i];
        EmitVertex();
    }
    
    EndPrimitive();
}
```

## Camera System

The camera system supports two modes:

1. **Orbital Mode**: The default mode where the camera rotates around the model.
2. **Fly-through Mode**: First-person camera that allows moving through the scene.

The camera implementation uses a look-at matrix for view transformation:

```cpp
// Camera position calculation for orbital mode
float radius = 3.0f;
float camX = sin(glm::radians(yaw)) * cos(glm::radians(pitch)) * radius;
float camY = sin(glm::radians(pitch)) * radius;
float camZ = cos(glm::radians(yaw)) * cos(glm::radians(pitch)) * radius;

// Camera in fly-through mode
glm::vec3 front;
front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
front.y = sin(glm::radians(pitch));
front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
cameraFront = glm::normalize(front);

// View matrix calculation
view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
```

## Lighting System

The lighting system supports up to three customizable lights. Each light has:

1. **Position**: 3D coordinates in the scene.
2. **Color**: RGB color of the light.
3. **Intensity**: Brightness factor.
4. **Enabled/Disabled State**: Toggle for each light.

Lights are implemented as structures in the shader and controlled via uniform variables:

```cpp
// Light structure in C++
struct Light {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    bool enabled;
};

// Setting light uniforms
shader.setVec3("lights[0].position", lights[0].position);
shader.setVec3("lights[0].color", lights[0].color);
shader.setFloat("lights[0].intensity", lights[0].intensity);
shader.setBool("lights[0].enabled", lights[0].enabled);
```

## Interactive Features

### Normal Calculation

Vertex normals are calculated by averaging the normals of all faces that share the vertex:

```cpp
// Calculate face normals
for (size_t i = 0; i < indices.size(); i += 3) {
    unsigned int idx0 = indices[i];
    unsigned int idx1 = indices[i + 1];
    unsigned int idx2 = indices[i + 2];
    
    glm::vec3 v0 = vertices[idx0];
    glm::vec3 v1 = vertices[idx1];
    glm::vec3 v2 = vertices[idx2];
    
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));
    
    // Accumulate face normals for each vertex
    vertexNormals[idx0] += faceNormal;
    vertexNormals[idx1] += faceNormal;
    vertexNormals[idx2] += faceNormal;
}

// Normalize accumulated normals
for (auto& normal : vertexNormals) {
    normal = glm::normalize(normal);
}
```

### Continuous Rotation

The continuous rotation feature updates the model matrix every frame:

```cpp
// Continuous rotation
if (continuousRotation) {
    float rotationSpeed = 0.5f;  // degrees per frame
    modelRotationAngle += rotationSpeed;
    if (modelRotationAngle > 360.0f) modelRotationAngle -= 360.0f;
    
    glm::mat4 rotationMatrix = glm::rotate(
        glm::mat4(1.0f),
        glm::radians(modelRotationAngle),
        glm::normalize(rotationAxis)
    );
    
    model = rotationMatrix * initialModelMatrix;
}
```

### Exploded View

The exploded view feature separates the faces of the mesh by moving them along their normals:

```cpp
// Enable exploded view in the shader
shader.use();
shader.setBool("explodeView", explodeViewEnabled);
shader.setFloat("explodeAmount", explodeAmount);
shader.setVec3("modelCenter", modelCenter);
```

## Performance Considerations

Several optimizations are implemented to ensure smooth performance:

1. **Vertex Buffer Objects (VBOs)**: All mesh data is stored in GPU memory.
2. **Frustum Culling**: Only objects in the camera's view are rendered.
3. **Back-face Culling**: Faces pointing away from the camera are not rendered.
4. **Shader Optimizations**: Conditional branching in shaders is minimized.

Example of setting up back-face culling:

```cpp
// Enable back-face culling
glEnable(GL_CULL_FACE);
glCullFace(GL_BACK);
glFrontFace(GL_CCW);  // Counter-clockwise winding order
```

---

This implementation document provides technical details about how MeshVista is built. For usage instructions and feature overview, please refer to the main [README.md](README.md).
