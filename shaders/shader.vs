#version 330


// #############################################################################################################
// for each vertex get the position and normal
layout(location = 0) in vec3 Position;  
layout(location = 1) in vec3 Normal;

// ##############################################################################################################
// uniform variables for transformation matrices
uniform mat4 gWorld;            // World Matrix                         -->> Needed for transformation to clip space == Projection * View * Model
uniform mat4 gModel;            // Only Model Matrix                    -->> Needed for transformation to world space only rotation and normalisation
uniform mat4 gNormalMatrix;     // Normal transformation matrix         -->> Needed for Normal Transformation

// ##############################################################################################################
// Output to geometry shader
out vec3 VS_FragPos;            // Position in world space      
out vec3 VS_FragNormal;         // Normal in world space       

void main()
{
    
    vec3 transformedNormal = normalize(mat3(gNormalMatrix) * Normal);       // --> Transform normal to world space
    
    vec3 scaledPosition = 0.5 * Position;                                   // --> Scale the model [ Just to fit in the screen ]
    
    gl_Position = gWorld * vec4(scaledPosition, 1.0);                       // --> Final transformation of the vertex position to [clip space]
    
    VS_FragPos = vec3(gModel * vec4(scaledPosition, 1.0));                  // --> Transform position to [world space]
    
    VS_FragNormal = transformedNormal;                                      // --> Pass the transformed normal to the fragment shader   
}