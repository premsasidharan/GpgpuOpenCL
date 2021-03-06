# GP-GPU with OpenCL
*One day training(6 hr) workshop to introduce the basics of GP-GPU programming. Covers GP-GPU basic concepts through 12 programming examples.

## Agenda
    - OpenCL Programming Model, Basics

    - Example 1: Vector Addition
    - Example 2: Color Inversion (OpenGL interop)
    - Example 3: Image Gradient
    - Example 4: Canny Edge Detection
    - Example 5: Image Histogram
    
    - Parallel Processing Patterns
        - Map
        - Stencil
        - Scatter
        - Gather
        - Reduction (Example 6: Reduce Sum)
        - Scan (Example 7: Prefix Sum)
        - Stream Compaction (Example 8: Harris Corner detection)
        
    - OpenCL 2.0
        - Prefix Sum with dynamic parallelism (Example 9)
        - Harris Corner Detection (Example 10)
        - Radix Sort (Example 11)
        - Matrix Multiplication (Example 12) 

    - Exercise: RGB Histogram

## Using the Training Material
[Presentation Slides](https://github.com/premsasidharan/GpgpuOpenCL/blob/master/docs/heterogeneous%20computing.pdf).

Open Visual Studio Workspace - GpgpuOpenCL/vs2015/GpgpuOpencl.sln


## Software and hardware requirements
    - Visual Studio 2015
    - A recent windows laptop (Intel HD graphics 5xxx/6xxx series or a discrete gpu from Nvidia or AMD).
    - Install “Intel OpenCL SDK” or “Nvidia CUDA SDK” depending on the GPU.
    - The workshop material includes all the other required libraries.
    - Some of the examples uses the system camera. For laptops the built in camera is enough, desktops might require a USB camera.
