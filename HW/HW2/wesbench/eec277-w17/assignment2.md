# Assignment 2

The purpose of Assignment 2 is to explore measurement and performance analysis of graphics hardware. In one sense, measuring graphics hardware is a little bit easier than microprocessors in that graphics hardware does a specific task (graphics) rather than more general tasks, so testing can be more focused. But on the other hand, graphics hardware is much more closed architecturally than a microprocessor, so it's difficult to know exactly what's happening under the hood.

The assignment has two parts; all students will do both parts. We are releasing only one part at first, on characterizing graphics performance. The next part will focus on reverse-engineering graphics functionality. I prefer that you do this assignment in groups of 2.

## Part 1: Measurement and Performance Analysis

The graphics characterization part was written by Wes Bethel at Lawrence Berkeley Laboratory (thanks Wes!) and slightly modified (to make it more of an instructional assignment, and more recently to use glfw) by me. The files are in a "wesBench" folder on Canvas.

The major deliverable for this part of the assignment is to write up the results for the investigation of the crossover point between geometry and rasterization. It is expected this writeup will contain data in the form of graphs and associated explanations. However, being brief while being complete is appreciated.

The key goal of this part of the assignment is to explore the crossover point between geometry and rasterization.

Modern graphics hardware can be generalized into two parallel components: geometry and rasterization. The performance of the graphics system is determined by the slower of these two components. For small triangles, the rasterization work per triangle is small so the system is limited by the rate at which vertices can be processed. For large triangles, the fragment operations dominate the rendering pipeline.

WesBench is a benchmark program that allows you to make this characterization under different conditions. WesBench allows you to use different geometry input options; to enable texturing, lighting, or both; and to iterate over different triangle areas, different geometry batch sizes, or different texture sizes.

In the source code, I have removed/altered the code that (a) calculates rendering (fill rate, vertex rate, etc.) performance and (b) iterates over triangle areas, geometry batch sizes, and texture sizes. All of these parts of the code are marked with TODO. This entire part of the assignment can be completed without writing a single line of OpenGL code—all changes are strictly C. However, it is vital that you understand the OpenGL code to benefit from this assignment. For those of you without much OpenGL experience, you will certainly have to do some investigation of the OpenGL calls used in the assignment.

Especially for OpenGL rookies, make sure that you understand the following questions as you do the assignment (no need to write them up, but you need to understand them to benefit from the assignment):

- Why do we use glDrawXXX calls rather than glBegin, glVertex, glEnd calls to draw geometry?
- What is wesBench actually drawing? Make sure you understand the matrix manipulation calls and the way the draw arrays are populated and what they represent.
- How does wesBench make sure that all geometry that it draws will pass through the entire pipeline (and affect the timing results) as opposed to getting clipped/culled/depth-tested out of the pipeline at an earlier stage (and thus making our results invalid)?

Your writeup should answer the following questions:

- How does the geometry rate (vertices/second) and fill rate (fragments/second) vary with triangle size? Why do you see this behavior? What is the crossover point of your GPU in terms of balance between geometry and rasterization? (-doareatest)
- How and why does the geometry/fill rate behavior / crossover point change when you (a) enable lighting (-light); (b) enable texturing (-tx); (c) enable texturing and lighting; (d) vary the triangle type (specifically, disjoint triangles vs. triangle strips) (-tt)?
- You will notice that at small batch sizes (batch size == amount of geometry sent in one call), the performance is limited by the CPU's ability to send work to the GPU ("interface-limited"). Roughly, at what batch size does your system stop being interface-limited and start being limited by the GPU? At that point, what is the geometry bandwidth between the CPU and the GPU? (-dovbtest)
- You will also notice in your texture tests that small texture sizes result in a faster fill rate than later texture sizes. Why? Calculate, characterize, and describe the relationship between (a) fill rate as a function of texture size and (b) texture bandwidth as a function of texture size. (-dotxtest)

## Grading

A high grade will be awarded if you demonstrate a good understanding of how graphics hardware could work. Coming up with the correct value for a particular performance metric is less important than how you analyize your results. You are not expected to know all of the details regarding the system you benchmark. Rather your grade will be determined by the tests you design and your analysis of the results.

## Honor Code

The full wesBench program is available on LBL's open-source website. It is an honor code violation to download or look at any source code from the full wesBench program. Please only use the instructional wesBench program.
