#version 330

// #########################################
// Input from geometry shader (or directly from vertex shader if geometry shader is not used)
in vec3 FragPos;
in vec3 FragNormal;

// ##########################################
// Define a light struct to handle multiple lights
struct Light {
    vec3 position;
    vec3 color;
    float intensity;
    bool enabled;
};

// Maximum number of lights
#define MAX_LIGHTS 3

// Lighting uniforms
uniform Light lights[MAX_LIGHTS];           // Array of lights
uniform vec3 viewPos;                       // Camera position for specular calculation
uniform vec3 objectColor;                   // Object color
// Properties of the material
uniform float ambientStrength;              // Ambient light strength
uniform float diffuseStrength;              // Diffuse light strength
uniform float specularStrength;             // Specular light strength
uniform float shininess;                    // Shininess factor for specular highlights

// Depth-based coloring parameters
uniform bool depthColoringEnabled;          // Enable depth-based coloring
uniform float depthColorNear;               // Near distance for depth color
uniform float depthColorFar;                // Far distance for depth color
uniform vec3 depthColorNearValue;           // Color for near distance
uniform vec3 depthColorFarValue;            // Color for far distance

uniform bool explodeViewEnabled;            //  Enable explode view
uniform bool highlightTriangles;            // Highlight triangles in explode view

// ##########################################
layout(location = 0) out vec4 diffuseColor; // Output color

vec3 generateTriangleColor(vec3 position) {
    // Create a pseudo-random color based on position
    // This gives each triangle a unique color
    float r = fract(sin(dot(position.xy, vec2(12.9898, 78.233))) * 43758.5453);
    float g = fract(sin(dot(position.yz, vec2(12.9898, 78.233))) * 43758.5453);
    float b = fract(sin(dot(position.zx, vec2(12.9898, 78.233))) * 43758.5453);
    
    // Brighten the colors a bit so they're not too dark
    return vec3(r * 0.6 + 0.4, g * 0.6 + 0.4, b * 0.6 + 0.4);
}

// ###############################################################################
// [Blinn-Phong Illumination Model] 
// Calculate contribution from a single light
vec3 calculateLight(Light light, vec3 normal, vec3 viewDir) {

    if (!light.enabled) return vec3(0.0);                       // if disabled, return zero contribution 
    
    // Calculate lighting vectors
    vec3 lightDir = normalize(light.position - FragPos);        // Direction from fragment to light
    vec3 halfwayDir = normalize(lightDir + viewDir);            // Halfway vector for Blinn-Phong specular calculation
    
    // Apply light intensity to light color [Effective I]
    vec3 effectiveLight = light.color * light.intensity;        // Scale light color by intensity
    
    // Ambient component [Ka * I]
    vec3 ambient = ambientStrength * effectiveLight;            // Scale ambient light by strength
    
    // Diffuse component    [Kd * I * cos(theta)]
    float diff = max(dot(normal, lightDir), 0.0);               // Diffuse factor
    vec3 diffuse = diffuseStrength * diff * effectiveLight;     // Scale diffuse light by strength
    
    // Specular component (Blinn-Phong model) [Ks * I * (n . h)^alpha]
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess); // Specular factor
    vec3 specular = specularStrength * spec * effectiveLight;       // Scale specular light by strength
    
    return ambient + diffuse + specular;               // Combine all components [Blinn-Phong illumination model]
}

void main()
{
    // ############################################################################################################
    vec3 finalObjectColor;
    
    if (depthColoringEnabled) {
        // Existing depth coloring code
        float distanceToCamera = length(viewPos - FragPos);                                                                     // Calculate distance to camera
        float normalizedDepth = clamp((distanceToCamera - depthColorNear) / (depthColorFar - depthColorNear), 0.0, 1.0);        // Normalize depth value
        finalObjectColor = mix(depthColorNearValue, depthColorFarValue, normalizedDepth);                                       // Interpolate between near and far colors based on normalized depth
    } 
    else if (explodeViewEnabled && highlightTriangles) {
        // For explode view, color each triangle uniquely so they are distinguishable
        finalObjectColor = generateTriangleColor(FragPos);                                                                      // Generate a unique color for each triangle based on its position
    }
    else {
        // Use regular object color
        finalObjectColor = objectColor;                                                                                         // Use the object color as the final color
    }
    
    // Ensure normal is normalized
    vec3 normal = normalize(FragNormal);
    
    // View direction for specular calculation
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Initialize lighting result
    vec3 lightingResult = vec3(0.0);
    
    // Calculate contribution from each light using the Blinn-Phong model
    for (int i = 0; i < MAX_LIGHTS; i++) {
        lightingResult += calculateLight(lights[i], normal, viewDir);
    }
    
    // Final color calculation
    vec3 result = lightingResult * finalObjectColor;
    
    // Ensure result values are in valid range
    result = clamp(result, 0.0, 1.0);
    
    diffuseColor = vec4(result, 1.0);
}