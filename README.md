# Extrude

Extrudes an image into a 3D model in c++ with no external dependencies. Sample VS Code project that uses the single header file sokol libraries for rendering the extruded model.

asm.js/wasm live demo: https://stevinz.github.io/extrude-html5/

## Screenshots:

<p align="center"><img src="images/extruded.png" /></p>

## Methodology:

<p align="center"><img src="images/method.png" /></p>

1. The image is loaded into a byte array and then binarized based on the alpha channel (with an adjustable tolerance). 
2. Individual objects are counted using a flood fill algorithm. As an object is found it is removed from original image, this is repeated until there are no pixels remaining. The seperate objects are stored in new arrays sized accordingly, with their original start position also stored.
3. Holes within each object are found using additional flood fills. 
4. Polygons are formed by outlining the resulting objects using TraceImageOutline(). It starts at the top left of an image and scans down and to the right until a pixel is hit. It then proceeds, in a clockwise rotation, to the next adjacent pixel found with the smallest angle between that pixel and the current position.  

<br>

<p align="center"><img src="images/method2.png" /></p>

5. The resulting polygon is simplified by reducing points that fall along the same lines using the Ramer-Douglas-Peucker algorithm. 
6. The simplified polygon, along with the list of holes, is fed into an optimal polygon triangulation algorithm to form the front and back faces. 
7. Triangles are added betweem the front and back faces to form the sides. 
8. Boom, 3D!  

<br>

## Thanks to these libraries used during extrusion:

- Delaunator (MIT): https://github.com/soerendd/delaunator-cpp
- Handmade-Math (CC0): https://github.com/StrangeZak/Handmade-Math
- Mesh Optimizer (MIT): https://github.com/zeux/meshoptimizer 
- Poly Partition (MIT): https://github.com/ivanfratric/polypartition
- Ramer-Douglas-Peucker (CC0): https://gist.github.com/TimSC/0813573d77734bcb6f2cd2cf6cc7aa51
- Stb_image (MIT): https://github.com/nothings/stb

## Also thanks to these libraries used for the demo:

- Fontstash (Zlib): https://github.com/memononen/fontstash
- Sokol (Zlib): https://github.com/floooh/sokol
- Whereami (MIT): https://github.com/gpakosz/whereami