# Extrude

Extrudes an image into a 3D model in c++ with no external dependencies. Sample VS Code project that uses the single header file sokol libraries for rendering the extruded model.

asm.js/wasm live demo: https://stevinz.github.io/extrude-html5/

## Screenshots:

<p align="center"><img src="images/extruded.png" /></p>

## Methodology:

<p align="center"><img src="images/method.png" /></p>

Initially image is loaded into a byte array and then binarized based on the alpha channel (with an adjustable tolerance). Individual objects within the image are counted using flood fills. As an object is found it is removed from original image, this is repeated until there are no valid pixels. The seperate objects found are stored in new arrays sized accordingly, with their original start position stored as well. Holes within each object are found with additional flood fills. Polygons are formed by outlining the resulting objects using TraceImageOutline(). It works by starting at the top left of an image and scanning down and right until a pixel is hit. The algorithm proceeds to the next adjacent pixel with the smallest angle between its current position and the next adjacent pixel, in a clockwise rotation. The resulting polygon is simplified by reducing points along the same lines using the Ramer-Douglas-Peucker algorithm. The simplified polygon, along with the list of holes is fed into an optimal polygon triangulation algorithm to form the front and back faces. Triangles are added to form the sides based on the outline points. And boom, 3D!

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