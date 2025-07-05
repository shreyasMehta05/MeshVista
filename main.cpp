#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "file_utils.h"
#include "math_utils.h"
#include "models/OFFReader.h"
#include <vector>

#define GL_SILENCE_DEPRECATION

#define MAX_LIGHTS 3


/********************************************************************/
/*   Variables */

char theProgramTitle[] = "Sample";
int theWindowWidth = 700, theWindowHeight = 700;
int theWindowPositionX = 40, theWindowPositionY = 40;
bool isFullScreen = false;
bool isAnimating = true;
float rotation = 0.0f;
GLuint VBO, VAO, IBO;

// #####################################################################################
// #####################################################################################
// #####################################################################################
/*			 Model variables			 */
OffModel* model = nullptr;
std::vector<Vector3f> modelVertices; 			// --> store the loaded vertices
std::vector<Vector3f> modelNormals;  			// --> store the model normals for each vertex
std::vector<Vector3f> faceNormals;   			// --> calculate the face normals
std::vector<unsigned int> modelIndices; 		// --> store the indices for the triangles
int vertexCount = 0; 							// --> number of vertices
int indexCount = 0; 							// --> number of indices
char modelPath[256] = "models/2oar.off"; 		// --> path name
bool modelLoaded = false; 						// --> flag to check if model is loaded || model == nullptr
// #######################################################################################
int renderMode = 0; 							// Render mode (0 = solid, 1 = wireframe)
int normalMode = 0; 							// Normal visualization mode (0 = none, 1 = vertex normals, 2 = face normals)
float normalLength = 0.1f;  					// --> Length of normal visualization lines
float orthoSize = 2.0f; 						// -->>> Orthographic projection size
// #######################################################################################
/* Constants - Already Mentioned */
const int ANIMATION_DELAY = 20; /* milliseconds between rendering */
const char *pVSFileName = "shaders/shader.vs";
const char *pFSFileName = "shaders/shader.fs";
// #######################################################################################
// View - Fly mode variables
bool flyThroughMode = false; 								// --> Toggle for fly-through mode
Vector3f cameraPosition = Vector3f(0.0f, 0.0f, 5.0f); 		// --> Camera position (eye) point default is (0, 0, 5) in world coordinates
Vector3f cameraTarget = Vector3f(0.0f, 0.0f, 0.0f); 		// --> Camera target (look-at) point Default is the origin
Vector3f cameraUp = Vector3f(0.0f, 1.0f, 0.0f); 			// --> Camera up vector (Y-axis)
float cameraSpeed = 0.05f; 									// --> Camera movement speed
float cameraSensitivity = 0.005f;							// --> Camera mouse sensitivity
float cameraYaw = -90.0f;   								// --> Yaw stands for the rotation about the Y-axis i.e. left-right movement -90 ==> right
float cameraPitch = 0.0f;									// --> Pitch stands for the rotation about the X-axis i.e. up-down movement
bool firstMouse = true;										// --> Flag to check if the mouse is moved for the first time
double lastMouseX = 400.0f, lastMouseY = 300.0f;			// --> Last mouse position initialized to the center of the window
bool isDraggingCamera = false; 								// --> Flag to check if the camera is being dragged
double dragStartX = 0, dragStartY = 0;						// --> Start position of the drag
float dragStartYaw = 0, dragStartPitch = 0;					// --> Start yaw and pitch of the drag

// #######################################################################################
bool keys[1024] = {false}; 									// --> Array to store the state of keys
// ####################################################################################
// Uniform locations for the shader program
GLuint gWorldLocation;		   // To pass the world matrix
GLuint gModelMatrixLocation;   // To pass the model matrix
GLuint gNormalMatrixLocation;  // To pass the normal transformation matrix
GLuint gViewPosLocation;       // To pass camera position for specular calculation
// ###################################################################################
// Add these variables for lighting control
float ambientStrength = 0.2f;   		// Ambient light strength	
float diffuseStrength = 0.7f;			// Diffuse light strength
float specularStrength = 0.5f;			// Specular light strength
float shininess = 32.0f;				// Shininess factor for specular highlights 
Vector3f lightPosition = Vector3f(5.0f, 5.0f, 5.0f);	// Light position
Vector3f objectColor = Vector3f(0.8f, 0.8f, 0.8f);		// Object color --> Default is gray
// ####################################################################################
// To pass the light properties to the shader
/*				Lighting			*/
GLuint objectColorLocation;				// Object color uniform location
GLuint ambientStrengthLocation;			// Ambient light strength uniform location
GLuint diffuseStrengthLocation;			// Diffuse light strength uniform location
GLuint specularStrengthLocation;		// Specular light strength uniform location
GLuint shininessLocation;				// Shininess uniform location
/*			Depth Coloring		*/
GLuint depthColoringEnabledLocation;	// Toggle for depth-based coloring
GLuint depthColorNearLocation;			// Near plane for depth coloring
GLuint depthColorFarLocation;			// Far plane for depth coloring
GLuint depthColorNearValueLocation;		// Near color value for depth coloring
GLuint depthColorFarValueLocation;		// Far color value for depth coloring
/*			BLOW UP			*/
GLuint explodeViewEnabledLocation;		// Toggle for explode view
GLuint explodeAmountLocation;			// Explode amount for explode view
GLuint modelCenterLocation;				// Model center uniform location
Vector3f modelCenter;					// Model center for normalization
GLuint highlightTrianglesLocation;		// Highlight triangles in explode view
// #####################################################################################
bool depthColoringEnabled = false; 							// Toggle for depth-based coloring
float depthColorNear = 0.1f;       							// Near plane for depth coloring
float depthColorFar = 10.0f;       							// Far plane for depth coloring
Vector3f depthColorNearValue = Vector3f(0.0f, 0.0f, 1.0f);  // Blue for near objects
Vector3f depthColorFarValue = Vector3f(1.0f, 0.0f, 0.0f);   // Red for far objects
// ######################################################################################
bool explodeViewEnabled = false;    // Toggle for explode view
float explodeAmount = 0.1f;         // Controls how far to push triangles apart
// #####################################################################################
// Shader file names
const char *pGSFileName = "shaders/explosion.gs";		// Geometry shader for explosion effect
bool highlightTriangles = false;  						// Toggle for highlighting triangles in explode view
// #####################################################################################
bool useObjectColorForDepth = true;  			// When true, use shades of object color for depth coloring
// #####################################################################################
/*			MULTIPLE LIGHTS			*/
struct Light {  					// Light structure
    Vector3f position;				// Light position
    Vector3f color;					// Light color
    float intensity;				// Light intensity
    bool enabled;					// Light enabled/disabled
};
Light lights[MAX_LIGHTS] = {		// Array of lights
    // Light 1 - Red point light 
    {Vector3f(5.0f, 5.0f, 5.0f), Vector3f(1.0f, 0.2f, 0.2f), 1.0f, true},  
    // Light 2 - Green point light
    {Vector3f(-5.0f, 0.0f, 5.0f), Vector3f(0.2f, 1.0f, 0.2f), 1.0f, true},
    // Light 3 - Blue point light
    {Vector3f(0.0f, -5.0f, -5.0f), Vector3f(0.2f, 0.2f, 1.0f), 1.0f, true}
};
GLuint lightsPositionLocation[MAX_LIGHTS];  	// Light position uniform locations
GLuint lightsColorLocation[MAX_LIGHTS];			// Light color uniform locations
GLuint lightsIntensityLocation[MAX_LIGHTS];		// Light intensity uniform locations
GLuint lightsEnabledLocation[MAX_LIGHTS];		// Light enabled uniform locations
// #####################################################################################
// #####################################################################################
// #####################################################################################

