# MeshVista 🧊 3D
![Waketime](https://img.shields.io/badge/Waketime-53%3hrs%2015%20mins-blueviolet?style=flat&labelColor=black&logo=clock&logoColor=white)

**MeshVista** is an interactive 3D mesh visualization tool built with OpenGL. It allows users to load, render, and interact with 3D models in OFF format with advanced features like Phong shading, custom lighting, fly-through navigation, and exploded views.

---

## Authors 🧑‍💻

**Shreyas Mehta** (2023101059)  
Computer Graphics (2nd Year, IIIT Hyderabad)

## Index 📚
- [File Structure](#-project-structure)
- [Description](#-introduction)
- [Features](#-features-implemented)
- [How to Compile and Run](#-build-instructions)
  - [Prerequisites](#-prerequisites)
  - [Compilation](#-compilation)
  - [Running the Application](#️-running-the-application)
- [Controls](#-controls)
- [Code Explanation](#️-code-quality)
- [Summary](#-project-information)
- [Next Steps/Improvements](#-acknowledgments)
- [Implementation Details](IMPLEMENTATION.md) 📋

## 📂 Project Structure

```
.
├── main.cpp
├── MeshLoader.cpp / .h
├── OFFReader.cpp / .h
├── Shaders/
│   ├── vertex.glsl
│   ├── fragment.glsl
│   └── geometry.glsl
├── include/
│   └── imgui/
├── assets/
│   └── models/
├── Makefile
├── README.md
└── IMPLEMENTATION.md
```

## 📖 Introduction

The **MeshVista** is an interactive application developed as part of a Computer Graphics course. It allows users to load, visualize, and interact with 3D models stored in the **OFF (Object File Format)**. Implemented in **C++ using OpenGL**, this project showcases modern rendering, lighting, and model manipulation techniques.

The viewer supports features like **Phong shading**, **depth-based coloring**, **fly-through navigation**, and **exploded views**, making it a powerful tool for studying and exploring 3D mesh structures.

## ✅ Features Implemented

### 1. OFF Mesh File Loading
- **Description**: Loads 3D models stored in the OFF format by parsing vertex and face data.
- **Implementation**: Handled in the `OFFReader` module, which calculates bounding boxes and normalizes the mesh for consistent rendering.

### 2. Normal Calculation
- **Description**: Computes face and vertex normals, crucial for realistic lighting and geometric analysis.
- **Implementation**: 
  - Face normals: Cross product of triangle edges.
  - Vertex normals: Averaged from surrounding face normals.

### 3. Model Transformations
- **Description**: Centers the mesh and scales it to fit neatly in the viewport.
- **Implementation**: Uses bounding box calculations to center and uniformly scale the model.

### 4. Continuous Rotation
- **Description**: Enables smooth rotation around an arbitrary axis with adjustable speed.
- **Implementation**: Updates a rotation matrix every frame to apply continuous transformation.

### 5. Fly-through Mode
- **Description**: Lets users "walk through" the 3D scene in a first-person perspective.
- **Implementation**: Controls camera position and view direction using yaw and pitch angles.

### 6. Phong Shading (Blinn-Phong)
- **Description**: Realistic lighting using ambient, diffuse, and specular components.
- **Implementation**: Shader-based calculation in the fragment shader using the Blinn-Phong model.

### 7. Multiple Lights
- **Description**: Support for up to 3 customizable dynamic lights.
- **Implementation**: Lights are defined in GLSL as structs; their parameters are controlled via the GUI.

### 8. Depth-Based Coloring
- **Description**: Vertices are colored based on distance from the camera.
- **Implementation**: The fragment shader interpolates color from near to far using normalized depth.

### 9. Exploded View
- **Description**: Visually separates faces from the mesh center to highlight its structure.
- **Implementation**: A geometry shader computes displacement using face normals and distance from the model center.

## 🚀 Build Instructions

### 🔧 Prerequisites

Ensure the following libraries are installed:

- **OpenGL** (version 3.3 or higher)
- **GLFW** – Window and input handling
- **GLEW** – OpenGL extension wrangler
- **GLM** – Math library for vectors/matrices
- **ImGui** – Graphical user interface

### 📦 Compilation

1. Clone the repository:
   ```bash
   git clone <repository>
   cd <to-project-directory>
   ```

2. Build the project:
   ```bash
   make
   ```

This will generate an executable named `sample`.

### ▶️ Running the Application

Run the compiled binary:

```bash
./sample
```

### 🧹 Clean Up

To clean up the build files, use:

```bash
make clean
```
This will remove all object files and the executable.

## 🎮 Controls

### 🔄 General Controls
- **Load Model**: Use GUI to select and load an OFF file.
- **Render Mode**: Switch between solid and wireframe modes.
- **Zoom**: Scroll mouse wheel.

### 🚶 Fly-through Mode
- **Toggle Fly Mode**: `F`
- **Move**: `W` / `A` / `S` / `D`
- **Ascend/Descend**: `Space` / `Shift`
- **Look Around**: Drag mouse

### 💡 Lighting Controls
- **Enable/Disable Lights**: Toggle via "Lighting Controls" in GUI.
- **Edit Properties**: Change color, position, and intensity.
- **Presets**: Quick presets like "RGB Setup" and "White Trio".

### 🌈 Depth-Based Coloring
- **Enable Coloring**: Toggle in "Visualization Options".
- **Use Object Shades**: Enable shading using base object color.
- **Customize Colors**: Manually choose near/far gradient colors.

### 💥 Exploded View
- **Enable View**: Toggle via "Visualization Options".
- **Adjust Amount**: Use slider to control explosion magnitude.

## 🛠️ Code Quality

The codebase is **modular, clean, and well-documented**:
- Separated into multiple logical modules for loading, rendering, shading, and user interaction.
- Extensive in-line comments make it beginner-friendly and maintainable.
- Uses modern OpenGL practices (VAOs, VBOs, shaders, etc.).

## 👤 Project Information

**Author**: Shreyas Mehta (2023101059)  
**Course**: Computer Graphics (2nd Year, IIIT Hyderabad)  
**Assignment**: 2

This project was developed as part of the Computer Graphics course at IIIT Hyderabad, exploring 3D graphics programming with OpenGL. The assignment focused on implementing a versatile 3D mesh viewer capable of loading and rendering models in the OFF format with interactive visualization techniques.

## 📜 Acknowledgments

- **GLFW** – For managing window and input.
- **GLEW** – For OpenGL extensions.
- **GLM** – For matrix and vector operations.
- **ImGui** – For UI elements and controls.

## 📝 Next Steps/Improvements

- **Texture Support**: Add ability to load and display textured models.
- **Performance Optimization**: Implement occlusion culling and level-of-detail techniques.
- **Advanced Rendering**: Add support for shadow mapping and ambient occlusion.
- **Model Export**: Allow exporting modified models to various file formats.
- **Animation Support**: Implement skeletal animation for compatible models.

---

🎉 *Happy Mesh Viewing!*