/********************************************************************
  Utility functions
 */

/* post: compute frames per second and display in window's title bar */
void computeFPS()
{
	static int frameCount = 0;
	static int lastFrameTime = 0;
	static char *title = NULL;
	int currentTime;

	if (!title)
		title = (char *)malloc((strlen(theProgramTitle) + 20) * sizeof(char));
	frameCount++;
	currentTime = 0;
	if (currentTime - lastFrameTime > 1000)
	{
		sprintf(title, "%s [ FPS: %4.2f ]",
				theProgramTitle,
				frameCount * 1000.0 / (currentTime - lastFrameTime));
		lastFrameTime = currentTime;
		frameCount = 0;
	}
}

// ######################################################################################
// Calculate the normal of a face defined by three vertices --> Cross btw two edges
Vector3f CalculateFaceNormal(const Vector3f& v1, const Vector3f& v2, const Vector3f& v3) {
    Vector3f edge1 = v2 - v1;
    Vector3f edge2 = v3 - v1;
    Vector3f normal = edge1.Cross(edge2);
    normal.Normalize();
    return normal;
}
// #######################################################################################
// Load the OFF model and process vertices, normals, and faces on actual model vertices without modifying them
bool LoadOFFModel(const char* filename)
{
	// Check if the model is already loaded and free it 
    if (model) {					
        FreeOffModel(model);
        model = nullptr;
    }
    
	// Load the model from the OFF file
    model = readOffFile((char*)filename);  
    if (!model) {
        std::cerr << "Failed to load model: " << filename << std::endl;
        return false;
    }
    
    // Clear Previous vertices, normals, and indices
    modelVertices.clear();
    modelNormals.clear();
    faceNormals.clear();
    modelIndices.clear();
    
    // Store original vertex positions without modification
    for (int i = 0; i < model->numberOfVertices; i++) {
        Vector3f vertex;
        vertex.x = model->vertices[i].x;
        vertex.y = model->vertices[i].y;
        vertex.z = model->vertices[i].z;
        modelVertices.push_back(vertex);
        
        // Initialize vertex normals to zero
        modelNormals.push_back(Vector3f(0.0f, 0.0f, 0.0f));
    }


	// calculate the center of the model
	modelCenter = Vector3f(0.0f, 0.0f, 0.0f);
	if (model) {
		modelCenter.x = (model->minX + model->maxX) / 2.0f;
		modelCenter.y = (model->minY + model->maxY) / 2.0f;
		modelCenter.z = (model->minZ + model->maxZ) / 2.0f;
	}
		
    // Process faces and calculate face normals for each triangle
    for (int i = 0; i < model->numberOfPolygons; i++) {
		// Get the polygon (face) data
        Polygon& poly = model->polygons[i];
        
        if (poly.noSides >= 3) {
            // Get vertices for this face
            Vector3f& v0 = modelVertices[poly.v[0]];
            
            // For each triangle in the polygon
            for (int j = 1; j < poly.noSides - 1; j++) {
                Vector3f& v1 = modelVertices[poly.v[j]];
                Vector3f& v2 = modelVertices[poly.v[j + 1]];
                
                // Calculate face normal for this triangle
                Vector3f normal = CalculateFaceNormal(v0, v1, v2);
                faceNormals.push_back(normal);
                
                // Add indices for the triangle
                modelIndices.push_back(poly.v[0]);
                modelIndices.push_back(poly.v[j]);
                modelIndices.push_back(poly.v[j + 1]);
                
                // Accumulate normal to each vertex of this triangle
                modelNormals[poly.v[0]] += normal;
                modelNormals[poly.v[j]] += normal;
                modelNormals[poly.v[j + 1]] += normal;
                
                // Increment face count for vertices
                model->vertices[poly.v[0]].numIcidentTri++;
                model->vertices[poly.v[j]].numIcidentTri++;
                model->vertices[poly.v[j + 1]].numIcidentTri++;
            }
        }
    }
    
    // Normalize all vertex normals
    for (int i = 0; i < model->numberOfVertices; i++) {
        if (model->vertices[i].numIcidentTri > 0) {
            modelNormals[i].Normalize();
        }
        // Store normals in the model for reference
        model->vertices[i].normal = modelNormals[i];
    }
    
    vertexCount = modelVertices.size();
    indexCount = modelIndices.size();
    
    
    std::cout << "Model loaded: " << filename << std::endl;
    std::cout << "Vertices: " << vertexCount << std::endl;
    std::cout << "Faces: " << faceNormals.size() << std::endl;
    std::cout << "Indices: " << indexCount << std::endl;
    
    return true;
}

// #################################################################################
// Create a normalization matrix to center and scale the model
Matrix4f CreateNormalizationMatrix() {
    if (!model) return Matrix4f(); // Identity matrix if no model
    
    // Calculate center of the model
    float centerX = (model->minX + model->maxX) / 2.0f;
    float centerY = (model->minY + model->maxY) / 2.0f;
    float centerZ = (model->minZ + model->maxZ) / 2.0f;
    
    // Calculate scale factor
    float scale = 2.0f / model->extent;
    
    // Create translation matrix to center the model
    Matrix4f translationMatrix;
	translationMatrix.InitIdentity();
    translationMatrix.m[0][0] = 1.0f;
    translationMatrix.m[0][3] = -centerX;
    translationMatrix.m[1][1] = 1.0f;
    translationMatrix.m[1][3] = -centerY;
    translationMatrix.m[2][2] = 1.0f;
    translationMatrix.m[2][3] = -centerZ;
    translationMatrix.m[3][3] = 1.0f;
    
	
    // Create scaling matrix
    Matrix4f scaleMatrix;
	scaleMatrix.InitIdentity();

    scaleMatrix.m[0][0] = scale;
    scaleMatrix.m[1][1] = scale;
    scaleMatrix.m[2][2] = scale;
    scaleMatrix.m[3][3] = 1.0f;
    
    // Combine matrices: scale after translation
    return scaleMatrix * translationMatrix;
}


// #################################################################################
// function to calculate depth color values based on object color
void UpdateDepthColors() {
    if (useObjectColorForDepth) {
        // Create lighter version of object color for near objects
        depthColorNearValue.x = min(objectColor.x + 0.3f, 1.0f);
        depthColorNearValue.y = min(objectColor.y + 0.3f, 1.0f);
        depthColorNearValue.z = min(objectColor.z + 0.3f, 1.0f);
        
        // Create darker version of object color for far objects
        depthColorFarValue.x = max(objectColor.x - 0.3f, 0.0f);
        depthColorFarValue.y = max(objectColor.y - 0.3f, 0.0f);
        depthColorFarValue.z = max(objectColor.z - 0.3f, 0.0f);
    }
}

// #################################################################################
// Create an interleaved VBO with positions and normals
static void CreateVertexBuffer()
{
    // Create default triangle if no model is loaded
    if (!modelLoaded) {
        modelVertices.clear();
        modelNormals.clear();
        faceNormals.clear();
        modelIndices.clear();
        
        // Default triangle
        modelVertices.push_back(Vector3f(-1.0f, -1.0f, 0.0f));
        modelVertices.push_back(Vector3f(1.0f, -1.0f, 0.0f));
        modelVertices.push_back(Vector3f(0.0f, 1.0f, 0.0f));
        
        // Default normals pointing outward
        modelNormals.push_back(Vector3f(0.0f, 0.0f, 1.0f));
        modelNormals.push_back(Vector3f(0.0f, 0.0f, 1.0f));
        modelNormals.push_back(Vector3f(0.0f, 0.0f, 1.0f));
        
        modelIndices.push_back(0);
        modelIndices.push_back(1);
        modelIndices.push_back(2);
        
        // Default face normal
        faceNormals.push_back(Vector3f(0.0f, 0.0f, 1.0f));
        
        vertexCount = 3;
        indexCount = 3;
    }
	// #################################################
    // Create interleaved vertex data (position, normal) -->
    std::vector<float> vertexData;
    for (int i = 0; i < vertexCount; i++) {
        // Position
        vertexData.push_back(modelVertices[i].x);
        vertexData.push_back(modelVertices[i].y);
        vertexData.push_back(modelVertices[i].z);
        
        // Normal
        vertexData.push_back(modelNormals[i].x);
        vertexData.push_back(modelNormals[i].y);
        vertexData.push_back(modelNormals[i].z);
    }

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Create and populate the VBO with interleaved data
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

    // Create and populate the IBO
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), modelIndices.data(), GL_STATIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    
    // Normal attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    // Create a VAO for normal visualization if needed
    if (normalMode > 0) {
        // Create code for normal visualization (Not needed)
    }
}

static void AddShader(GLuint ShaderProgram, const char *pShaderText, GLenum ShaderType)
{
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0)
	{
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
		exit(0);
	}

	const GLchar *p[1];
	p[0] = pShaderText;
	GLint Lengths[1];
	Lengths[0] = strlen(pShaderText);
	glShaderSource(ShaderObj, 1, p, Lengths);
	glCompileShader(ShaderObj);
	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		exit(1);
	}

	glAttachShader(ShaderProgram, ShaderObj);
}

using namespace std;


// #################################################################
// Made some changes in this function to account for geometry shader
static void CompileShaders()
{
    GLuint ShaderProgram = glCreateProgram();

    if (ShaderProgram == 0)
    {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }

    string vs, fs, gs;

    if (!ReadFile(pVSFileName, vs))
    {
        exit(1);
    }

    if (!ReadFile(pFSFileName, fs))
    {
        exit(1);
    }


	bool gsSuccess = ReadFile(pGSFileName, gs);							// read geometry shader file
    AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);				// Add vertex shader
    AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);           // Add Fragment shader

	if(gsSuccess){
		AddShader(ShaderProgram, gs.c_str(), GL_GEOMETRY_SHADER);       // Add geometry shader
		printf("Successfully loaded geometry shader\n");
	}
	else{
		printf("Failed to load geometry shader\n");
	}

    GLint Success = 0;                                                  // --> success flagfor geometry shader
    GLchar ErrorLog[1024] = {0};                                        // --> error log for geometry shader

    glLinkProgram(ShaderProgram);                                       // --> link the shader program
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);            // --> check for success
    if (Success == 0)
    {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
    }
    
    glBindVertexArray(VAO);                                             // --> bind the vertex array object
    glValidateProgram(ShaderProgram);                                   // --> validate the shader program
    glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);        // --> check for success
    if (!Success)
    {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program1: '%s'\n", ErrorLog);
        exit(1);
    }

    glUseProgram(ShaderProgram);                    // --> use the shader program
    
    // Get uniform locations - uniform locations are used to pass data to the shader program
    gWorldLocation = glGetUniformLocation(ShaderProgram, "gWorld");                     // --> world matrix
    gModelMatrixLocation = glGetUniformLocation(ShaderProgram, "gModel");               // --> model matrix
    gNormalMatrixLocation = glGetUniformLocation(ShaderProgram, "gNormalMatrix");       // --> normal matrix
    gViewPosLocation = glGetUniformLocation(ShaderProgram, "viewPos");                  // --> camera position

	for (int i = 0; i < MAX_LIGHTS; i++) {
		char buffer[100];
		
		// Get light position uniform location
		sprintf(buffer, "lights[%d].position", i);
		lightsPositionLocation[i] = glGetUniformLocation(ShaderProgram, buffer);        
		
		// Get light color uniform location
		sprintf(buffer, "lights[%d].color", i);
		lightsColorLocation[i] = glGetUniformLocation(ShaderProgram, buffer);           
		
		// Get light intensity uniform location
		sprintf(buffer, "lights[%d].intensity", i);
		lightsIntensityLocation[i] = glGetUniformLocation(ShaderProgram, buffer);
		
		// Get light enabled uniform location
		sprintf(buffer, "lights[%d].enabled", i);
		lightsEnabledLocation[i] = glGetUniformLocation(ShaderProgram, buffer);
	}

	for (int i = 0; i < MAX_LIGHTS; i++) {
		glUniform3f(lightsPositionLocation[i], lights[i].position.x, lights[i].position.y, lights[i].position.z);   // --> pass light position   
		glUniform3f(lightsColorLocation[i], lights[i].color.x, lights[i].color.y, lights[i].color.z);               // --> pass light color
		glUniform1f(lightsIntensityLocation[i], lights[i].intensity);                                               // --> pass light intensity
		glUniform1i(lightsEnabledLocation[i], lights[i].enabled ? 1 : 0);                                           // --> pass light enabled/disabled
	}

	objectColorLocation = glGetUniformLocation(ShaderProgram, "objectColor");                   // --> object color
	ambientStrengthLocation = glGetUniformLocation(ShaderProgram, "ambientStrength");           // --> ambient light strength
	diffuseStrengthLocation = glGetUniformLocation(ShaderProgram, "diffuseStrength");           // --> diffuse light strength
	specularStrengthLocation = glGetUniformLocation(ShaderProgram, "specularStrength");         // --> specular light strength
	shininessLocation = glGetUniformLocation(ShaderProgram, "shininess");                       // --> shininess factor for specular highlights

	depthColoringEnabledLocation = glGetUniformLocation(ShaderProgram, "depthColoringEnabled"); // --> toggle for depth-based coloring
    depthColorNearLocation = glGetUniformLocation(ShaderProgram, "depthColorNear");             // --> near plane for depth coloring
    depthColorFarLocation = glGetUniformLocation(ShaderProgram, "depthColorFar");               // --> far plane for depth coloring
    depthColorNearValueLocation = glGetUniformLocation(ShaderProgram, "depthColorNearValue");   // --> near color value for depth coloring
    depthColorFarValueLocation = glGetUniformLocation(ShaderProgram, "depthColorFarValue");     // --> far color value for depth coloring

	explodeViewEnabledLocation = glGetUniformLocation(ShaderProgram, "explodeViewEnabled");     // --> toggle for explode view
    explodeAmountLocation = glGetUniformLocation(ShaderProgram, "explodeAmount");               //  --> explode amount for explode view

	modelCenterLocation = glGetUniformLocation(ShaderProgram, "modelCenter");                   //  --> model center uniform location
	highlightTrianglesLocation = glGetUniformLocation(ShaderProgram, "highlightTriangles");     //  --> highlight triangles in explode view

    // the glgetuniformlocation function returns the location of a uniform variable in the shader program and if it fails, it returns -1
	// + the input parameters are the shader program and the name of the uniform variable
    //the returned location is used to pass data to the shader program

}

/********************************************************************
 Callback Functions
 */

void onInit(int argc, char *argv[])
{
	/* by default the back ground color is black */
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	
	// Try to load default model
	modelLoaded = LoadOFFModel(modelPath);
	
	CreateVertexBuffer();
	CompileShaders();

    // #####################################################################
	// initialize depth colors based on object color
	UpdateDepthColors();

	/* set to draw in window based on depth  */
	glEnable(GL_DEPTH_TEST);
}

// #####################################################################################
// function to calculate normal matrix (inverse transpose of the model matrix)
Matrix4f CalculateNormalMatrix(const Matrix4f& modelMatrix) {
    // Create a copy of the model matrix for manipulation
    Matrix4f normalMatrix = modelMatrix;
    
    // We only need the 3x3 portion for normals, so set the translation part to zero
    normalMatrix.m[0][3] = 0.0f;
    normalMatrix.m[1][3] = 0.0f;
    normalMatrix.m[2][3] = 0.0f;
    
    // Calculate the inverse transpose
    normalMatrix.Inverse();
    normalMatrix.Transpose();
    
    return normalMatrix;
}

// #####################################################################################
//  function to generate an orthographic projection matrix
Matrix4f CreateOrthographicMatrix(float left, float right, float bottom, float top, float nearZ, float farZ) {
    Matrix4f result;
    result.SetZero();
    
    // Calculate matrix elements for orthographic projection
    result.m[0][0] = 2.0f / (right - left);
    result.m[1][1] = 2.0f / (top - bottom);
    result.m[2][2] = -2.0f / (farZ - nearZ);
    result.m[0][3] = -(right + left) / (right - left);
    result.m[1][3] = -(top + bottom) / (top - bottom);
    result.m[2][3] = -(farZ + nearZ) / (farZ - nearZ);
    result.m[3][3] = 1.0f;
    
    return result;
}

// #####################################################################################
// function to create a perspective projection matrix based on the fly-through mode
Matrix4f CreateProjectionMatrix() {
    if (flyThroughMode) {
        // Perspective projection for fly-through mode
        float fov = 45.0f;                                                      // --> Field of view in degrees
        float aspectRatio = (float)theWindowWidth / (float)theWindowHeight;     // --> Aspect ratio
        float nearPlane = 0.1f;                                                 // --> Near plane distance
        float farPlane = 100.0f;                                                // --> Far plane distance
        
        Matrix4f perspective;
        perspective.SetZero();
        //   Prespective matrix  
        //      1/tan(fov/2) 0 0 0
        //      0 1/tan(fov/2) 0 0
        //      0 0 (far+near)/(near-far) -1
        //      0 0 (2*far*near)/(near-far) 0
        // This is a simplified version of the perspective matrix used in OpenGL

        // Calculate perspective matrix constants
        float tanHalfFOV = tanf((fov / 2.0f) * 3.14159f / 180.0f); 
        float zRange = farPlane - nearPlane;  //
        
        // Fill in the perspective matrix properly
        perspective.m[0][0] = 1.0f / (tanHalfFOV * aspectRatio);
        perspective.m[1][1] = 1.0f / tanHalfFOV;
        perspective.m[2][2] = -(farPlane + nearPlane) / zRange;  
        perspective.m[2][3] = -(2.0f * farPlane * nearPlane) / zRange;  
        perspective.m[3][2] = -1.0f;  
        perspective.m[3][3] = 0.0f;  

        return perspective;
    } else {
        // Orthographic projection for normal mode
        return CreateOrthographicMatrix(-orthoSize, orthoSize, -orthoSize, orthoSize, -10.0f, 10.0f);
    }
}

// ################################################################################
// function to create a view matrix (camera transformation)
Matrix4f CreateViewMatrix() {
    if (!flyThroughMode) {
        // Default view for standard mode
        Matrix4f view;
        view.InitIdentity();
        return view;
    }
    
    // Yaw is the angle around the Y-axis (left-right)
    // Pitch is the angle around the X-axis (up-down)
    // Update camera position based on yaw and pitch
    // Calculate new camera direction vectors
    Vector3f direction;
    direction.x = cos(cameraYaw * 3.14159f/180.0f) * cos(cameraPitch * 3.14159f/180.0f);
    direction.y = sin(cameraPitch * 3.14159f/180.0f);
    direction.z = sin(cameraYaw * 3.14159f/180.0f) * cos(cameraPitch * 3.14159f/180.0f);
    direction.Normalize();
    
    cameraTarget = cameraPosition + direction;
    
    // Create view matrix using lookAt method
    Vector3f f = cameraTarget - cameraPosition;
    f.Normalize();
    
    Vector3f s = f.Cross(cameraUp);
    s.Normalize();
    
    Vector3f u = s.Cross(f);
    
    Matrix4f view;
    view.InitIdentity();
    
    view.m[0][0] = s.x;
    view.m[0][1] = s.y;
    view.m[0][2] = s.z;
    view.m[0][3] = -s.Dot(cameraPosition);
    
    view.m[1][0] = u.x;
    view.m[1][1] = u.y;
    view.m[1][2] = u.z;
    view.m[1][3] = -u.Dot(cameraPosition);
    
    view.m[2][0] = -f.x;
    view.m[2][1] = -f.y;
    view.m[2][2] = -f.z;
    view.m[2][3] = f.Dot(cameraPosition);
    
    return view;
}

// ######################################################################################
// function to create a rotation matrix based on the angle this is about the X and Y axes
Matrix4f CreateRotationMatrix(float angle) {
	Matrix4f rotationX, rotationY, rotationMatrix;
	
	rotationX.SetZero();
	rotationY.SetZero();
	rotationMatrix.SetZero();

	// X-axis rotation
	rotationX.m[0][0] = 1.0f;
	rotationX.m[0][1] = 0.0f;
	rotationX.m[0][2] = 0.0f;
	rotationX.m[0][3] = 0.0f;
	
	rotationX.m[1][0] = 0.0f;
	rotationX.m[1][1] = cosf(angle);
	rotationX.m[1][2] = -sinf(angle);
	rotationX.m[1][3] = 0.0f;
	
	rotationX.m[2][0] = 0.0f;
	rotationX.m[2][1] = sinf(angle);
	rotationX.m[2][2] = cosf(angle);
	rotationX.m[2][3] = 0.0f;
	
	rotationX.m[3][0] = 0.0f;
	rotationX.m[3][1] = 0.0f;
	rotationX.m[3][2] = 0.0f;
	rotationX.m[3][3] = 1.0f;
	
	// Y-axis rotation
	rotationY.m[0][0] = cosf(angle);
	rotationY.m[0][1] = 0.0f;
	rotationY.m[0][2] = sinf(angle);
	rotationY.m[0][3] = 0.0f;
	
	rotationY.m[1][0] = 0.0f;
	rotationY.m[1][1] = 1.0f;
	rotationY.m[1][2] = 0.0f;
	rotationY.m[1][3] = 0.0f;
	
	rotationY.m[2][0] = -sinf(angle);
	rotationY.m[2][1] = 0.0f;
	rotationY.m[2][2] = cosf(angle);
	rotationY.m[2][3] = 0.0f;
	
	rotationY.m[3][0] = 0.0f;
	rotationY.m[3][1] = 0.0f;
	rotationY.m[3][2] = 0.0f;
	rotationY.m[3][3] = 1.0f;
	
	// Combine rotations (Y * X)
	rotationMatrix = rotationY * rotationX;
	
	return rotationMatrix;
}



// ########################################################################################
// Modified this function for displaying the model
static void onDisplay()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set rendering mode based on user selection [0: solid, 1: wireframe]
    if (renderMode == 1) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Solid
    }

    // Create projection matrix based on current mode 
    Matrix4f Projection = CreateProjectionMatrix();
    
    // Create view matrix based on camera position
    Matrix4f View = CreateViewMatrix();
    
    // Create model matrix (rotation and normalization)
    Matrix4f Model;
    if (!flyThroughMode) {
        // In normal mode, apply rotation
        Model = CreateRotationMatrix(rotation) * CreateNormalizationMatrix();
    } else {
        // In fly-through mode, just normalize the model
        Model = CreateNormalizationMatrix();
    }
    
    // Calculate normal matrix (inverse transpose of model matrix)
    Matrix4f NormalMatrix = CalculateNormalMatrix(Model);
    
    // Combine matrices for the final transformation
    Matrix4f WorldMatrix = Projection * View * Model;

    // Send matrices to shaders [ we already got the uniform variable locations from the shader program ]
    glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, &WorldMatrix.m[0][0]);           // --> world matrix
    glUniformMatrix4fv(gModelMatrixLocation, 1, GL_TRUE, &Model.m[0][0]);           // --> model matrix
    glUniformMatrix4fv(gNormalMatrixLocation, 1, GL_TRUE, &NormalMatrix.m[0][0]);   // --> normal matrix


	// Set the new multiple lights instead
	for (int i = 0; i < MAX_LIGHTS; i++) {
		glUniform3f(lightsPositionLocation[i], lights[i].position.x, lights[i].position.y, lights[i].position.z);
		glUniform3f(lightsColorLocation[i], lights[i].color.x, lights[i].color.y, lights[i].color.z);
		glUniform1f(lightsIntensityLocation[i], lights[i].intensity);
		glUniform1i(lightsEnabledLocation[i], lights[i].enabled ? 1 : 0);
	}
		
	glUniform3f(objectColorLocation, objectColor.x, objectColor.y, objectColor.z);
	glUniform1f(ambientStrengthLocation, ambientStrength);
	glUniform1f(diffuseStrengthLocation, diffuseStrength);
	glUniform1f(specularStrengthLocation, specularStrength);
	glUniform1f(shininessLocation, shininess);

	glUniform1i(depthColoringEnabledLocation, depthColoringEnabled ? 1 : 0);
    glUniform1f(depthColorNearLocation, depthColorNear);
    glUniform1f(depthColorFarLocation, depthColorFar);
    glUniform3f(depthColorNearValueLocation, depthColorNearValue.x, depthColorNearValue.y, depthColorNearValue.z);
    glUniform3f(depthColorFarValueLocation, depthColorFarValue.x, depthColorFarValue.y, depthColorFarValue.z);

	glUniform1i(explodeViewEnabledLocation, explodeViewEnabled ? 1 : 0);
	glUniform1f(explodeAmountLocation, explodeAmount);

	glUniform3f(modelCenterLocation, modelCenter.x, modelCenter.y, modelCenter.z);

	glUniform1i(highlightTrianglesLocation, highlightTriangles ? 1 : 0);

    
    // Sending camera position for specular calculation 
    glUniform3f(gViewPosLocation, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    glBindVertexArray(VAO);
    
    // Use indexed drawing if we have indices
    if (indexCount > 0) {
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }
    
    // Render normals if needed
    if (normalMode > 0) {
        // Normal visualization rendering ()
    }
    
    glBindVertexArray(0);
    
    // Reset polygon mode to fill
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Check for rendering errors
    GLenum errorCode = glGetError();
    if (errorCode != GL_NO_ERROR) {
        fprintf(stderr, "OpenGL rendering error %d\n", errorCode);
    }
}

// #####################################################################################################
// Add this function to toggle mouse capture mode
void setMouseCapture(GLFWwindow* window, bool capture) {
    if (capture) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Hide cursor and capture it
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);  // Show normal cursor
    }
}
// #####################################################################################################
// Mouse button implementation for camera manipulation particularly for dragging
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    // Always forward mouse data to ImGui
    ImGuiIO& io = ImGui::GetIO();
    bool down = (action == GLFW_PRESS);
    io.AddMouseButtonEvent(button, down);

    // Only handle mouse input in your application if ImGui doesn't want to capture it
    if (!io.WantCaptureMouse && flyThroughMode)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) {
                // Start dragging
                isDraggingCamera = true;                                // Set dragging flag
                glfwGetCursorPos(window, &dragStartX, &dragStartY);     // Get initial mouse position
                dragStartYaw = cameraYaw;                               // Store initial yaw
                dragStartPitch = cameraPitch;                           // Store initial pitch
            } else if (action == GLFW_RELEASE) {
                // Stop dragging
                isDraggingCamera = false;                               // Reset dragging flag
            }
        }
    }
}
// #####################################################################################################
//  mouse movement callback to handle camera angle manipulation
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    // Always forward mouse position to ImGui
    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent((float)xpos, (float)ypos);

    // Handle direct camera manipulation with mouse drag in fly-through mode 
    if (!io.WantCaptureMouse && flyThroughMode && isDraggingCamera)     
    {
        // Calculate how much the mouse moved from the drag start position
        double xoffset = xpos - dragStartX;         // Horizontal movement
        double yoffset = dragStartY - ypos;         // Reversed direction for intuitive controls - vertical movement
        
        // Applying the offset to the camera angles
        // scaling factor to control sensitivity
        float sensitivity = 0.2f;
        cameraYaw = dragStartYaw + (float)(xoffset * sensitivity);
        cameraPitch = dragStartPitch + (float)(yoffset * sensitivity);
        
        // ####################################################################
        // Clamping pitch to avoid flipping [Important for fly-through mode]
        if (cameraPitch > 89.0f)
            cameraPitch = 89.0f;
        if (cameraPitch < -89.0f)
            cameraPitch = -89.0f;
    }
}

// #####################################################################################################
// Add a mouse scroll callback for zoom control
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // Forward scroll event to ImGui
    ImGuiIO& io = ImGui::GetIO();
    io.AddMouseWheelEvent((float)xoffset, (float)yoffset);
    
    // Only process if ImGui doesn't want to capture it
    if (!io.WantCaptureMouse) {
        if (flyThroughMode) {
            // In fly-through mode, adjust camera position along view direction
            // Calculate view direction
            Vector3f direction;
            // < x, y, z > = < cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch) >
            // yaw and pitch are in degrees, convert to radians for calculations
            direction.x = cos(cameraYaw * 3.14159f/180.0f) * cos(cameraPitch * 3.14159f/180.0f);
            direction.y = sin(cameraPitch * 3.14159f/180.0f);
            direction.z = sin(cameraYaw * 3.14159f/180.0f) * cos(cameraPitch * 3.14159f/180.0f);
            direction.Normalize();
            
            // Move camera position closer/further based on scroll
            cameraPosition = cameraPosition + direction * (float)yoffset * cameraSpeed * 2.0f;
        } else {
            // In standard mode, adjust orthographic zoom
            orthoSize -= (float)yoffset * 0.1f;         // Zoom in/out
            if (orthoSize < 0.1f) orthoSize = 0.1f;     // Prevent zooming too close
            if (orthoSize > 10.0f) orthoSize = 10.0f;   // Prevent zooming too far
        }
    }
}

// #####################################################################################
// Update the key callback function
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Forward keyboard input to ImGui
    ImGuiIO& io = ImGui::GetIO();
    
    // Only handle keyboard input in your application if ImGui doesn't want to capture it
    if (!io.WantCaptureKeyboard) 
    {
        if (action == GLFW_PRESS) {
            keys[key] = true;
            
            // Toggle fly-through mode with F key
            if (key == GLFW_KEY_F) {
                flyThroughMode = !flyThroughMode;
                if (flyThroughMode) {
                    // Reset camera position when entering fly-through mode
                    cameraPosition = Vector3f(0.0f, 0.0f, 5.0f);
                    cameraYaw = -90.0f;
                    cameraPitch = 0.0f;
                    firstMouse = true;
					setMouseCapture(window, true); // Capture mouse when entering fly-through mode
				} else {
					setMouseCapture(window, false); // Release mouse when exiting fly-through mode
                }
            }
			// In the key_callback function, add this:
			if (flyThroughMode && key == GLFW_KEY_TAB && action == GLFW_PRESS) {
				if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
					setMouseCapture(window, false); // Release mouse to interact with UI
				} else {
					setMouseCapture(window, true);  // Capture mouse for navigation
				}
			}
						
            // Other single-press key handlers  --> reset rotation, exit program
            if (key == GLFW_KEY_R) {
                rotation = 0;
            }
            if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) {
                glfwSetWindowShouldClose(window, true);
            }
        }
        else if (action == GLFW_RELEASE) {
            keys[key] = false;
        }
    }
}

// #####################################################################################
//  function to process keyboard input for camera movement
void process_keyboard_input()
{
    if (!flyThroughMode) return;
    
    // Calculate camera direction vectors
    Vector3f direction;
    direction.x = cos(cameraYaw * 3.14159f/180.0f) * cos(cameraPitch * 3.14159f/180.0f);
    direction.y = sin(cameraPitch * 3.14159f/180.0f);
    direction.z = sin(cameraYaw * 3.14159f/180.0f) * cos(cameraPitch * 3.14159f/180.0f);
    direction.Normalize();
    
    Vector3f right = direction.Cross(cameraUp);
    right.Normalize();
    
    // Process movement keys
    if (keys[GLFW_KEY_W])
        cameraPosition = cameraPosition + direction * cameraSpeed;
    if (keys[GLFW_KEY_S])
        cameraPosition = cameraPosition - direction * cameraSpeed;
    if (keys[GLFW_KEY_A])
        cameraPosition = cameraPosition - right * cameraSpeed;
    if (keys[GLFW_KEY_D])
        cameraPosition = cameraPosition + right * cameraSpeed;
    if (keys[GLFW_KEY_SPACE])
        cameraPosition.y += cameraSpeed;
    if (keys[GLFW_KEY_LEFT_SHIFT])
        cameraPosition.y -= cameraSpeed;
}


// Initialize ImGui
void InitImGui(GLFWwindow *window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	(void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
}

// ######################################################################################################
// Render ImGui
void RenderImGui(GLFWwindow *window)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Create model loading window
    ImGui::Begin("3D Model Viewer");
    
    ImGui::Text("Current model: %s", modelPath);
    ImGui::Text("Vertices: %d, Faces: %d", vertexCount, faceNormals.size());
    
    ImGui::InputText("Model Path", modelPath, sizeof(modelPath));
    
    if (ImGui::Button("Load Model")) {
        bool success = LoadOFFModel(modelPath);
        if (success) {
            modelLoaded = true;
            // Recreate vertex buffers
            glDeleteBuffers(1, &VBO);
            glDeleteBuffers(1, &IBO);
            glDeleteVertexArrays(1, &VAO);
            CreateVertexBuffer();
        }
    }
    
    ImGui::Separator();
    
    ImGui::Text("Rendering Options:");
    const char* renderModes[] = { "Solid", "Wireframe" };
    ImGui::Combo("Render Mode", &renderMode, renderModes, IM_ARRAYSIZE(renderModes));
    
    // const char* normalModes[] = { "No Normals", "Vertex Normals", "Face Normals" };
    // if (ImGui::Combo("Show Normals", &normalMode, normalModes, IM_ARRAYSIZE(normalModes))) {
    //     // Regenerate normal visualization if needed
    // }
    
    // if (normalMode > 0) {
    //     ImGui::SliderFloat("Normal Length", &normalLength, 0.01f, 0.5f);
    // }
    
    ImGui::Separator();
    
    // Camera controls
    if (ImGui::Checkbox("Fly-Through Mode", &flyThroughMode)) {
        if (flyThroughMode) {
            // Reset camera position when entering fly-through mode
            cameraPosition = Vector3f(0.0f, 0.0f, 5.0f);
            cameraYaw = -90.0f;
            cameraPitch = 0.0f;
            firstMouse = true;
            // Don't capture mouse in the new control system
            setMouseCapture(window, false);
        }
    }
    
    if (flyThroughMode) {
        // Add a second tab bar for camera controls
        if (ImGui::BeginTabBar("CameraControlTabs")) {
            if (ImGui::BeginTabItem("Position")) {
                // Camera position controls
                ImGui::Text("Camera Position:");
                ImGui::SliderFloat("X Position", &cameraPosition.x, -10.0f, 10.0f);
                ImGui::SliderFloat("Y Position", &cameraPosition.y, -10.0f, 10.0f);
                ImGui::SliderFloat("Z Position", &cameraPosition.z, -10.0f, 10.0f);
                
                // Camera movement speed control
                ImGui::SliderFloat("Movement Speed", &cameraSpeed, 0.01f, 0.5f);
                
                ImGui::Text("WASD - Move horizontally");
                ImGui::Text("Space/Shift - Move vertically");
                
                if (ImGui::Button("Reset Position")) {
                    cameraPosition = Vector3f(0.0f, 0.0f, 5.0f);
                }
                
                ImGui::EndTabItem();
            }
            
            if (ImGui::BeginTabItem("Orientation")) {
                // Camera orientation controls
                ImGui::Text("Camera Angle:");
                
                // Use sliders to directly control camera angles
                bool angleChanged = false;
                angleChanged |= ImGui::SliderFloat("Yaw (Left/Right)", &cameraYaw, -180.0f, 180.0f);
                angleChanged |= ImGui::SliderFloat("Pitch (Up/Down)", &cameraPitch, -89.0f, 89.0f);
                
                if (angleChanged) {
                    firstMouse = true; // Prevent jumping if switching to mouse
                }
                
                ImGui::EndTabItem();
            }
            
            if (ImGui::BeginTabItem("Presets")) {
                // Camera preset buttons for quick positioning
                if (ImGui::Button("Front View")) {
                    cameraPosition = Vector3f(0.0f, 0.0f, 5.0f);
                    cameraYaw = -90.0f;
                    cameraPitch = 0.0f;
                }
                
                if (ImGui::Button("Back View")) {
                    cameraPosition = Vector3f(0.0f, 0.0f, -5.0f);
                    cameraYaw = 90.0f;
                    cameraPitch = 0.0f;
                }
                
                if (ImGui::Button("Left View")) {
                    cameraPosition = Vector3f(-5.0f, 0.0f, 0.0f);
                    cameraYaw = 0.0f;
                    cameraPitch = 0.0f;
                }
                
                if (ImGui::Button("Right View")) {
                    cameraPosition = Vector3f(5.0f, 0.0f, 0.0f);
                    cameraYaw = 180.0f;
                    cameraPitch = 0.0f;
                }
                
                if (ImGui::Button("Top View")) {
                    cameraPosition = Vector3f(0.0f, 5.0f, 0.0f);
                    cameraYaw = -90.0f;
                    cameraPitch = -89.0f;
                }
                
                if (ImGui::Button("Bottom View")) {
                    cameraPosition = Vector3f(0.0f, -5.0f, 0.0f);
                    cameraYaw = -90.0f;
                    cameraPitch = 89.0f;
                }
                
                if (ImGui::Button("Isometric View")) {
                    cameraPosition = Vector3f(3.5f, 3.5f, 3.5f);
                    cameraYaw = -135.0f;
                    cameraPitch = -35.0f;
                }
                
                ImGui::EndTabItem();
            }
            
            ImGui::EndTabBar();
        }
    }
    else {
        ImGui::Checkbox("Auto Rotate", &isAnimating);
        ImGui::SliderFloat("Rotation", &rotation, 0.0f, 6.28f);
        if (ImGui::Button("Reset Rotation")) {
            rotation = 0.0f;
        }
        ImGui::SliderFloat("Zoom", &orthoSize, 0.5f, 5.0f);
    }


	ImGui::End();  // <-- End the "3D Model Viewer" window


	// In your RenderImGui function, replace the existing lighting controls with:
	ImGui::Begin("Lighting Controls");

	ImGui::Text("Material Properties:");
	// ImGui::ColorEdit3("Object Color", &objectColor.x);
	if (ImGui::ColorEdit3("Object Color", &objectColor.x)) {
		// Update depth colors if we're using object color shades
		if (useObjectColorForDepth) {
			UpdateDepthColors();
		}
	}
	ImGui::SliderFloat("Ambient Strength", &ambientStrength, 0.0f, 1.0f);
	ImGui::SliderFloat("Diffuse Strength", &diffuseStrength, 0.0f, 1.0f);
	ImGui::SliderFloat("Specular Strength", &specularStrength, 0.0f, 1.0f);
	ImGui::SliderFloat("Shininess", &shininess, 1.0f, 128.0f);

	ImGui::Separator();

	// Light controls
	if (ImGui::BeginTabBar("LightControls")) {
		// Add a tab for each light
		for (int i = 0; i < MAX_LIGHTS; i++) {
			char tabName[32];
			sprintf(tabName, "Light %d", i+1);
			
			if (ImGui::BeginTabItem(tabName)) {
				ImGui::Checkbox("Enabled", &lights[i].enabled);
				
				// Only show controls if the light is enabled
				if (lights[i].enabled) {
					char label[64];
					
					sprintf(label, "Position##%d", i);
					ImGui::SliderFloat3(label, &lights[i].position.x, -10.0f, 10.0f);
					
					sprintf(label, "Color##%d", i);
					ImGui::ColorEdit3(label, &lights[i].color.x);
					
					sprintf(label, "Intensity##%d", i);
					ImGui::SliderFloat(label, &lights[i].intensity, 0.0f, 2.0f);
				}
				
				ImGui::EndTabItem();
			}
		}
		
		// Add a tab for combined controls
		if (ImGui::BeginTabItem("All Lights")) {
			if (ImGui::Button("Enable All Lights")) {
				for (int i = 0; i < MAX_LIGHTS; i++) {
					lights[i].enabled = true;
				}
			}
			
			ImGui::SameLine();
			
			if (ImGui::Button("Disable All Lights")) {
				for (int i = 0; i < MAX_LIGHTS; i++) {
					lights[i].enabled = false;
				}
			}
			
			// Create light presets
			if (ImGui::Button("RGB Setup")) {
				// Red light
				lights[0].position = Vector3f(5.0f, 5.0f, 5.0f);
				lights[0].color = Vector3f(1.0f, 0.2f, 0.2f);
				lights[0].intensity = 1.0f;
				lights[0].enabled = true;
				
				// Green light
				lights[1].position = Vector3f(-5.0f, 0.0f, 5.0f);
				lights[1].color = Vector3f(0.2f, 1.0f, 0.2f);
				lights[1].intensity = 1.0f;
				lights[1].enabled = true;
				
				// Blue light
				lights[2].position = Vector3f(0.0f, -5.0f, -5.0f);
				lights[2].color = Vector3f(0.2f, 0.2f, 1.0f);
				lights[2].intensity = 1.0f;
				lights[2].enabled = true;
			}
			
			ImGui::SameLine();
			
			if (ImGui::Button("White Trio")) {
				// Front light
				lights[0].position = Vector3f(0.0f, 0.0f, 5.0f);
				lights[0].color = Vector3f(1.0f, 1.0f, 1.0f);
				lights[0].intensity = 0.7f;
				lights[0].enabled = true;
				
				// Left light
				lights[1].position = Vector3f(-5.0f, 0.0f, 0.0f);
				lights[1].color = Vector3f(1.0f, 1.0f, 1.0f);
				lights[1].intensity = 0.7f;
				lights[1].enabled = true;
				
				// Top light
				lights[2].position = Vector3f(0.0f, 5.0f, 0.0f);
				lights[2].color = Vector3f(1.0f, 1.0f, 1.0f);
				lights[2].intensity = 0.7f;
				lights[2].enabled = true;
			}
			
			ImGui::EndTabItem();
		}
		
		ImGui::EndTabBar();
	}

	if (ImGui::Button("Reset Lighting")) {
		ambientStrength = 0.2f;
		diffuseStrength = 0.7f;
		specularStrength = 0.5f;
		shininess = 32.0f;
		objectColor = Vector3f(0.8f, 0.8f, 0.8f);
		
		// Reset all lights to default
		// Light 1 - Red point light
		lights[0].position = Vector3f(5.0f, 5.0f, 5.0f);
		lights[0].color = Vector3f(1.0f, 0.2f, 0.2f);
		lights[0].intensity = 1.0f;
		lights[0].enabled = true;
		
		// Light 2 - Green point light
		lights[1].position = Vector3f(-5.0f, 0.0f, 5.0f);
		lights[1].color = Vector3f(0.2f, 1.0f, 0.2f);
		lights[1].intensity = 1.0f;
		lights[1].enabled = true;
		
		// Light 3 - Blue point light
		lights[2].position = Vector3f(0.0f, -5.0f, -5.0f);
		lights[2].color = Vector3f(0.2f, 0.2f, 1.0f);
		lights[2].intensity = 1.0f;
		lights[2].enabled = true;
	}

	ImGui::End();

	// Add after the lighting controls window or create a new window
	ImGui::Begin("Visualization Options");

	// Depth coloring controls
	// ImGui::Checkbox("Enable Depth Coloring", &depthColoringEnabled);
	bool prevDepthColorState = depthColoringEnabled;
	if (ImGui::Checkbox("Enable Depth Coloring", &depthColoringEnabled)) {
		// If just turned on depth coloring, update the colors
		if (!prevDepthColorState && depthColoringEnabled && useObjectColorForDepth) {
			UpdateDepthColors();
		}
	}
		
	if (depthColoringEnabled) {
		// Show depth range controls
		ImGui::SliderFloat("Near Distance", &depthColorNear, 0.1f, depthColorFar - 0.1f);
		ImGui::SliderFloat("Far Distance", &depthColorFar, depthColorNear + 0.1f, 20.0f);
		
		// Add option to use object color shades
		if (ImGui::Checkbox("Use Object Color Shades", &useObjectColorForDepth)) {
			if (useObjectColorForDepth) {
				// Update colors when switching to object color mode
				UpdateDepthColors();
			}
		}
		
		// Only show color pickers if not using object color shades
		if (!useObjectColorForDepth) {
			ImGui::ColorEdit3("Near Color", &depthColorNearValue.x);
			ImGui::ColorEdit3("Far Color", &depthColorFarValue.x);
		} else {
			// Just display the automatically calculated colors
			ImGui::Text("Near Color (Auto):");
			ImGui::SameLine();
			ImGui::ColorButton("##NearColor", ImVec4(depthColorNearValue.x, depthColorNearValue.y, depthColorNearValue.z, 1.0f));
			
			ImGui::Text("Far Color (Auto):");
			ImGui::SameLine();
			ImGui::ColorButton("##FarColor", ImVec4(depthColorFarValue.x, depthColorFarValue.y, depthColorFarValue.z, 1.0f));
		}
		
		if (ImGui::Button("Reset Depth Coloring")) {
			depthColorNear = 0.1f;
			depthColorFar = 10.0f;
			useObjectColorForDepth = true;
			UpdateDepthColors();
		}
		
		ImGui::Text("Depth coloring visualizes distance from camera");
		ImGui::Text("using color gradient between near and far points.");
	}

	ImGui::End();


	ImGui::Begin("Model View Options");
    
    // Add depth coloring controls here if you had them before
    
    ImGui::Separator();
    
    ImGui::Checkbox("Explode View", &explodeViewEnabled);
    
    if (explodeViewEnabled) {
		ImGui::SliderFloat("Explosion Amount", &explodeAmount, 0.0f, 2.0f);
		ImGui::Checkbox("Highlight Triangles", &highlightTriangles);
		
		// Add some diagnostic info
		ImGui::Text("Model Center: (%.2f, %.2f, %.2f)", modelCenter.x, modelCenter.y, modelCenter.z);
		ImGui::Text("Triangle Count: %d", indexCount / 3);
		
		if (ImGui::Button("Reset Explosion")) {
			explodeAmount = 0.2f;
			highlightTriangles = false;
		}
		
		ImGui::Text("Explode view separates triangles to show");
		ImGui::Text("the constituent primitives of the model.");
		ImGui::Text("Using geometry shader for precise triangle movement.");
	}
	   
    
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


// ########################################################################################
// Define main function
int main(int argc, char *argv[])
{

    // Initialize GLFW
    glfwInit();

    // Define version and compatibility settings
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Create OpenGL window and context
    GLFWwindow *window = glfwCreateWindow(800, 600, "3D Model Viewer", NULL, NULL);
    
    // Check for window creation failure
    if (!window)
    {
        // Terminate GLFW
        glfwTerminate();
        return 0;
    }
    
    // Make the OpenGL context current BEFORE checking for extensions
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    glewInit();
    printf("GL version: %s\n", glGetString(GL_VERSION));
    
    // ######################################################################################
    // NOW check for geometry shader support
    bool geometryShadersSupported = false;
    
    // Check if OpenGL 3.2+ (which supports geometry shaders)
    const char* version = (const char*)glGetString(GL_VERSION);
    int major = 0, minor = 0;
    sscanf(version, "%d.%d", &major, &minor);
    
    if (major > 3 || (major == 3 && minor >= 2)) {
        geometryShadersSupported = true;
        printf("Geometry shaders supported. Explosion view will use advanced triangle-based displacement.\n");
    } else {
        printf("Warning: Geometry shaders not supported. Explosion view may not work correctly.\n");
    }
    // ######################################################################################
    
    // Continue with initialization
    onInit(argc, argv);

    // Initialize ImGui
    InitImGui(window);

    // #####################################################################################
	// Set GLFW callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
    // #####################################################################################

	// Event loop
	while (!glfwWindowShouldClose(window))
	{
		// Clear the screen
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // #################################
		// process keyboard input
		process_keyboard_input();

        // #################################
		// Update rotation
		if(!flyThroughMode && isAnimating){
			rotation += 0.01f;
		}
		
		// Display 3D model
		onDisplay();

		// Render UI
		RenderImGui(window);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	
	if (model) {
	    FreeOffModel(model);
	}
	
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &IBO);
	glDeleteVertexArrays(1, &VAO);

	// Terminate GLFW
	glfwTerminate();
	return 0;
}
